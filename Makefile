
atr : atr.c
	gcc -std=c23 -W -Wall -pedantic -o atr atr.c
atr2imd : atr2imd.c
	gcc -std=c23 -W -Wall -pedantic -o atr2imd atr2imd.c
imd2atr : imd2atr.c
	gcc -std=c23 -W -Wall -pedantic -o imd2atr imd2atr.c
detok: detok.c
	gcc -std=c23 -W -Wall -pedantic -o detok detok.c
all: atr atr2imd imd2atr detok
clean:
	@rm -f atr *.o *.exe
