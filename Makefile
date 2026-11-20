CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Iinclude
SAN_FLAGS = -std=c17 -Wall -Wextra -g -O1 -fsanitize=address,undefined -Iinclude

SRCS = src/main.c src/transaction.c src/data_generator.c src/graph.c src/sorting.c src/searching.c src/string_matching.c src/fraud_rules.c src/analysis.c src/divide_conquer.c src/divide_conquer_matrix.c src/greedy.c src/dp.c src/backtracking.c src/flow.c src/number_theory.c src/benchmark.c
LIB_SRCS = src/transaction.c src/data_generator.c src/graph.c src/sorting.c src/searching.c src/string_matching.c src/fraud_rules.c src/analysis.c src/divide_conquer.c src/divide_conquer_matrix.c src/greedy.c src/dp.c src/backtracking.c src/flow.c src/number_theory.c src/benchmark.c

all: workbench

workbench: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o workbench -lm

test: tests/test_all.c $(LIB_SRCS)
	$(CC) $(CFLAGS) tests/test_all.c $(LIB_SRCS) -o test_all -lm
	./test_all

sanitize: tests/test_all.c $(LIB_SRCS)
	$(CC) $(SAN_FLAGS) tests/test_all.c $(LIB_SRCS) -o test_all_san -lm
	./test_all_san

clean:
	rm -f workbench test_all test_all_san
