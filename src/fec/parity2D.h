/**
 * @file parity2D.h
 * @author Harini MINA
 * @brief Fonctions de détection des erreurs par le CRC (Cyclic Redundancy Check) qui est un type de checksum
 * @date 2026-11-12
 */



#ifndef PARITY2D_H
#define PARITY2D_H

#include <stdint.h> // uint8_t
#include <stddef.h> // size_t

#define PARITY2D_DATA_BYTES 8 // octets de data par bloc (matrice 8 x 8 bits)
#define PARITY2D_CODED_BYTES 10 // 8 octets de data + 1 octet VRC + 1 octet LRC

size_t sf_parity2d_encode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_size); // inclus VRC et LRC
int sf_parity2d_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_size, size_t *corrected); // récepteur dit si il y a erreur, et dans ce cas il va aussi corriger automatiquement l'erreur

#endif