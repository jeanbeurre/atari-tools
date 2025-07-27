/* Convert Nick Kennedy's .ATR (Atari) disk image file format to
 * Dave Dunfield's .IMD (ImageDisk) file format
 *
 *	Copyright
 *		(C) 2011 Joseph H. Allen
 *
 * This is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 1, or (at your option) any later version.  
 *
 * It is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
 * details.  
 * 
 * You should have received a copy of the GNU General Public License along with 
 * this software; see the file COPYING.  If not, write to the Free Software Foundation, 
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A track */

struct track {
	struct track *next;
	int mode; /*
		0 = 500 kbps FM
		1 = 300 kbps FM
		2 = 250 kbps FM
		3 = 500 kbps MFM
		4 = 300 kbps MFM
		5 = 250 kbps MFM */
	int sec_size;
	int head;
	int cyl;
	int sects;
	unsigned char *map;
	unsigned char *data;
};

/* A loaded .IMD file */

struct imd {
	char *comment;
	struct track *tracks;
	int ntracks;
};

void free_imd(struct imd *imd)
{
	struct track *t;
	while ((t = imd->tracks)) {
		imd->tracks = t->next;
		if (t->map)
			free(t->map);
		if (t->data)
			free(t->data);
		free(t);
	}
	if (imd->comment)
		free(imd->comment);
	free(imd);
}

struct imd *read_imd(char *name)
{
	char buf[1024];
	int c;
	FILE *f = fopen(name, "rb");
	struct track* last = nullptr;

	if (!f) {
		fprintf(stderr, "Couldn't open %s\n", name);
		return nullptr;
	}

	printf("Converting %s\n", name);

	/* Read header */
	int x = 0;
	while (c = fgetc(f), c != -1 && c != 0x1A) {
		if (x < (int)(sizeof(buf) - 1))
			buf[x++] = c;
	}
	buf[x] = 0;

	if (!x) {
		fprintf(stderr, "No header?\n");
		fclose(f);
		return nullptr;
	}

	struct imd* imd = malloc(sizeof(struct imd));
	imd->comment = strdup(buf);
	imd->tracks = nullptr;
	imd->ntracks = 0;

	/* Read tracks */
	while (c = fgetc(f), c != -1) {
		if (c < 0 || c > 5) {
			fprintf(stderr,"Invalid mode byte?\n");
			fclose(f);
			free_imd(imd);
			return nullptr;
		}
//		printf("track\n");
		struct track* track = malloc(sizeof(struct track));
		track->data = nullptr;
		track->map = nullptr;
		track->next = nullptr;
		track->mode = c;
		c = fgetc(f);
		if (c < 0 || c > 80) {
			fprintf(stderr,"Invalid cylinder number\n");
			fclose(f);
			free_imd(imd);
			return nullptr;
		}
		track->cyl = c;
		c = fgetc(f);
		if (c < 0 || c > 1) {
			fprintf(stderr,"Invalid head number\n");
			fclose(f);
			free_imd(imd);
			return nullptr;
		}
		track->head = c;
		c = fgetc(f);
		if (c < 1) {
			fprintf(stderr,"Invalid number of sectors\n");
			fclose(f);
			free_imd(imd);
			return nullptr;
		}
		track->sects = c;
		c = fgetc(f);
		if (c < 0 || c > 6) {
			fprintf(stderr,"Invalid sector size\n");
			fclose(f);
			free_imd(imd);
			return nullptr;
		}
		track->sec_size = 128 << c;
		track->map = (unsigned char *)malloc(track->sects);
		if (1 != fread(track->map, track->sects, 1, f)) {
			fprintf(stderr,"Couldn't read sector map\n");
			fclose(f);
			free_imd(imd);
			return nullptr;
		}
		track->data = (unsigned char *)malloc(track->sects * track->sec_size);
		for (int i = 0; i != track->sects; ++i) {
			c = fgetc(f);
			if (c < 0 || c > 8) {
				fprintf(stderr,"Invalid sector type\n");
				fclose(f);
				free_imd(imd);
				return nullptr;
			}
			if (c & 1) {
				if (1 != fread(track->data + i * track->sec_size, track->sec_size, 1, f)) {
					fprintf(stderr,"Couldn't read sectors\n");
					fclose(f);
					free_imd(imd);
					return nullptr;
				}
			} else if (c == 0) {
				memset(track->data + i * track->sec_size, 0, track->sec_size);
			} else {
				c = fgetc(f);
				if (c < 0) {
					fprintf(stderr,"Couldn't compressed sector\n");
					fclose(f);
					free_imd(imd);
					return nullptr;
				}
				memset(track->data + i * track->sec_size, c, track->sec_size);
			}
		}
		if (last) {
			last->next = track;
			last = track;
		} else {
			last = imd->tracks = track;
		}
		++imd->ntracks;
	}
	fclose(f);
	return imd;
}

char *modes[] =
{
	"0 (500 kbps FM)",
	"1 (300 kbps FM)",
	"2 (250 kbps FM)",
	"3 (500 kbps MFM)",
	"4 (300 kbps MFM)",
	"5 (250 kbps MFM)"
};

