CC = gcc
CFLAGS = -std=gnu99 -Wall -g -pthread -DBENCH
OBJS = list.o threadpool.o main.o

.PHONY: all clean test

GIT_HOOKS := .git/hooks/pre-commit

ifeq ($(strip $(BENCH)),1)
CFLAGS += -DBENCH
endif

all: $(GIT_HOOKS) sort

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

deps := $(OBJS:%.o=.%.o.d)
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF .$@.d -c $<

sort: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -rdynamic

input_generator: all 
	$(CC) $(CFLAGS) input_generator.c -o input_generator

bench: input_generator
	for i in `seq 100 100 800`; do \
		./input_generator $$i; \
		for j in 1 2 4 8 16 32 64; do \
			./sort $$j $$i; \
		done \
	done

plot: bench
	gnuplot scripts/runtime.gp

clean:
	rm -f $(OBJS) sort \
		input_generator input output runtime.png 
	@rm -rf $(deps)

-include $(deps)
