#include "blockchain.h"


// ==========================================
// CALCULER SHA-256
// ==========================================

void calculerHash(
    const char *texte,
    char hash[HASH_SIZE]
) {

    unsigned char digest[SHA256_DIGEST_LENGTH];

    SHA256(
        (const unsigned char *)texte,
        strlen(texte),
        digest
    );

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {

        sprintf(
            hash + i * 2,
            "%02x",
            digest[i]
        );
    }

    hash[64] = '\0';
}


// ==========================================
// CALCULER LE HASH D'UN BLOC
// ==========================================

void calculerHashBloc(Block *block) {

    char texte[1024];

    snprintf(
        texte,
        sizeof(texte),

        "%d|%s|%s|%s|%lu",

        block->index,
        block->data,
        block->previousHash,
        block->validateur,
        block->nonce
    );

    calculerHash(
        texte,
        block->hash
    );
}


// ==========================================
// VERIFIER LA DIFFICULTE
// ==========================================

int hashValide(const char *hash) {

    for (int i = 0; i < DIFFICULTE; i++) {

        if (hash[i] != '0') {
            return 0;
        }
    }

    return 1;
}


// ==========================================
// PROOF OF WORK
// ==========================================

void proofOfWork(Block *block) {

    block->nonce = 0;

    do {

        calculerHashBloc(block);

        if (hashValide(block->hash)) {
            break;
        }

        block->nonce++;

    } while (1);
}


// ==========================================
// INITIALISER LA BLOCKCHAIN
// ==========================================

void initialiserBlockchain(Blockchain *chain) {

    chain->count = 0;

    chain->consensus = 0;
}


// ==========================================
// AJOUTER UN BLOC AVEC PoW
// ==========================================

void ajouterBlocPoW(
    Blockchain *chain,
    const char *data
) {

    if (chain->count >= MAX_BLOCKS) {

        printf("Blockchain pleine !\n");

        return;
    }


    Block *block =
        &chain->blocks[chain->count];


    block->index =
        chain->count;


    strcpy(
        block->data,
        data
    );


    // En PoW, pas de validateur

    strcpy(
        block->validateur,
        "Aucun"
    );


    // ======================================
    // HASH PRECEDENT
    // ======================================

    if (chain->count == 0) {

        strcpy(
            block->previousHash,
            "0"
        );

    } else {

        strcpy(
            block->previousHash,
            chain->blocks[
                chain->count - 1
            ].hash
        );
    }


    // ======================================
    // MINAGE
    // ======================================

    proofOfWork(block);


    chain->count++;


    printf(
        "\n===== BLOC %d (PoW) =====\n",
        block->index
    );

    printf(
        "Data          : %s\n",
        block->data
    );

    printf(
        "Previous Hash : %s\n",
        block->previousHash
    );

    printf(
        "Nonce         : %lu\n",
        block->nonce
    );

    printf(
        "Hash          : %s\n",
        block->hash
    );
}


// ==========================================
// CHOISIR UN VALIDATEUR PoS
// ==========================================

int choisirValidateur(
    Validateur validateurs[],
    int n
) {

    int totalStake = 0;


    // Calcul du stake total

    for (int i = 0; i < n; i++) {

        if (validateurs[i].stake < 0) {
            return -1;
        }

        totalStake +=
            validateurs[i].stake;
    }


    if (totalStake <= 0) {
        return -1;
    }


    // Tirage aléatoire

    int tirage =
        rand() % totalStake + 1;


    int cumul = 0;


    for (int i = 0; i < n; i++) {

        cumul +=
            validateurs[i].stake;

        if (tirage <= cumul) {
            return i;
        }
    }


    return -1;
}


// ==========================================
// AJOUTER UN BLOC AVEC PoS
// ==========================================

