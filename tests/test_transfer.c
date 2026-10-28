/**
 * @file test_transfer.c
 * @author Harini MINA
 * @brief Pour tester les fonctions de sender.c et receiver.c Ce test a été généré par l'IA
 * @date 2026-10-26
 */

 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include "../src/network/udp_socket.h"
#include "../src/network/packet.h"
#include "../src/network/sender.h"
#include "../src/network/receiver.h"

#define PORT_SRV 5100   // port du serveur (receiver)
#define PORT_PRX 5101   // port du proxy qui perd des paquets

/* ---------- utilitaires ---------- */

// crée un fichier de 'size' octets au contenu pseudo-aléatoire reproductible
static void make_file(const char *path, size_t size) {
    FILE *f = fopen(path, "wb");
    assert(f != NULL);
    unsigned x = 12345;
    for (size_t i = 0; i < size; i++) {
        x = x * 1103515245u + 12345u;
        fputc((int)((x >> 16) & 0xFF), f);
    }
    fclose(f);
}

// 1 si les deux fichiers ont exactement le même contenu
static int files_equal(const char *a, const char *b) {
    FILE *fa = fopen(a, "rb"), *fb = fopen(b, "rb");
    if (!fa || !fb) { if (fa) fclose(fa); if (fb) fclose(fb); return 0; }
    int egal = 1;
    int ca, cb;
    do {
        ca = fgetc(fa);
        cb = fgetc(fb);
        if (ca != cb) { egal = 0; break; }
    } while (ca != EOF);
    fclose(fa);
    fclose(fb);
    return egal;
}

/* ---------- proxy qui perd des paquets ---------- */
/* client -> proxy -> serveur, et retour.
   Perd 1 DATA sur 3, et 1 ACK sur 4 parmi ceux qui répondent à un DATA.
   Ne touche jamais au FIN ni à son ACK. */
static void proxy_loop(int prx) {
    struct sockaddr_in srv_addr, client_addr, from;
    int have_client = 0, nb_data = 0, nb_ack = 0;
    uint8_t last_client_type = 0;
    uint8_t buf[2048];

    udp_make_addr(&srv_addr, "127.0.0.1", PORT_SRV);
    udp_set_timeout(prx, 200);

    while (1) {
        int n = udp_recv(prx, buf, sizeof(buf), &from);
        if (n < 3) continue;                       // délai dépassé ou trop court

        if (from.sin_port == htons(PORT_SRV)) {    // serveur -> client
            if (!have_client) continue;
            if (last_client_type == SF_TYPE_DATA && ++nb_ack % 4 == 0)
                continue;                          // ACK perdu
            udp_send(prx, &client_addr, buf, (size_t)n);
        } else {                                   // client -> serveur
            client_addr = from;
            have_client = 1;
            last_client_type = buf[2];             // octet 2 = type du paquet
            if (buf[2] == SF_TYPE_DATA && ++nb_data % 3 == 0)
                continue;                          // DATA perdu
            udp_send(prx, &srv_addr, buf, (size_t)n);
        }
    }
}

/* ---------- un transfert complet ---------- */
// retourne 1 si tout s'est bien passé
static int transfer_case(size_t size, int lossy) {
    const char *orig = "tmp_original.bin";
    const char *recu = "tmp_recu.bin";

    make_file(orig, size);
    remove(recu);

    // sockets liées AVANT le fork : pas de course entre émetteur et récepteur
    int srv = udp_open();
    assert(srv >= 0 && udp_bind(srv, PORT_SRV) == 0);
    int prx = -1;
    if (lossy) {
        prx = udp_open();
        assert(prx >= 0 && udp_bind(prx, PORT_PRX) == 0);
    }

    fflush(stdout);

    pid_t pid_rcv = fork();
    assert(pid_rcv >= 0);
    if (pid_rcv == 0) {                            // fils : le serveur
        if (prx >= 0) close(prx);
        _exit(sf_receive_file(srv, recu) == 0 ? 0 : 1);
    }

    pid_t pid_prx = -1;
    if (lossy) {
        pid_prx = fork();
        assert(pid_prx >= 0);
        if (pid_prx == 0) {                        // fils : le proxy
            close(srv);
            proxy_loop(prx);
            _exit(0);
        }
    }
    close(srv);
    if (prx >= 0) close(prx);

    // père : le client
    int client = udp_open();
    assert(client >= 0);
    struct sockaddr_in dst;
    assert(udp_make_addr(&dst, "127.0.0.1", lossy ? PORT_PRX : PORT_SRV) == 0);
    int res = sf_send_file(client, &dst, orig);
    close(client);

    int status = 0;
    waitpid(pid_rcv, &status, 0);
    if (lossy) {
        kill(pid_prx, SIGTERM);
        waitpid(pid_prx, NULL, 0);
    }

    int ok = (res == 0) && WIFEXITED(status) && WEXITSTATUS(status) == 0
             && files_equal(orig, recu);
    printf("  %s %6zu octets, %s\n", ok ? "[OK]" : "[ÉCHEC]", size,
           lossy ? "avec pertes" : "sans perte");
    remove(orig);
    remove(recu);
    return ok;
}

int main(void) {
    int ok = 1;

    printf("Transferts sans perte :\n");
    size_t tailles[] = {0, 7, 1024, 1025, 2500, 10000};
    for (size_t i = 0; i < sizeof(tailles) / sizeof(tailles[0]); i++)
        ok &= transfer_case(tailles[i], 0);

    printf("Transferts avec pertes (1 DATA sur 3, 1 ACK sur 4) :\n");
    ok &= transfer_case(2500, 1);
    ok &= transfer_case(10000, 1);

    printf("Cas d'erreur :\n");
    int client = udp_open();
    struct sockaddr_in dst;
    udp_make_addr(&dst, "127.0.0.1", 5199);        // personne n'écoute ici
    make_file("tmp_original.bin", 100);
    int r1 = sf_send_file(client, &dst, "tmp_original.bin");
    printf("  %s serveur absent : retourne -1 après les retransmissions\n",
           r1 == -1 ? "[OK]" : "[ÉCHEC]");
    int r2 = sf_send_file(client, &dst, "fichier_inexistant.bin");
    printf("  %s fichier inexistant : retourne -1\n", r2 == -1 ? "[OK]" : "[ÉCHEC]");
    ok &= (r1 == -1) && (r2 == -1);
    close(client);
    remove("tmp_original.bin");

    printf(ok ? "\nTous les tests de transfert sont OK\n"
              : "\nAu moins un test a échoué\n");
    return ok ? 0 : 1;
}