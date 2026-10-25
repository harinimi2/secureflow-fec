/**
 * @file protocol.h
 * @author Harini MINA
 * @brief Correspond a l'étiquette FedEx (cf. COURS 1) : le modèle de l'entête avec ses cases à remplir
 * @date 2026-10-25
 */

#include <stdint.h>

#define SF_MAGIC        0x5346 // identifie un paquet de mon protocole

// les 4 types du paquet
#define SF_TYPE_HANDSHAKE 0x01
#define SF_TYPE_DATA      0x02
#define SF_TYPE_ACK       0x03
#define SF_TYPE_FIN       0x04

// limite de mon protoccole (valeur que j'ai choisi)
// la quantité max de données utiles qu'on mettra dans un seul de nos paquets
#define SF_MAX_PAYLOAD    1024 //bine inférieur à MTU =1500 octets donc ça laisse de la marge 

// structure de l'entête
typedef struct __attribute__((packed)) {
    uint16_t magic; // identifie le protocole
    uint8_t  type; // DATA pour les fragments de fichier, mais ACK, FIN ou HANDSHAKE pour les autres paquets
    uint32_t seq; // 0,1,2,3.... permet au récepteur de remettre les fragments dans l'ordre 
    uint16_t payload_len; // la charge utile (cad la partie data)
    uint32_t crc32; // pour détecter les erreurs (cf. COURS 2 soit avec matrice de parité, soit avec le checksum cf.COURS 1)
} sf_header_t;

// idée: dans la boucle d'envoi,on va créer un entête par fragment, en incrémentant seq à chaque tour 
// magic : 2 octets
// type : 1 octet
// seq : 4 octets
// payload_len : 2 octets
// crc32 : 4 octets

// donc au total, notre entête fait 13 octects
