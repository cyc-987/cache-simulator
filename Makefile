
CC = gcc
CFLAGS = -g

all:  sim

sim:  main.o cache.o cache_utils.o
	$(CC) -o sim main.o cache.o cache_utils.o -lm

main.o:  main.c cache.h
	$(CC) $(CFLAGS) -c main.c

cache.o:  cache.c cache.h cache_utils.h
	$(CC) $(CFLAGS) -c cache.c

cache_utils.o:  cache_utils.c cache.h cache_utils.h
	$(CC) $(CFLAGS) -c cache_utils.c

clean:
	rm -f sim *.o

# test cases

testp1: sim
	./sim -bs 16 -us 8192 -a 1 -wb -wa ../traces/public-assoc.trace
testp2: sim
	./sim -bs 16 -us 8192 -a 1 -wb -wa ../traces/public-block.trace
testp3: sim
	./sim -bs 16 -us 8192 -a 1 -wb -wa ../traces/public-instr.trace
testp4: sim
	./sim -bs 16 -us 8192 -a 1 -wb -wa ../traces/public-write.trace
tests1: sim
	./sim -bs 16 -us 8192 -a 1 -wb -wa ../ext_traces/spice.trace
tests2: sim
	./sim -bs 16 -us 8192 -a 1 -wb -wa ../traces/spice10.trace
tests3: sim
	./sim -bs 16 -us 8192 -a 1 -wb -wa ../traces/spice100.trace
tests4: sim
	./sim -bs 16 -us 8192 -a 1 -wb -wa ../traces/spice1000.trace

testt1: sim
	./sim -bs 16 -is 8192 -ds 8192 -a 1 -wb -wa ../ext_traces/spice.trace
testt2: sim
	./sim -bs 16 -is 65536 -ds 65536 -a 1 -wb -wa ../ext_traces/spice.trace
testt3: sim
	./sim -bs 16 -us 8192 -a 1 -wb -wa ../ext_traces/spice.trace
testt4: sim
	./sim -bs 64 -us 8192 -a 1 -wb -wa ../ext_traces/spice.trace
testt5: sim
	./sim -bs 128 -is 32768 -ds 32768 -a 1 -wb -wa ../ext_traces/spice.trace
testt6: sim
	./sim -bs 64 -is 8192 -ds 8192 -a 2 -wb -wa ../ext_traces/spice.trace
testt7: sim
	./sim -bs 64 -is 8192 -ds 8192 -a 2 -wt -wa ../ext_traces/spice.trace
testt8: sim
	./sim -bs 16 -is 8192 -ds 8192 -a 1 -wt -wa ../ext_traces/spice.trace
testt9: sim
	./sim -bs 16 -is 8192 -ds 8192 -a 1 -wb -nw ../ext_traces/spice.trace