/**
 * @file test_udp.c
 * @author Harini MINA
 * @brief Pour tester les fonctions de udp_socket.c. Ce test a été généré par l'IA
 * @date 2026-10-26
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include "../src/network/udp_socket.h"
#include "../src/network/packet.h"

#define PORT 5000

static double ms_ecoulees(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000.0 + (b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(void) {
    uint8_t buf[2048];
    struct sockaddr_in dst, from;

    /* 1. udp_make_addr : IP valide, puis IP invalide */
    assert(udp_make_addr(&dst, "127.0.0.1", PORT) == 0);
    assert(dst.sin_family == AF_INET);
    assert(dst.sin_port == htons(PORT));
    assert(dst.sin_addr.s_addr == htonl(INADDR_LOOPBACK));
    struct sockaddr_in poubelle;
    assert(udp_make_addr(&poubelle, "pas une ip", PORT) == -1);
    printf("[OK] udp_make_addr\n");

    /* 2. udp_open et udp_bind (côté serveur) */
    int serveur = udp_open();
    assert(serveur >= 0);
    assert(udp_bind(serveur, PORT) == 0);
    printf("[OK] udp_open + udp_bind (descripteur serveur = %d)\n", serveur);

    /* 3. udp_set_timeout : sans rien envoyer, udp_recv doit rendre 0 après ~300 ms */
    assert(udp_set_timeout(serveur, 300) == 0);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int n = udp_recv(serveur, buf, sizeof(buf), &from);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double duree = ms_ecoulees(t0, t1);
    assert(n == 0);
    assert(duree >= 250 && duree < 600);
    printf("[OK] udp_set_timeout : udp_recv a rendu 0 après %.0f ms\n", duree);

    /* 4. Envoi d'un vrai paquet SecureFlow du client vers le serveur */
    int client = udp_open();            // pas de bind : le noyau choisit un port source
    assert(client >= 0);

    const char *msg = "bonjour";
    int taille = sf_packet_pack(buf, sizeof(buf), SF_TYPE_DATA, 42,
                                (const uint8_t *)msg, (uint16_t)strlen(msg));
    assert(taille == 20);
    assert(udp_send(client, &dst, buf, (size_t)taille) == taille);
    printf("[OK] udp_send : %d octets envoyés\n", taille);

    /* 5. Réception côté serveur */
    uint8_t recu[2048];
    n = udp_recv(serveur, recu, sizeof(recu), &from);
    assert(n == taille);
    printf("[OK] udp_recv : %d octets reçus\n", n);

    /* 6. L'expéditeur est bien identifié */
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &from.sin_addr, ip, sizeof(ip));
    printf("     expéditeur : %s:%u\n", ip, ntohs(from.sin_port));
    assert(from.sin_addr.s_addr == htonl(INADDR_LOOPBACK));
    assert(ntohs(from.sin_port) != PORT);   // port éphémère choisi par le noyau

    /* 7. Désencapsulation du paquet reçu */
    sf_header_t h;
    const uint8_t *payload;
    assert(sf_packet_unpack(recu, (size_t)n, &h, &payload) == 0);
    assert(h.type == SF_TYPE_DATA);
    assert(h.seq == 42);
    assert(h.payload_len == strlen(msg));
    assert(memcmp(payload, msg, strlen(msg)) == 0);
    printf("[OK] unpack : type=%u seq=%u payload=\"%.*s\"\n",
           h.type, h.seq, h.payload_len, payload);

    /* 8. Le serveur répond avec un ACK, grâce à l'adresse 'from' */
    int t_ack = sf_packet_pack(buf, sizeof(buf), SF_TYPE_ACK, 43, NULL, 0);
    assert(udp_send(serveur, &from, buf, (size_t)t_ack) == t_ack);

    udp_set_timeout(client, 500);
    n = udp_recv(client, recu, sizeof(recu), &from);
    assert(n == t_ack);
    assert(sf_packet_unpack(recu, (size_t)n, &h, &payload) == 0);
    assert(h.type == SF_TYPE_ACK && h.seq == 43);
    printf("[OK] ACK reçu par le client (seq=%u)\n", h.seq);

    close(client);
    close(serveur);
    printf("\nTous les tests UDP sont OK\n");
    return 0;
}