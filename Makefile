BIN = bin

all: atr atr2imd imd2atr detok

bin:
	mkdir -p $(BIN)

atr: atr.c | $(BIN)
	gcc -W -Wall -pedantic -o $(BIN)/atr atr.c

atr2imd: atr2imd.c | $(BIN)
	gcc -W -Wall -pedantic -o $(BIN)/atr2imd atr2imd.c

imd2atr: imd2atr.c | $(BIN)
	gcc -W -Wall -pedantic -o $(BIN)/imd2atr imd2atr.c

detok: detok.c | $(BIN)
	gcc -W -Wall -pedantic -o $(BIN)/detok detok.c

clean:
	@rm -rf $(BIN)