/**
 * @file parity2D.h
 * @author Harini MINA
 * @brief Fonctions de détection des erreurs par le CRC (Cyclic Redundancy Check) qui est un type de checksum
 * @date 2026-11-03
 */



#ifndef PARITY2D_H
#define PARITY2D_H

#include <stdint.h> // uint8_t
#include <stddef.h> // size_t

#define PARITY2D_DATA_BYTES 8 // octets de data par bloc (matrice 8 x 8 bits)
#define PARITY2D_CODED_BYTES 10 // 8 octets de data + 1 octet VRC + 1 octet LRC

uint8_t calculate_vrc(uint8_t b); // calcule de bits de parité (VRC) pour un octet donné
size_t sf_parity2d_encode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_size); // inclus VRC et LRC

#endif