void dump_imd(const struct imd *imd)
{
	printf("Comment = %s\n", imd->comment);
	printf("%d tracks\n", imd->ntracks);
	for (const struct track* t = imd->tracks; t; t = t->next) {
		printf("Cyl=%d Head=%d Sects=%d Sec_size=%d Mode=%s\n  Map:",
		       t->cyl, t->head, t->sects, t->sec_size, modes[t->mode]);
		for(int x = 0; x != t->sects; ++x)
			printf(" %d", t->map[x]);
		printf("\n");
	}
}

long imd_size(const struct imd *imd)
{
	long size = 0;
	for (const struct track* t = imd->tracks; t; t = t->next) {
		size += t->sec_size * t->sects;
	}
	return size;
}

int write_atr(const struct imd *imd, char *dest_name, const int logical, const int sio)
{
	unsigned char header[16];
	unsigned char buf[8192];
	int count = 0;

	const int sec_size = imd->tracks->sec_size;
	long size = imd_size(imd);

	printf("Sector size is %d\n", sec_size);

	if (sec_size == 256) {
		if (logical)
			printf("  Using logical\n");
		else if (sio)
			printf("  Using sio\n");
		else
			printf("  Using physical\n");
	}

	if (size & (sec_size - 1)) {
		fprintf(stderr,"Invalid .imd file size = %ld bytes\n", size);
		fprintf(stderr,"It must be multiple of %d\n", sec_size);
		return 1;
	}
	printf("Disk size is %ldK\n", size / 1024);

	FILE* f = fopen(dest_name, "rb");
	if (f) {
		char strbuf[80];
		fclose(f);
		printf("%s already exists.  Overwrite (y,n)?", dest_name);
		fgets(strbuf,sizeof(strbuf)-1,stdin);
		if (strbuf[0] != 'y' && strbuf[0] != 'Y') {
			printf("Skipping...\n");
			return 0;
		}
	}

	f = fopen(dest_name, "wb");
	if (!f) {
		fprintf(stderr,"Couldn't open %s\n", dest_name);
		return 1;
	}

	if (logical && sec_size == 256 && size >= 768)
		size -= 384;

	memset(header, 0, 16);
	header[0] = 0x96;
	header[1] = 0x02;
	header[4] = sec_size;
	header[5] = sec_size >> 8;
	header[2] = size >> 4;
	header[3] = size >> 12;
	header[6] = size >> 20;

	fwrite(header, 16, 1, f);

	for (const struct track* t = imd->tracks; t; t = t->next) {
		for (int x = 1; x != t->sects + 1; ++x) {
			int y;
			for (y = 0; y != t->sects; ++y)
				if (t->map[y] == x)
					break;
			memcpy(buf, t->data + t->sec_size * y, t->sec_size);
			for (y = 0; y != t->sec_size; ++y)
				buf[y] ^= 0xFF;
			if ((logical || sio) && sec_size == 256 && count < 3)
				fwrite(buf, 128, 1, f);
			else
				fwrite(buf, t->sec_size, 1, f);
			++count;
			if (sio && count == 3) {
				memset(buf, 0, 384);
				fwrite(buf, 384, 1, f);
			}
		}
	}

	fclose(f);
	return 0;
}

int main(const int argc, char *argv[])
{
	(void)argc;

	int dump = 0;
	int logical = 1;
	int sio = 0;

	int err = 0;
	int did = 0;

	/* Parse args */

	for (int x = 1; argv[x]; ++x) {
		if (argv[x][0] == '-') {
			if (!strcmp(argv[x], "--dump"))
				dump = 1;
			else if (!strcmp(argv[x], "--logical")) {
				logical = 1;
				sio = 0;
			} else if (!strcmp(argv[x], "--sio")) {
				sio = 1;
				logical = 0;
			} else if (!strcmp(argv[x], "--physical")) {
				sio = 0;
				logical = 0;
			} else
				err = 1;
		} else {
			char *source_name = argv[x];
			char dest_name[1024];
			struct imd *imd;
			char *p;

			/* Create destination name based on source name */
			strcpy(dest_name, source_name);
			if ((p = strrchr(dest_name, '.')))
				*p = 0;
			strcat(dest_name, ".atr");

			/* Read imd file */
			if (!(imd = read_imd(source_name)))
				return 1;

			if (dump)
				dump_imd(imd);

			/* Write atr file */
			if (write_atr(imd, dest_name, logical, sio))
				return 1;

			did = 1;
		}
	}

	if (!did || err) {
		fprintf(stderr,"Convert Dave Dunfield's .IMD (ImageDisk) file format to\n");
		fprintf(stderr,"Nick Kennedy's .ATR (ATARI) disk image file format.\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"       version 1.0\n");
		fprintf(stderr,"       by: Joe Allen (2011)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"imd2atr [options] filename\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"  --dump    Show tracks\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"The following options control how we deal with first three sectors of a 256-byte\n");
		fprintf(stderr,"sector disk.  Such disks store 256 bytes on the disk for these sectors, but the\n");
		fprintf(stderr,"Atari makes use of only the first 128 bytes of them.\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"  --physical Write all 256 bytes of these first three sectors.\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"  --logical  Write only 128 bytes each for first three sectors.\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"  --sio      Write only 128 bytes each for first three sectors,\n");
		fprintf(stderr,"             then write 384 bytes of zeros (followed by rest of disk).\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"Default format is '--logical', but emulators can deal with\n");
		fprintf(stderr,"all three of them and '--physical' preserves all data actually read.\n");
		return 1;
	}

	return 0;
}
