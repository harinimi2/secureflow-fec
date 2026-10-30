/**
 * @file crc32.h
 * @author Harini MINA
 * @brief Implémentation du CRC
 * @date 2026-10-30
 */

#ifndef CRC32_H
#define CRC32_H

#include <stdint.h> // uint8_t, uint32_t
#include <stddef.h> // size_t

uint32_t sf_crc32(const uint8_t *data, size_t len); //calcule le CRC-32 IEEE 802.3 d'un bloc

#endif
