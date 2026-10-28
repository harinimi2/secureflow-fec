/**
 * @file sender.c
 * @author Harini MINA
 * @brief sender est le module qui envoie un fichier complet à travers UDP, de façon fiable
 * C'est le client : il possède le fichier et l'envoie
 * @date 2026-10-26
 */

#include "sender.h"
#include "sender.h"
#include "packet.h" // sf_packet_pack, sf_packet_unpack, sf_header_t, SF_TYPE_*
#include "udp_socket.h" // udp_send, udp_recv, udp_set_timeout car la source est la socket
// la destination est le buffer

#include <stdio.h> // fopen, fread, fclose, perror, printf
#include <string.h> // memcmp, memcpy (si vous en avez besoin)


#define SF_TIMEOUT_MS 300
#define SF_MAX_RETRIES 5

// seq sert de reconnaître le ACK qui correspond au paquet qu'on vient d'envoyer
// ex: le paquet 3, l'ACK qui lui correspond est 3
// si on a un ACK 2, on sait que ce n'est pas le ACK du packet 3


int sf_send_packet_reliable(int sock, const struct sockaddr_in *dst, const uint8_t *buf, size_t len, uint32_t seq, int max_retries) {
    // tant qu'il reste des tentatives envoyer le paquet, attendre une réponse au plus d'un délai, et s'arrêter dès qu'on reçoit l'ACK portant le bon seq
    // Rappel: le délai (le tampo) est envoyé par la source donc gérer par la fonction udp_set_timeout
    // la source ici est la socket
    // la destination est le buffer
    // la fonction udp_recv signale quand le délai est dépassé (le moment où on doit retransmettre)
    
    uint8_t rep[sizeof(sf_header_t) + SF_MAX_PAYLOAD]; // buffer de réception de l'ACK
    struct sockaddr_in from; // udp_recv y écrit l'expéditeur
    sf_header_t hdr; // sf_packet_unpack y range l'entête
    const uint8_t *payload;// sf_packet_unpack y met l'adresse du payload

    int essai;
    for(essai = 0; essai <= max_retries; essai++) {
        // on (ré)envoie le packet 
        // Echo request
        if( udp_send(sock, dst, buf, len) == -1) {
            return -1;
        }
        // on attend le bon ACK 
        while(1) { // condition toujours vrai donc on attend tout le temps sauf quand le délai est dépassé
            int delai = udp_recv(sock, rep, sizeof(rep), &from);
            
            if(delai == 0) { // si le délai est dépassé
                break;
            }

            else if(delai < 0) {
                return -1; // erreur car un délai ne peur être négatif
            }

            // si le "bon" paquet est arrivé il faut le désencapsuler
            // on vérifie si c'est le bon ACK avec seq
            // on vérifie si c'est bien un paquet qui a une entête ACK
            // sf_packet_unpack() renvoie 0 si la lecture est valide
            
            // Echo reply
            if((sf_packet_unpack(rep,(size_t)delai, &hdr, &payload) == 0) && (hdr.type == SF_TYPE_ACK) && (hdr.seq == seq)){ 
                return 0; // ok c'est le bon ACK
            }

            // sinon on ignore ce paquet et on continue d'attendre
        }

    }
    return -1; // erreur
}

// on doti découper le fichier en fragments
// on choisit le premier fragement à 0 et on incrémente
// FIN porte le seq suivant le dernier fragment (donc le nb tot de fragmts, il est acquitté comme un DATA)
int sf_send_file(int sock, const struct sockaddr_in *dst, const char *path) {
    
    FILE* f = fopen(path, "rb"); // on ouvre le fichier en mode lecture binaire
    if (f == NULL) {
        perror("sf_send_file");
        return -1;
    }

    // un bloc lu dans le fichier
    uint8_t data[SF_MAX_PAYLOAD]; // tableau data de longueur max de la charge utile
    uint8_t pkt[sizeof(sf_header_t) + SF_MAX_PAYLOAD];   // en-tête + la charge utile (cad le paquet construit)
    uint32_t seq = 0; // on init ce fragment à 0
    size_t n; // le nombre de octest que fread a lu

     // délai d'attente de l'ACK, réglé une seule fois avant la boucle
    if (udp_set_timeout(sock, SF_TIMEOUT_MS) == -1) {
        fclose(f);
        return -1;
    }

    // un tour boucle de lecture = un fragment data
    while ((n = fread(data, 1, sizeof(data), f)) > 0) {
        int taille = sf_packet_pack(pkt, sizeof(pkt), SF_TYPE_DATA, seq, data, (uint16_t)n);

        // on traite tous les cas des erreurs possibles
        if (taille == -1) {
            fclose(f);
            return -1; // erreur 
        }

        if (sf_send_packet_reliable(sock, dst, pkt, (size_t)taille,seq, SF_MAX_RETRIES) == -1) {
            fclose(f);
            return -1; // erreur
        }
        seq++; // on coupe le fragment à la sortie de boucle
    }

    // fread a rendu 0 : fin de fichier ou erreur de lecture ?
    if (ferror(f)) {
        perror("sf_send_file : lecture");
        fclose(f);
        return -1;
    }

    // paquet FIN : pas de données, seq = numéro suivant le dernier fragment
    int taille = sf_packet_pack(pkt, sizeof(pkt), SF_TYPE_FIN, seq, NULL, 0);
    if (taille == -1 ||
        sf_send_packet_reliable(sock, dst, pkt, (size_t)taille,
                                seq, SF_MAX_RETRIES) == -1) {
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}