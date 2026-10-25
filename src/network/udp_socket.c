/**
 * @file udp_socket.c
 * @author Harini MINA
 * @brief Implémentation de la socket 
 * @date 2026-10-25
 */

#include "udp_socket.h"

#include <stdio.h>         // perror
#include <string.h>        // memset
#include <unistd.h>        // close
#include <sys/socket.h>    // socket, bind, sendto, recvfrom, setsockopt
#include <sys/time.h>      // struct timeval (pour le délai d'attente)
#include <arpa/inet.h>     // inet_pton, htons

// le socket, c'est comme un fichier 
// le noyau est composé d'une liste de socket

// création de la socket
int udp_open(void) {

    // Rappel signature de socket :
    // int socket(int domain, int type, int protocol);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    // AF_INET : les @ IPv4 (rmq : pour IPv6, AF_INET6)
    //SOCK_DGRAM : communication par datagrammes, sans connexion (cad UDP) (rmq: pour les TCP, on aurait mis SOCK_STREAM)
    // 0 : car SOCK_DGRAM sur IPv4 n'a qu'un protocole possibke (UDP), donc on laisse le systèle le choisir

    if(sock == -1) { // si socket() échoue
        perror("udp_open");
        return -1;
    }
    return sock;
}

// ATTENTION PIEGE :
// à la création, la sokcet n'a pas encore d'@ mais elle existe, mais personne ne sait la joindre

// quand un paquet arrive, il faut que le noyau remet à cette socket tous les datagrammes arrivant sur ce port
// cette fonction associe mon socket à un prot local

int udp_bind(int sock, uint16_t port) {
    // pour mieux comprendre, ce que c'est une socket, faire man2 bind dans le terminal
    
    // création d'une struct sockaddr_in qui représente une @reseau IPv4
    struct sockaddr_in {
        sa_family_t sin_family;   // la famille : IPv4
        in_port_t sin_port;     // le port (16 bits)
        struct in_addr sin_addr;     // l'adresse IP (32 bits), dans sin_addr.s_addr
    //+ quelques octets de remplissage
    };

    struct sockaddr_in nouv_adr_sock; // @IPv4 locale à associer à la socket
    nouv_adr_sock.sin_family = AF_INET; // famille IPv4
    nouv_adr_sock.sin_port = htons(port); //unint16_t mais qu'il faut convertir en ordre réseau (cad on a converti le port en ordre réseau)
    nouv_adr_sock.sin_addr.s_addr = htonl(INADDR_ANY); // toutes les interfaces de la machine

    if (bind(sock, (struct sockaddr *)&nouv_adr_sock, sizeof(nouv_adr_sock)) == -1) {
        perror("udp_bind");
        return -1;
    }
    return 0;
}

// même idée que bind sauf que on remplit la structure fourni en paramètre
int udp_make_addr(struct sockaddr_in *addr, const char* ip, uint16_t port) {
    // il faut mettre la structure à 0
    // Rappel : void *memset(void *s, int c, size_t n);
    // s : @ du deb de la zone
    // c : valeur à écrire dans chaque octet
    // n : nb d'octets
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    // texte "127.0.0.1" -> 4 octets en ordre réseau, écrits dans addr->sin_addr
    if (inet_pton(AF_INET, ip, &addr->sin_addr) != 1) {
        fprintf(stderr, "udp_make_addr : adresse IP invalide : %s\n", ip);
        return -1;
    }
    return 0;
}
