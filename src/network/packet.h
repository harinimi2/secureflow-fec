/**
 * @file packet.h
 * @author Harini MINA
 * @brief toutes les opérations sur les paquets
 * cad : coller étiquette sur le paquet, lire le message dans le paquet
 * @date 2026-10-25
 */

 #ifndef PACKET_H
#define PACKET_H

#include <stddef.h>
#include <stdint.h>
#include "protocol.h"

// construire un paquet dans buffer : entête et charge utile
// retourne la taille totale en octets, ou 1 en cas d'erreur
// cad encapsulation du paquet
int sf_packet_pack(uint8_t *buf, size_t buf_size, uint8_t type, uint32_t seq, const uint8_t *payload, uint16_t payload_len);

// lit un paquet reçu
// remplut entête et pointe la charge utile vers les données dans le buufer
// retourne 0 si c'est 0K, -1 si c'est invalide
// cad désencapsulation du paquet

int sf_packet_unpack(const uint8_t *buf, size_t len, sf_header_t *hdr, const uint8_t **payload);
// len est la longueur du paquet entier

#endif
