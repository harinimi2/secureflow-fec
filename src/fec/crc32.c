/**
 * @file crc32.h
 * @author Harini MINA
 * @brief Fonctions de détection des erreurs par le CRC (Cyclic Redundancy Check) qui est un type de checksum
 * @date 2026-10-30
 */

#include "crc32.h"

// idée : ce que fait la fct c'est qu'elle prend un bloc d'octets et retourne un nombre de 32 bis, son "empreinte"
// CRC est le reste d'une division polynomiale module 2 avec seulement des XOR et des décalages (COURS 2)

// on fait comme dans le td2
uint32_t sf_crc32(const uint8_t *data, size_t len) {
    uint32_t reste = 0x00000000; // le reste vaut 0
    size_t i;
    for(i = 0; i < len; i++) { // on regarde octet par octet dans le data
        crc ⁼ ((uint32_t))
        for(int bit = 0; bit < 8; bit++) {
            // à contin dm
        }
    }
    return reste; 
}