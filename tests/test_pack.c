/**
 * @file test_pack.c
 * @author Harini MINA
 * @brief Pour tester les fonctions de packet.c. Ce test a été généré par l'IA
 * @date 2026-10-25
 */
 
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../src/network/packet.h"

static void dump(const uint8_t *buf, int n) {
    for (int i = 0; i < n; i++) {
        printf("%02x ", buf[i]);
        if (i == 12) printf("| ");   // sépare l'en-tête du payload
    }
    printf("\n");
}

static void test_pack(void) {
    uint8_t buf[2048];
    const char *msg = "bonjour";
    size_t msg_len = strlen(msg);

    /* paquet normal */
    int n = sf_packet_pack(buf, sizeof(buf), SF_TYPE_DATA, 42,
                           (const uint8_t *)msg, (uint16_t)msg_len);
    printf("pack retourne %d : ", n);
    dump(buf, n);

    assert(n == (int)(sizeof(sf_header_t) + msg_len));
    assert(buf[0] == 0x53 && buf[1] == 0x46);
    assert(buf[2] == SF_TYPE_DATA);
    assert(buf[3] == 0 && buf[4] == 0 && buf[5] == 0 && buf[6] == 42);
    assert(buf[7] == 0 && buf[8] == 7);
    assert(memcmp(buf + 13, msg, msg_len) == 0);

    /* payload vide avec payload NULL (ex. un ACK) */
    n = sf_packet_pack(buf, sizeof(buf), SF_TYPE_ACK, 7, NULL, 0);
    assert(n == (int)sizeof(sf_header_t));

    /* buffer trop petit */
    uint8_t petit[16];
    n = sf_packet_pack(petit, sizeof(petit), SF_TYPE_DATA, 1,
                       (const uint8_t *)msg, (uint16_t)msg_len);
    assert(n == -1);

    /* payload au-dessus de la limite du protocole */
    static uint8_t gros[SF_MAX_PAYLOAD + 1];
    n = sf_packet_pack(buf, sizeof(buf), SF_TYPE_DATA, 1,
                       gros, (uint16_t)sizeof(gros));
    assert(n == -1);

    printf("test_pack OK\n");
}

static void test_unpack_roundtrip(void) {
    uint8_t buf[2048];
    const char *msg = "bonjour";
    size_t msg_len = strlen(msg);

    int n = sf_packet_pack(buf, sizeof(buf), SF_TYPE_DATA, 42,
                           (const uint8_t *)msg, (uint16_t)msg_len);

    sf_header_t h;
    const uint8_t *p = NULL;
    assert(sf_packet_unpack(buf, (size_t)n, &h, &p) == 0);

    assert(h.magic == SF_MAGIC);            // converti en ordre machine
    assert(h.type == SF_TYPE_DATA);
    assert(h.seq == 42);                    // converti en ordre machine
    assert(h.payload_len == msg_len);
    assert(h.crc32 == 0);
    assert(p == buf + sizeof(sf_header_t)); // pointe dans buf, pas de copie
    assert(memcmp(p, msg, msg_len) == 0);

    /* payload vide */
    n = sf_packet_pack(buf, sizeof(buf), SF_TYPE_FIN, 9, NULL, 0);
    assert(sf_packet_unpack(buf, (size_t)n, &h, &p) == 0);
    assert(h.type == SF_TYPE_FIN && h.seq == 9 && h.payload_len == 0);

    printf("test_unpack_roundtrip OK\n");
}

static void test_unpack_rejets(void) {
    uint8_t buf[2048];
    const char *msg = "bonjour";
    size_t msg_len = strlen(msg);
    sf_header_t h;
    const uint8_t *p = NULL;

    int n = sf_packet_pack(buf, sizeof(buf), SF_TYPE_DATA, 42,
                           (const uint8_t *)msg, (uint16_t)msg_len);

    /* moins de 13 octets : pas même un en-tête */
    assert(sf_packet_unpack(buf, 5, &h, &p) == -1);

    /* paquet tronqué d'un octet */
    assert(sf_packet_unpack(buf, (size_t)n - 1, &h, &p) == -1);

    /* octet en trop à la fin */
    buf[n] = 0xAA;
    assert(sf_packet_unpack(buf, (size_t)n + 1, &h, &p) == -1);

    /* mauvais magic */
    uint8_t copie[2048];
    memcpy(copie, buf, (size_t)n);
    copie[0] = 0x00;
    assert(sf_packet_unpack(copie, (size_t)n, &h, &p) == -1);

    /* payload_len annoncé supérieur à la limite du protocole */
    memcpy(copie, buf, (size_t)n);
    copie[7] = 0xFF; copie[8] = 0xFF;      // payload_len = 65535
    assert(sf_packet_unpack(copie, (size_t)n, &h, &p) == -1);

    /* payload_len incohérent avec la taille reçue */
    memcpy(copie, buf, (size_t)n);
    copie[8] = 100;                         // annonce 100 octets, il y en a 7
    assert(sf_packet_unpack(copie, (size_t)n, &h, &p) == -1);

    printf("test_unpack_rejets OK\n");
}

int main(void) {
    test_pack();
    test_unpack_roundtrip();
    test_unpack_rejets();
    printf("Tous les tests sont OK\n");
    return 0;
}