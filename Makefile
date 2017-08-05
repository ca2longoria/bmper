
CC := gcc
OPTS := -O2

all: clean compiletest
	
clean:
	-rm build/*
pull:
	git pull
test:	compiletest runtest

compiletest:
	-mkdir build
	$(CC) $(OPTS) -o build/test src/test.c
	cp src/bmper.h build/
runtest:
	build/test


