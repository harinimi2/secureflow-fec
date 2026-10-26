/**
 * @file sender.c
 * @author Harini MINA
 * @brief sender est le module qui envoie un fichier complet à travers UDP, de façon fiable
 * C'est le client : il possède le fichier et l'envoie
 * @date 2026-10-26
 */

#include "sender.h"
#include "sender.h"
#include "packet.h"        // sf_packet_pack, sf_packet_unpack, sf_header_t, SF_TYPE_*
#include "udp_socket.h"    // udp_send, udp_recv, udp_set_timeout

#include <stdio.h>         // fopen, fread, fclose, perror, printf
#include <string.h>        // memcmp, memcpy (si vous en avez besoin)

int sf_send_packet_reliable(int sock, const struct sockaddr_in *dst, const uint8_t *pkt, size_t len, uint32_t seq, int max_retries) {

}
