/**
 * @file receiver.c
 * @author Harini MINA
 * @brief Implémentation reception du paquet sous forme de plusieurs fragments
 * @date 2026-10-28
 */

#include "receiver.h"
#include "packet.h"// sf_packet_pack, sf_packet_unpack, SF_TYPE_*, SF_MAX_PAYLOAD
#include "udp_socket.h" // udp_recv, udp_send
#include <stdio.h>  // fopen, fseek, fwrite, fclose, perror

#define SF_RECV_TIMEOUT_MS 500 // délai d'attente d'un paquet
#define SF_MAX_IDLE 20 // délais consécutifs sans paquet avant d'abandonner

int sf_receive_file(int sock, const char *out_path) {
    // fichier de sortie
    FILE *f = fopen(out_path, "wb"); // en mode écriture binaire

    if(f == NULL) {
        perror("sf_receive_file");
        return -1;
    }

    uint8_t rep[sizeof(sf_header_t) + SF_MAX_PAYLOAD]; // reçoit les paquets
    uint8_t ack[sizeof(sf_header_t)]; // un ACK n'a que l'en-tête
    struct sockaddr_in from; // expéditeur, rempli par udp_recv
    sf_header_t hdr; // rempli par sf_packet_unpack
    const uint8_t *payload; // rempli par sf_packet_unpack
    int inactif = 0; // délais consécutifs sans rien recevoir

    // le principe est presque pareil que avec sender.c

    // sans délai, udp_recv bloquerait indifinement si l'émetteur disparait 
    if(udp_set_timeout(sock, SF_RECV_TIMEOUT_MS) == -1) {
        fclose(f);
        return -1;
    }

    // on réassemble les fragments mais avt cela faut vérif certaines erreurs
    while(1) {
        int n = udp_recv(sock, rep, sizeof(rep), &from);

        if (n == 0) { // délai dépassé : rien reçu
            inactif++;
            if (inactif >= SF_MAX_IDLE) {
                fprintf(stderr, "sf_receive_file : plus de nouvelles de l'émetteur\n");
                    fclose(f);
                    return -1;
            }
            
            continue; // on réécoute
            
            } else if (n < 0) { // vraie erreur de réception
                
                fclose(f);
                return -1;
            }

            inactif = 0; // un paquet est arrivé

            // cette partie a été par l'aide de l'IA
            // paquet invalide (pas de notre protocole, tronqué...) : on l'ignore
            if (sf_packet_unpack(rep, (size_t)n, &hdr, &payload) != 0) {
                continue;
            }

            if (hdr.type == SF_TYPE_DATA) {
                // le fragment seq va à l'offset seq * SF_MAX_PAYLOAD (cast avant la multiplication)
                long offset = (long)hdr.seq * SF_MAX_PAYLOAD;
                if (fseek(f, offset, SEEK_SET) != 0 || fwrite(payload, 1, hdr.payload_len, f) != hdr.payload_len) {
                    perror("sf_receive_file : écriture");
                    fclose(f);
                    return -1;
                }

                // on acquitte toujours, même un doublon
                int t = sf_packet_pack(ack, sizeof(ack), SF_TYPE_ACK, hdr.seq, NULL, 0);
                if (t == -1 || udp_send(sock, &from, ack, (size_t)t) == -1) {
                    fclose(f);
                    return -1;
                }
            } else if (hdr.type == SF_TYPE_FIN) {
                int t = sf_packet_pack(ack, sizeof(ack), SF_TYPE_ACK, hdr.seq, NULL, 0);
                if (t == -1 || udp_send(sock, &from, ack, (size_t)t) == -1) {
                    fclose(f);
                    return -1;
                }
                fclose(f);
                return 0; // transfert terminé
            }
    }
}
