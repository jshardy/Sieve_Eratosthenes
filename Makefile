CLFAGS = -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls \
		-Wmissing-declarations -Wold-style-definition -Wmissing-prototypes \
        -Wdeclaration-after-statement -pthread
DEPS =
PROG = lab8

all: $(PROG)

lab8.o: lab8.c $(DEPS)
	$(CC) -c $< $(CLFAGS)

lab8: lab8.o
	$(CC) -o $@ $^ $(CLFAGS)

extract_primes.o: extract_primes.c $(DEPS)
	$(CC) -g -c $<

extract_primes: extract_primes.o
	$(CC) -g -o $@ $^

valgrind1: lab8
	valgrind ./lab4_1 -t 8 -u 10240

compress: clean
	tar -czvf lab.tar.gz *

clean:
	rm -rf $(PROG) *.o *.dSYM
