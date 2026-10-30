/**
 * @file server_main.c
 * @author Harini MINA
 * @brief Lit le port et le fichier de sortie 
 * @date 2026-10-28
 */

// cette partie a été faite par l'aide de l'IA
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "udp_socket.h"
#include "receiver.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage : %s <port> <fichier_de_sortie>\n", argv[0]);
        return 1;
    }

    // conversion du port, avec contrôle de validité
    char *fin;
    long port = strtol(argv[1], &fin, 10);
    if (*fin != '\0' || port < 1 || port > 65535) {
        fprintf(stderr, "Port invalide : %s (attendu : 1 à 65535)\n", argv[1]);
        return 1;
    }

    int sock = udp_open();
    if (sock == -1) {
        return 1;
    }
    if (udp_bind(sock, (uint16_t)port) == -1) {
        close(sock);
        return 1;
    }

    printf("Serveur en écoute sur le port %ld, sortie : %s\n", port, argv[2]);

    int res = sf_receive_file(sock, argv[2]);
    close(sock);

    if (res == -1) {
        fprintf(stderr, "Réception échouée\n");
        return 1;
    }
    printf("Fichier reçu : %s\n", argv[2]);
    return 0;
}