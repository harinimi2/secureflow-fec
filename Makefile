CC      = gcc
CFLAGS  = -Wall -Wextra -g

SRC_NET = src/network/packet.c src/network/udp_socket.c

SRC_ALL = src/network/packet.c src/network/udp_socket.c \
          src/network/sender.c src/network/receiver.c

HDRS    = src/network/protocol.h src/network/packet.h src/network/udp_socket.h \
          src/network/sender.h src/network/receiver.h

all: test_pack test_udp test_transfer

test_pack: tests/test_pack.c src/network/packet.c $(HDRS)
	$(CC) $(CFLAGS) -o $@ tests/test_pack.c src/network/packet.c

test_udp: tests/test_udp.c $(SRC_NET) $(HDRS)
	$(CC) $(CFLAGS) -o $@ tests/test_udp.c $(SRC_NET)

test_transfer: tests/test_transfer.c $(SRC_ALL) $(HDRS)
	$(CC) $(CFLAGS) -o $@ tests/test_transfer.c $(SRC_ALL)

test: all
	./test_pack
	./test_udp
	./test_transfer

clean:
	rm -f test_pack test_udp test_transfer tmp_original.bin tmp_recu.bin

.PHONY: all test clean