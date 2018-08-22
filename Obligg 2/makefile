CFLAGS=-Wall -Wextra -Wpedantic -std=c99

ruter: ruterdrift.c
	gcc $(CFLAGS) ruterdrift.c -o ruter

run1: ruter
	./ruter 1router.dat

run5: ruter
	./ruter 5router.dat

valg: ruter
	valgrind --track-origins=yes ./ruter 1router.dat

valg5: ruter
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./ruter 5router.dat