void ajouterBlocPoS(
    Blockchain *chain,
    const char *data,
    Validateur validateurs[],
    int n
) {

    if (chain->count >= MAX_BLOCKS) {

        printf("Blockchain pleine !\n");

        return;
    }


    Block *block =
        &chain->blocks[chain->count];


    block->index =
        chain->count;


    strcpy(
        block->data,
        data
    );


    // ======================================
    // HASH PRECEDENT
    // ======================================

    if (chain->count == 0) {

        strcpy(
            block->previousHash,
            "0"
        );

    } else {

        strcpy(
            block->previousHash,
            chain->blocks[
                chain->count - 1
            ].hash
        );
    }


    // ======================================
    // CHOISIR LE VALIDATEUR
    // ======================================

    int choix =
        choisirValidateur(
            validateurs,
            n
        );


    if (choix == -1) {

        printf(
            "Erreur : aucun validateur disponible !\n"
        );

        return;
    }


    strcpy(
        block->validateur,
        validateurs[choix].nom
    );


    // ======================================
    // PAS DE MINAGE EN PoS
    // ======================================

    block->nonce = 0;


    // ======================================
    // CALCUL DU HASH
    // ======================================

    calculerHashBloc(block);


    chain->count++;


    printf(
        "\n===== BLOC %d (PoS) =====\n",
        block->index
    );

    printf(
        "Data          : %s\n",
        block->data
    );

    printf(
        "Validateur    : %s\n",
        block->validateur
    );

    printf(
        "Stake         : %d coins\n",
        validateurs[choix].stake
    );

    printf(
        "Previous Hash : %s\n",
        block->previousHash
    );

    printf(
        "Nonce         : %lu\n",
        block->nonce
    );

    printf(
        "Hash          : %s\n",
        block->hash
    );
}


// ==========================================
// MINER UN BLOC
// ==========================================
//
// Cette fonction est différente de
// ajouterBlocPoW() car elle permet
// de préciser le nom du mineur.
//
// Elle sera utilisée pour les attaques.
//
// ==========================================

void minerUnBloc(
    Blockchain *chain,
    const char *data,
    const char *mineur
) {

    if (chain->count >= MAX_BLOCKS) {

        printf(
            "Blockchain pleine !\n"
        );

        return;
    }


    Block *block =
        &chain->blocks[chain->count];


    block->index =
        chain->count;


    strcpy(
        block->data,
        data
    );


    // Ici on utilise le nom du mineur

    strcpy(
        block->validateur,
        mineur
    );


    // ======================================
    // HASH PRECEDENT
    // ======================================

    if (chain->count == 0) {

        strcpy(
            block->previousHash,
            "0"
        );

    } else {

        strcpy(
            block->previousHash,
            chain->blocks[
                chain->count - 1
            ].hash
        );
    }


    // ======================================
    // PROOF OF WORK
    // ======================================

    proofOfWork(block);


    chain->count++;
}


// ==========================================
// COPIER UN BLOC
// ==========================================

void copierBloc(
    Block *destination,
    const Block *source
) {

    *destination = *source;
}


// ==========================================
// VERIFIER LA BLOCKCHAIN
// ==========================================

int verifierBlockchain(Blockchain *chain) {

    for (int i = 0; i < chain->count; i++) {

        Block *block = &chain->blocks[i];

        char hashOriginal[HASH_SIZE];
        char hashRecalcule[HASH_SIZE];

        strcpy(hashOriginal, block->hash);

        calculerHashBloc(block);

        strcpy(hashRecalcule, block->hash);

        // Restaurer le hash original
        strcpy(block->hash, hashOriginal);

        if (strcmp(
                hashOriginal,
                hashRecalcule
            ) != 0) {

            return 0;
        }

        if (i > 0) {

            if (strcmp(
                    block->previousHash,
                    chain->blocks[i - 1].hash
                ) != 0) {

                return 0;
            }
        }

        if (chain->consensus == 1) {

            if (!hashValide(block->hash)) {
                return 0;
            }
        }
    }

    return 1;
}

// ==========================================
// AFFICHER LA BLOCKCHAIN
// ==========================================

void afficherBlockchain(
    const Blockchain *chain
) {

    printf(
        "\n\n========== BLOCKCHAIN ==========\n"
    );


    for (int i = 0;
         i < chain->count;
         i++) {

        const Block *block =
            &chain->blocks[i];


        printf(
            "\n-------- BLOC %d --------\n",
            block->index
        );

        printf(
            "Data          : %s\n",
            block->data
        );

        printf(
            "Previous Hash : %s\n",
            block->previousHash
        );

        printf(
            "Validateur    : %s\n",
            block->validateur
        );

        printf(
            "Nonce         : %lu\n",
            block->nonce
        );

        printf(
            "Hash          : %s\n",
            block->hash
        );
    }
}