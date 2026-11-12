/**
 * @file hamming.c
 * @author Harini MINA
 * @brief Fonctions de détection des erreurs par le CRC (Cyclic Redundancy Check) qui est un type de checksum
 * @date 2026-11-12
 */

#include "parity2D.h"
#include <stdbool.h> // pour les booléens

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


// Rappel: VRC on doit faire en sorte qu'il y a le nombre de 1 paires sur chaque ligne
// LRC: on doit faire en sorte d'avoir des bits impairs sur chaque colonne
// calcule de bits de parité (VRC) pour un octet donné

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

        // compte le nb de 1 sur chaque bit de la ligne courante
        int bit;
        for(bit = 0; bit < 8; bit++) { // car initialement on a le tableau 8 x 8, là on fait la partie VRC donc 8 x 9
            int b_val = (current_row >> bit) & 1;
            if (b_val == 1) {
                col_ones[bit]++; // maj la colonne correspondante pour le LRC
            }
        } 

        // Déterminer le bit de VRC pour cette ligne :
        // si c'est pair on mettra un 0 en fin de ligne, sinon 1
        uint8_t vrc_bit;
        if(is_pair(current_row)) { // cas pair vérifié via is_pair
            vrc_bit = 0;
        }

        else {
            vrc_bit = 1; // cas impair
            // si le VRC est 1, on met à jour le compteur de la 9ème colonne (colonne VRC)
            col_ones[8]++;
        }

        // copie de la ligne dans le buffer de sortie (ou formatage incluant le VRC)
        out[out_idx++] = current_row;
    }

    // LRC : traitement colonne par colonne
    if (out_idx >= out_size) {
        return 0;
    }

    uint16_t lrc_word = 0; // adapté en uint16_t pour stocker les 9 bits (8 données + 1 VRC)

    // on parcourt les 9 colonnes (8 colonnes de données + 1 colonne VRC)
    for (int col = 0; col < 9; col++) {
        // on compte le nb de 1 sur chaque colonne et on fait pareil sur les colonnes
        // 0 si pair, 1 si impair
        uint8_t lrc_bit;
        
        if(col_ones[col] % 2 == 0) {
            lrc_bit = 0; // cas pair
        }
        else {
            lrc_bit = 1; // cas impair
        }

        // On construit l'octet LRC final en plaçant chaque bit au bon endroit
        if (col < 8) {
            lrc_word |= (lrc_bit << col);
        } else {
            // Le 9ème bit (colonne de VRC) est pris en compte comme spécifié
            // (tu peux l'ajuster selon la structure exacte de ton protocole de trame)
            lrc_word |= (lrc_bit << col);
        }
    }

    // Ajout de l'octet de contrôle LRC à la fin du bloc
    if (out_idx + 1 < out_size) {
        out[out_idx++] = (uint8_t)(lrc_word & 0xFF);
        out[out_idx++] = (uint8_t)((lrc_word >> 8) & 0xFF);
    } else if (out_idx < out_size) {
        out[out_idx++] = (uint8_t)(lrc_word & 0xFF);
    } else {
        return 0;
    }

    return out_idx;
}

// le recepteur ne va pas seulement dire qu'il y a une erreur, il va aussi trouver la position exacte de l'erreur (parcours de la matrice pour trouver la position exacte)
// le recepteur corrigera automatiquement l'erreur (cad inversion du bit selon la parité)

// decodage 2d (vrc + lrc) avec correction auto d'une err
int sf_parity2d_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_size, size_t *corrected) {
    // verif args de base
    if ((in == NULL) || (out == NULL) || (in_len < 2) || (corrected == NULL)) {
        return -1;
    }
    *corrected = 0;

    size_t data_len = in_len - 1; // nb lignes data (le dernier octet c'est le lrc)
    if (data_len > out_size) {
        return -1; // buf out trop ptit
    }

    // copie des data ds out pr pouvoir les corriger si besoin
    for (size_t i = 0; i < data_len; i++) {
        out[i] = in[i];
    }

    uint8_t received_lrc = in[in_len - 1]; // lrc recu du reseau
    uint8_t calc_lrc = 0; // lrc calcule en direct

    int bad_row = -1;
    int row_err_nb = 0;
    
    // tab pr stocker si la ligne est ok ou pas
    // (on alloue ou on traite a la volee selon la taille, ici direct ds la boucle)

    // verif vrc ligne par ligne
    for (size_t i = 0; i < data_len; i++) {
        uint8_t row = out[i];
        
        // calc lrc global par xor des lignes
        calc_lrc ^= row;

        // verif parite de la ligne (vrc)
        if (!is_pair(row)) {
            row_err_nb++;
            bad_row = (int)i; // idx de la ligne corrompue
        }
    }

    // verif lrc col par col (comparaison lrc calc vs recu)
    uint8_t lrc_diff = calc_lrc ^ received_lrc;

    // cas 0 err : tt est ok
    if (row_err_nb == 0 && lrc_diff == 0) {
        return 0;
    }

    // cas 1 err unique (1 ligne err et des diffs sur le lrc)
    if (row_err_nb == 1 && lrc_diff != 0) {
        // lrc_diff donne exactmt quelles cols ont un souci
        // on cherche le bit erronee ds la mauvaise ligne
        uint8_t row = out[bad_row];
        uint8_t corrected_row = row ^ lrc_diff; // inversion du bit fautif

        out[bad_row] = corrected_row;
        (*corrected)++; // incremente le nb de corrections

        return 0; // succes apres correction
    }

    // si trop d'err ou cas nn gerable par parity 2d simple
    return -1;
}