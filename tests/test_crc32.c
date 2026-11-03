/**
 * @file test_crc32.c
 * @author Harini MINA
 * @brief Pour tester les fonctions de crc32.c Ce test a été généré par l'IA
 */

#include <stdio.h>
#include <assert.h>
#include "../src/fec/crc32.h"

int main(void) {
    assert(sf_crc32((const uint8_t *)"", 0) == 0x00000000);
    assert(sf_crc32((const uint8_t *)"a", 1) == 0xA864DB20);
    assert(sf_crc32((const uint8_t *)"123456789", 9) == 0x89A1897F);

    // un bit inversé doit changer le CRC
    uint8_t d[4] = {0x12, 0x34, 0x56, 0x78};
    uint32_t ref = sf_crc32(d, 4);
    for (int i = 0; i < 8; i++) {
        d[2] ^= (uint8_t)(1u << i);               // on inverse un bit
        assert(sf_crc32(d, 4) != ref);
        d[2] ^= (uint8_t)(1u << i);               // on le remet
    }

    printf("test_crc32 OK\n");
    return 0;
}