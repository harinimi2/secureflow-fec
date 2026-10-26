CC      = gcc
CFLAGS  = -Wall -Wextra -g

SRC_NET = src/network/packet.c src/network/udp_socket.c

all: test_pack test_udp

test_pack: tests/test_pack.c src/network/packet.c src/network/packet.h src/network/protocol.h
	$(CC) $(CFLAGS) -o $@ tests/test_pack.c src/network/packet.c

test_udp: tests/test_udp.c $(SRC_NET) src/network/udp_socket.h src/network/packet.h src/network/protocol.h
	$(CC) $(CFLAGS) -o $@ tests/test_udp.c $(SRC_NET)

test: all
	./test_pack
	./test_udp

clean:
	rm -f test_pack test_udp

.PHONY: all test clean