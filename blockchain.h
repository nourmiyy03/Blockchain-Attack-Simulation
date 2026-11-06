#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/sha.h>


#define MAX_BLOCKS 100
#define DATA_SIZE 256
#define HASH_SIZE 65

#define DIFFICULTE 3


// ==========================================
// STRUCTURE D'UN BLOC
// ==========================================

typedef struct {

    int index;

    char data[DATA_SIZE];

    char previousHash[HASH_SIZE];

    char hash[HASH_SIZE];

    char validateur[30];

    unsigned long nonce;

} Block;


// ==========================================
// STRUCTURE DE LA BLOCKCHAIN
// ==========================================

typedef struct {

    Block blocks[MAX_BLOCKS];

    int count;

    // 1 = PoW
    // 2 = PoS

    int consensus;

} Blockchain;


// ==========================================
// STRUCTURE D'UN VALIDATEUR PoS
// ==========================================

typedef struct {

    char nom[30];

    int stake;

} Validateur;


// ==========================================
// STRUCTURE D'UN MINEUR PoW
// ==========================================

typedef struct {

    char nom[30];

    int hashrate;

} Miner;


// ==========================================
// FONCTIONS BLOCKCHAIN
// ==========================================

// SHA-256
void calculerHash(
    const char *texte,
    char hash[HASH_SIZE]
);


// Hash complet d'un bloc
void calculerHashBloc(
    Block *block
);


// Vérifier la difficulté PoW
int hashValide(
    const char *hash
);


// Proof of Work
void proofOfWork(
    Block *block
);


// Initialiser une blockchain
void initialiserBlockchain(
    Blockchain *chain
);


// Ajouter un bloc PoW
void ajouterBlocPoW(
    Blockchain *chain,
    const char *data
);


// Choisir un validateur PoS
int choisirValidateur(
    Validateur validateurs[],
    int n
);


// Ajouter un bloc PoS
void ajouterBlocPoS(
    Blockchain *chain,
    const char *data,
    Validateur validateurs[],
    int n
);


// Vérifier une blockchain
int verifierBlockchain(
    const Blockchain *chain
);


// Afficher une blockchain
void afficherBlockchain(
    const Blockchain *chain
);


// ==========================================
// FONCTIONS POUR LES SIMULATIONS D'ATTAQUES
// ==========================================

// Miner un bloc sur une blockchain donnée
void minerUnBloc(
    Blockchain *chain,
    const char *data,
    const char *mineur
);


// Copier un bloc
void copierBloc(
    Block *destination,
    const Block *source
);


void simulerAttaque51(void);

// Selfish Mining
void simulerSelfishMining(void);

#endif