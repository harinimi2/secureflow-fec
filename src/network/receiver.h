/**
 * @file receiver.h
 * @author Harini MINA
 * @brief Reception du paquet sous forme de plusieurs fragments
 * @date 2026-10-28
 */

#ifndef RECEIVER_H
#define RECEIVER_H

int sf_receive_file(int sock, const char *out_path); // reçoit les fragmts, envoie les ACK et restitue le fichier
// ATTENTION PIEGE: les fragments ne sont pas reçu forcément dans l'ordre

#endif