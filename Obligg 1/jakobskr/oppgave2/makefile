CFLAGS=-Wall -Wextra -Wpedantic

stringsum: stringsum.c
	gcc stringsum.c -o stringsum

test: oblig1_tests.c stringsum.c stringsum2.c split.c distance_between.c string_between.c
	gcc $(CFLAGS) oblig1_tests.c stringsum.c stringsum2.c split.c distance_between.c string_between.c -o test

clean:
	rm -f test

val:
	valgrind --show-leak-kinds=all --track-origins=yes ./test
