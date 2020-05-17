
#RPATH=-Wl,-rpath /common/pkgs/gcc/4.9.4/lib64/
CC=g++
OPT?=3
CFLAGS=-I. -std=c++11 -O$(OPT) -pthread -g -Werror $(RPATH)
DEPS = 

WORKING_SET=10000 100000 1000000 10000000 100000000
NUMBER_OF_SEARCHES=10000000
PERF=perf stat -B -e cache-references,cache-misses,cache-misses:u,cycles,instructions,branches,faults,migrations 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

binary_search: binary_search.o 
	$(CC) -o binary_search binary_search.o $(CFLAGS)

clean:
	rm *.o
	rm binary_search

binary-search-runtimes: binary_search
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-off --stride=0 --searches=$(NUMBER_OF_SEARCHES) ; \
	done
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-on --stride=0 --searches=$(NUMBER_OF_SEARCHES); \
	done
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-off --stride=1 --searches=$(NUMBER_OF_SEARCHES); \
	done
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-on --stride=1 --searches=$(NUMBER_OF_SEARCHES); \
	done
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-off --stride=100 --searches=$(NUMBER_OF_SEARCHES); \
	done
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-on --stride=100 --searches=$(NUMBER_OF_SEARCHES); \
	done
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-off --stride=10000 --searches=$(NUMBER_OF_SEARCHES); \
	done
	for number in $(WORKING_SET) ; do \
		./binary_search --working-set=$$number --prefetching-on --stride=10000 --searches=$(NUMBER_OF_SEARCHES); \
	done

binary-search-cache-misses: binary_search
	for number in $(WORKING_SET) ; do \
		$(PERF) ./binary_search --working-set=$$number --prefetching-off --stride=0 --searches=$(NUMBER_OF_SEARCHES) ; \
	done
	for number in $(WORKING_SET) ; do \
		$(PERF) ./binary_search --working-set=$$number --prefetching-on --stride=0 --searches=$(NUMBER_OF_SEARCHES); \
	done


profile:
	perf record --call-graph dwarf ./binary_search

report:
	perf report
