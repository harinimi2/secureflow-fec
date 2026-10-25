/**
 * @file udp_socket.h
 * @author Harini MINA
 * @brief Sorte de boîte aux lettres dans lequel on mettra le paquet
 * @date 2026-10-25
 */

#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <stdint.h>        // uint8_t, uint16_t
#include <stddef.h>        // size_t
#include <netinet/in.h>    // struct sockaddr_in

// idée : la socket ne regarde pas ce qu'il a dans le paquet

int udp_open(void); // crée une socket UDP et retour le descripteur, ou -1
int udp_bind(int sock, uint16_t port); // lie la socket à un port local
// attention ne pas oublier de convertir le port (16 bits) en ordre réseau

int udp_make_addr(struct sockaddr_in *addr, const char* ip, uint16_t port); // remplit une @ de desti à partir d'une IP texte et d'un port
int udp_send(int sock, const struct sockaddr_in *dst, const uint8_t *buf, size_t len); // envoie un datagramme
#endif