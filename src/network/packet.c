/**
 * @file packet.c
 * @author Harini MINA
 * @brief implémentation des opérations sur les paquets : encapsulation et désencapsulation
 * @date 2026-10-25
 */

#include <string.h> // pour memcpy notamment
#include <stdio.h>
#include "packet.h" // pas besoin de faire include "protocol.h" car c déjà fait dans le packet.h

// pour la partie écriture dans le buffer si écriture possible
// pour les fct htons et htonl : convertissent un nb de l'ordre de notre machine vers l'ordre du réseau, avt d'envoyer
// htons : host to network short
// htonl : host to network long

#include <arpa/inet.h>

// idée: Rappel: un paquet contient les données elle-même

int sf_packet_pack(uint8_t *buf, size_t buf_size, uint8_t type, uint32_t seq, const uint8_t *payload, uint16_t payload_len) {

    // on écrit dans le buf l'entête et le playload
    // deux risques :

    // soit dépasse la limite imposée par notre protocole
    if(payload_len > SF_MAX_PAYLOAD) {
        printf("Erreur ! La taille de la charge utile dépasse la limite de notre protocole!\n");
        return -1;
    }

    //soit dépasse la taille du buffer
    // calcule la taille total du paquet (entête + charge utile)
    size_t tot = sizeof(sf_header_t) + payload_len;

    if(tot > buf_size) {
        printf("Erreur ! Notre écriture dépasse la taille du buffer !\n");
        return -1;
    }

    //commençons l'écriture en suivant le protocole

    // creattion d'une entête donc dans le buf on a [entête]
    // conersion machine -> réseau
    sf_header_t nouv;
    nouv.magic       = htons(SF_MAGIC);
    nouv.type        = type;
    nouv.seq         = htonl(seq);
    nouv.payload_len = htons(payload_len);
    nouv.crc32       = 0; // qui sera calculé par la suite
    
    // on veut rajouter la charge utile après l'entête
    // comme ça dans le buf on a : [entête][payload]

    // on va donc copier l'entête qu'on a crée juste au dessus vers le buf
    memcpy(buf, &nouv, sizeof(sf_header_t));
    // source : @ de nouv dans la mémo
    // desti : buf

    // il faut maintenant dire que la charge utile doit être placée après l'entête
    // Attention pour dire que charge utile APRÈS entête il faut faire buf + taille de entête
    if(payload_len > 0 ) { // si charge utile non vide
        memcpy(buf + sizeof(sf_header_t), payload, payload_len);
    }

    // retourner la taille total -> paquet complet prêt à être envoyé
    return (int)tot;
}

// faot le chemi, inverse :
// lit le paquet reçu, le valide et rend l'entête et l'emplacement du payload
int sf_packet_unpack(const uint8_t *buf, size_t len, sf_header_t *hdr, const uint8_t **payload) {
    // verif sur le buf est assez grd pour contenir une entête
    // on sait que on va lire 13 octets (taille entête) dans le buf
    if(len < sizeof(sf_header_t)) {
        printf("Erreur ! Paquet trop court : %zu octets reçus, %zu attendus au minimum\n", len, sizeof(sf_header_t));
        return -1; // lecture invalide
    }

    // on va recopier mais chemin inverse que le pack
    // recopie de header
    memcpy(hdr, buf, sizeof(sf_header_t));

    // conversion réseau -> machine 
    hdr->magic       = ntohs(hdr->magic);
    hdr->seq         = ntohl(hdr->seq);
    hdr->payload_len = ntohs(hdr->payload_len);
    hdr->crc32       = ntohl(hdr->crc32);

     if (hdr->magic != SF_MAGIC) {
        printf("Erreur ! Magic invalide : 0x%04x\n", hdr->magic);
        return -1;
    }
    if (hdr->payload_len > SF_MAX_PAYLOAD) {
        printf("Erreur ! payload_len annoncé trop grand : %u\n", hdr->payload_len);
        return -1;
    }
    if (len != sizeof(sf_header_t) + hdr->payload_len) {
        printf("Erreur ! Taille incohérente : %zu octets reçus, %zu annoncés\n",
               len, sizeof(sf_header_t) + hdr->payload_len);
        return -1;
    }

    // pas besoin de recopier la charge utile, on pointe vers lui, juste après l'entête
    *payload = buf + sizeof(sf_header_t);
    return 0; // la lecture est valide


}