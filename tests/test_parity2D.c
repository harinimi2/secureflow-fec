/**
 * @file test_parity2D.c
 * @author Harini MINA
 * @brief Pour tester les fonctions de test_parity2D.c Ce test a été généré par l'IA
 * @date 2026-11-12
 */


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

// Déclarations des fonctions du module fec
bool is_pair(uint8_t b);
size_t sf_parity2d_encode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_size);
int sf_parity2d_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_size, size_t *corrected);

int main(void) {
    printf("[TEST] Lancement des tests unitaires pour la parité 2D...\n");

    // 1. Test de la fonction is_pair
    assert(is_pair(0b1100) == true);  // Deux bits à 1 -> pair
    assert(is_pair(0b1000) == false); // Un bit à 1 -> impair
    printf("[OK] Test is_pair réussi.\n");

    // 2. Test encode / decode sans erreur
    uint8_t original_data[] = "SecureFlow";
    size_t in_len = strlen((char *)original_data);
    
    uint8_t encoded_buf[256];
    size_t encoded_len = sf_parity2d_encode(original_data, in_len, encoded_buf, sizeof(encoded_buf));
    
    if (encoded_len == 0) {
        fprintf(stderr, "[ERREUR] Échec de l'encodage parité 2D.\n");
        return 1;
    }
    printf("[OK] Encodage 2D réussi (taille encodée : %zu octets).\n", encoded_len);

    uint8_t decoded_buf[256] = {0};
    size_t corrected_bits = 0;
    int decode_res = sf_parity2d_decode(encoded_buf, encoded_len, decoded_buf, sizeof(decoded_buf), &corrected_bits);
    
    if (decode_res != 0 || corrected_bits != 0) {
        fprintf(stderr, "[ERREUR] Échec du décodage sans erreur.\n");
        return 1;
    }
    
    if (memcmp(original_data, decoded_buf, in_len) != 0) {
        fprintf(stderr, "[ERREUR] Les données décodées ne correspondent pas à l'original.\n");
        return 1;
    }
    printf("[OK] Décodage sans erreur validé.\n");

    // 3. Test avec injection d'une erreur d'un bit (correction automatique)
    uint8_t corrupted_buf[256];
    memcpy(corrupted_buf, encoded_buf, encoded_len);
    
    // Inversion d'un bit sur le premier octet (injection de bruit simulé)
    corrupted_buf[0] ^= 0x01; 

    uint8_t repaired_buf[256] = {0};
    size_t corrected_count = 0;
    
    int repair_res = sf_parity2d_decode(corrupted_buf, encoded_len, repaired_buf, sizeof(repaired_buf), &corrected_count);
    
    if (repair_res != 0) {
        fprintf(stderr, "[ERREUR] Le décodeur n'a pas pu corriger l'erreur unique.\n");
        return 1;
    }
    
    if (corrected_count == 0) {
        fprintf(stderr, "[ERREUR] Aucun bit n'a été corrigé alors qu'une erreur a été injectée.\n");
        return 1;
    }

    if (memcmp(original_data, repaired_buf, in_len) != 0) {
        fprintf(stderr, "[ERREUR] Le message réparé est incorrect.\n");
        return 1;
    }
    printf("[OK] Correction d'erreur unique validée avec succès (bits corrigés : %zu).\n", corrected_count);

    printf("[SUCCES] Tous les tests unitaires de parité 2D ont passé avec succès !\n");
    return 0;
}