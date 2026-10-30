/**
 * @file client_main.c
 * @author Harini MINA
 * @brief Lit IP, port et chemin du fichier en arguments
 * @date 2026-10-30
 */

// cette partie a été faite par l'aide de l'IA

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "udp_socket.h"
#include "sender.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage : %s <ip_serveur> <port> <fichier_à_envoyer>\n", argv[0]);
        return 1;
    }

    char *fin;
    long port = strtol(argv[2], &fin, 10);
    if (*fin != '\0' || port < 1 || port > 65535) {
        fprintf(stderr, "Port invalide : %s (attendu : 1 à 65535)\n", argv[2]);
        return 1;
    }

    struct sockaddr_in dst;
    if (udp_make_addr(&dst, argv[1], (uint16_t)port) == -1) {
        return 1;
    }

    int sock = udp_open();     // pas de bind : le noyau choisit un port source
    if (sock == -1) {
        return 1;
    }

    printf("Envoi de %s vers %s:%ld\n", argv[3], argv[1], port);

    int res = sf_send_file(sock, &dst, argv[3]);
    close(sock);

    if (res == -1) {
        fprintf(stderr, "Envoi échoué\n");
        return 1;
    }
    printf("Fichier envoyé\n");
    return 0;
}