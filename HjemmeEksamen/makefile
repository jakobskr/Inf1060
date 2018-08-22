CFLAGS=-Wall -Wextra -Wpedantic -std=gnu99 -g
# CFLAGD

klient: klient.c
	gcc $(CFLAGS)  klient.c -o klient

server: server.c
	gcc $(CFLAGS) server.c -o server

debug_server: server.c
	gcc $(CFLAGS) -DDEBUG server.c -o server

debug_klient: klient.c
	gcc $(CFLAGS) -DDEBUG klient.c -o klient

both: klient server
	make klient server

run: klient
	./klient 127.0.0.1 4444

runs: server
	./server "a_doll's_house_short.job" 4444

valgs: server
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./server "a_doll's_house_short.job" 4444

valgk: klient
	valgrind  --track-origins=yes --leak-check=full --show-leak-kinds=all ./klient 127.0.0.1 4444
