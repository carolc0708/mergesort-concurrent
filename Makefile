CC = gcc
CFLAGS = -std=gnu99 -Wall -g -pthread 
OBJS = list.o threadpool.o main.o

.PHONY: all clean test

GIT_HOOKS := .git/hooks/pre-commit

all: $(GIT_HOOKS) sort sort_monitor

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

deps := $(OBJS:%.o=.%.o.d)
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF .$@.d -c $<

sort: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -rdynamic

sort_monitor:
	gcc -std=gnu99 -Wall -g -pthread  -o list.o -MMD -MF .list.o.d -c list.c
	gcc -std=gnu99 -Wall -g -pthread  -DMONITOR -o threadpool.o -MMD -MF .threadpool.o.d -c threadpool.c
	gcc -std=gnu99 -Wall -g -pthread  -DMONITOR -o main.o -MMD -MF .main.o.d -c main.c
	gcc -std=gnu99 -Wall -g -pthread  -DMONITOR -o sort_monitor list.o threadpool.o main.o -rdynamic

bench: CFLAGS += -DBENCH

input_generator: all 
	$(CC) $(CFLAGS) input_generator.c -o input_generator

bench: input_generator
	for i in `seq 1 1 200`; do \
		./input_generator $$i; \
		for j in 1 2 4 8 16 32 64; do \
			./sort $$j $$i; \
		done \
	done

plot: bench
	gnuplot scripts/runtime.gp

bench_monitor: input_generator
	for i in `seq 1 1 200`; do \
		./input_generator $$i; \
		for j in 1 2 4 8 16 32 64; do \
			./sort $$j $$i; \
			./sort_monitor $$j $$i; \
		done \
	done

calculate: bench_monitor
	$(CC) $(CFLAGS) calculate.c -o calculate

plot_monitor: calculate
	./calculate
	gnuplot scripts/runtime2.gp

clean:
	rm -f $(OBJS) sort sort_monitor\
		input_generator calculate input output runtime.png monitor.png \
		orig.txt monitor.txt output.txt
	@rm -rf $(deps)

-include $(deps)
