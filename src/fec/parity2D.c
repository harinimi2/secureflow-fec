/**
 * @file hamming.c
 * @author Harini MINA
 * @brief Fonctions de détection des erreurs par le CRC (Cyclic Redundancy Check) qui est un type de checksum
 * @date 2026-11-03
 */

#include "parity2D.h"
#include <stdbool.h> // pour les booléens



// Rappel: VRC on doit faire en sorte qu'il y a le nombre de 1 paires sur chaque ligne
// LRC: on doit faire en sorte d'avoir des bits impairs sur chaque colonne
// calcule de bits de parité (VRC) pour un octet donné

// renvoie faux si le nb de bits 1 est impair, sinon true
bool is_pair(uint8_t b) {
    int cpt = 0;
    
    // compter le nombre de bits à 1 dans l'octet
    for (int i = 0; i < 8; i++) {
        if ((b >> i) & 1) {
            cpt++;
        }
    }
    
    // Retourne true si le total est pair, false s'il est impair
    return (cpt % 2 == 0);
}

// uint8_t calculate_vrc(uint8_t b) {
//     uint8_t parity = 0;
//     int i;
//     for(i = 0; i < 8; i++) {
        
//     }  
//     return parity;  
// }

// cette fonction découpe les données en blocs de 8 octets
// dans cette fonction on fera d'abord VRC puis du LRC
// fct principal
size_t sf_parity2d_encode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_size) {
    if((in == NULL) || (out == NULL) ||(in_len == 0)) {
        return 0;
    }
    // on fait d'abord le VRC ligne par ligne
    // on va donc compte le nb de 1 sur chaque ligne
    // si c'est pair on mettra un 0 en fin de ligne, sinon 1

    // ensuite on fera le LRC colonne par colonne
    // on va donc compte le nb de 1 sur chaque  colonne
    // et faire de même
    // attention le dernier bit du LRC prend en compte la colonne de VRC


    size_t out_idx = 0;
    // de base on a une matrice 8 x 8
    // mais la fct encode va donc renvoyer en sortie une matrice 9 x 9
    int col_ones[9] = {0}; // on utilise un tableau de 9 entiers dans lequel on stockera le compteurs de 1 PAR COLONNE

    // VRC
    size_t i;
    for(i = 0; i < in_len; i++) {
        if(out_idx >= out_size) { // sécurité dépassement buffer de sortie
            return 0;
        }

        uint8_t current_row = in[i];
        int row_ones = 0;

        // compte le nb de 1 sur chaque bit de la ligne courante
        int bit;
        for(bit = 0; bit < 8; bit++) { // car initialement on a le tableau 8 x 8, là on fait la partie VRC donc 8 x 9
            int b_val = (current_row >> bit) & 1;
            if (b_val == 1) {
                row_ones++;
                col_ones[bit]++; // maj la colonne correspondante pour le LRC
            }

            /////////// regrader à partir de ici

            // Déterminer le bit de VRC pour cette ligne :
        // "si c'est pair on mettra un 0 en fin de ligne, sinon 1"
        uint8_t vrc_bit = (row_ones % 2 == 0) ? 0 : 1;

        // Si le VRC est 1, on met à jour le compteur de la 9ème colonne (colonne VRC)
        if (vrc_bit == 1) {
            col_ones[8]++;
        }

        // Copie de la ligne dans le buffer de sortie (ou formatage incluant le VRC)
        out[out_idx++] = current_row;
    
        }


    }

    // -------------------------------------------------------------------------
    // 2. LRC : Traitement colonne par colonne
    // -------------------------------------------------------------------------
    if (out_idx >= out_size) {
        return 0;
    }

    uint8_t lrc_byte = 0;

    // On parcourt les 9 colonnes (8 colonnes de données + 1 colonne VRC)
    for (int col = 0; col < 9; col++) {
        // On compte le nb de 1 sur chaque colonne et on applique la même règle :
        // 0 si pair, 1 si impair
        uint8_t lrc_bit = (col_ones[col] % 2 == 0) ? 0 : 1;

        // On construit l'octet LRC final en plaçant chaque bit au bon endroit
        if (col < 8) {
            lrc_byte |= (lrc_bit << col);
        } else {
            // Le 9ème bit (colonne de VRC) est pris en compte comme spécifié
            // (tu peux l'ajuster selon la structure exacte de ton protocole de trame)
        }
    }

    // Ajout de l'octet de contrôle LRC à la fin du bloc
    out[out_idx++] = lrc_byte;

    return out_idx;

}
