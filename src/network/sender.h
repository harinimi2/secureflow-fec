/**
 * @file sender.h
 * @author Harini MINA
 * @brief 
 * @date 2026-10-26
 */

#ifndef SENDER_H
#define SENDER_H

#include <stdint.h>        // uint8_t, uint32_t
#include <stddef.h>        // size_t
#include <netinet/in.h>    // struct sockaddr_in

int sf_send_packet_reliable(int sock, const struct sockaddr_in *dst, const uint8_t *pkt, size_t len, uint32_t seq, int max_retries); // envoie un paquet, attend l'ACK correspondant, retransmet à l'expiration du délai
int sf_send_file(int sock, const struct sockaddr_in *dst, const char *path); // découpe le fichier en fragments, les envoie un à un, termine par FIN


#endif