BIN = bin

all: atr atr2imd imd2atr detok

$(BIN):
	mkdir -p $@

atr: atr.c | $(BIN)
	gcc -std=c23 -W -Wall -pedantic -o $(BIN)/$@ atr.c

atr2imd: atr2imd.c | $(BIN)
	gcc -std=c23 -W -Wall -pedantic -o $(BIN)/$@ atr2imd.c

imd2atr: imd2atr.c | $(BIN)
	gcc -std=c23 -W -Wall -pedantic -o $(BIN)/$@ imd2atr.c

detok: detok.c | $(BIN)
	gcc -std=c23 -W -Wall -pedantic -o $(BIN)/$@ detok.c

clean:
	@rm -rf $(BIN)