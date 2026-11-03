/**
 * @file crc32.h
 * @author Harini MINA
 * @brief Fonctions de détection des erreurs par le CRC (Cyclic Redundancy Check) qui est un type de checksum
 * @date 2026-10-30
 */

#include "crc32.h"

#define CRC32_POLY 0x04C11DB7 // polynôme générateur g(x) du CRC-32, sans le terme x^32
#define CRC32_TOP  0x80000000 // bit de tête du registre (bit 31)


// idée : ce que fait la fct c'est qu'elle prend un bloc d'octets et retourne un nombre de 32 bis, son "empreinte"
// CRC est le reste d'une division polynomiale module 2 avec seulement des XOR et des décalages (COURS 2)

// on fait comme dans le td2
// Rappel :  les opérations d'addition et de soustraction sans retenue sont des XOR (^ en C)
uint32_t sf_crc32(const uint8_t *data, size_t len) {
    uint32_t reste = 0x00000000; // le reste vaut 0 pour l'instant
    size_t i;
    for(i = 0; i < len; i++) { // on regarde octet par octet dans le data
        reste ^= ((uint32_t)data[i]) << 24; // on fait un xor sur le ie élément de la data
        // 32 (car le registre reste fait 32 bits)- 8 (car un octet de data fait 8 bits) = 24
        
        for(int bit = 0; bit < 8; bit++) {
            if(reste & CRC32_TOP) {

                // on décale d'abord (cad le bit de tête disparaît)
                // ensuite on a fait le XOR avec le reste du polynôme

                reste = (reste << 1) ^CRC32_POLY;
            }

            else { // si le bit de tête vaut 0, on avance/l'ignore
                reste = reste << 1;
            }
        }
    }
    return reste; 
}