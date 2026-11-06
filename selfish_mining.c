#include "blockchain.h"


// ======================================================
// SELFISH MINING
// ======================================================
//
// Principe :
//
// L'attaquant ne publie pas immédiatement ses blocs.
// Il construit une chaîne privée.
//
// Quand il possède une avance suffisante, il publie
// stratégiquement ses blocs afin de remplacer une partie
// de la chaîne honnête.
//
// Etat :
//
// 0 -> les deux chaînes sont à égalité
// 1 -> attaquant possède 1 bloc secret
// 2 -> attaquant possède au moins 2 blocs secrets
//
// ======================================================


// ------------------------------------------------------
// CHOISIR QUI MINE LE PROCHAIN BLOC
// ------------------------------------------------------

int choisirMineurSelfish(
    int honestHashrate,
    int attackerHashrate
) {

    int total =
        honestHashrate + attackerHashrate;

    int tirage =
        rand() % total;

    if (tirage < honestHashrate) {
        return 0;   // Honest
    }

    return 1;       // Attacker
}


// ------------------------------------------------------
// AFFICHER L'ETAT
// ------------------------------------------------------

void afficherEtatSelfish(
    int honestLength,
    int privateLength
) {

    printf("\n--------------------------------------------\n");

    printf("Chaine publique  : %d blocs\n",
           honestLength);

    printf("Chaine privee    : %d blocs\n",
           privateLength);

    printf("Avance attaquant  : %d blocs\n",
           privateLength - honestLength);

    printf("--------------------------------------------\n");
}


// ======================================================
// SIMULATION SELFISH MINING
// ======================================================

void simulerSelfishMining(void) {

    Blockchain publicChain;
    Blockchain privateChain;

    // --------------------------------------------------
    // PUISSANCE DE CALCUL
    // --------------------------------------------------

    int honestHashrate = 60;
    int attackerHashrate = 40;

    int tours = 0;

    int blocsAttaquant = 0;
    int blocsHonnêtes = 0;

    int blocsPublies = 0;


    // --------------------------------------------------
    // INITIALISATION
    // --------------------------------------------------

    initialiserBlockchain(&publicChain);
    initialiserBlockchain(&privateChain);

    publicChain.consensus = 1;
    privateChain.consensus = 1;

    srand(42);


    printf("\n\n");
    printf("============================================\n");
    printf("          SELFISH MINING - SIMULATION\n");
    printf("============================================\n");

    printf("\nPuissance mineurs honnêtes : %d%%\n",
           honestHashrate);

    printf("Puissance attaquant        : %d%%\n",
           attackerHashrate);


    // ==================================================
    // ETAPE 1 : GENESIS
    // ==================================================

    printf("\n[1] Creation du bloc Genesis...\n");

    minerUnBloc(
        &publicChain,
        "Genesis",
        "Network"
    );

    copierBloc(
        &privateChain.blocks[0],
        &publicChain.blocks[0]
    );

    privateChain.count = 1;


    printf("\nGenesis partage par tous les mineurs.\n");


    // ==================================================
    // ETAPE 2 : MINAGE
    // ==================================================

    printf("\n[2] Debut du Selfish Mining...\n");


    while (
        tours < 15 &&
        publicChain.count < MAX_BLOCKS - 2
    ) {

        tours++;

        int mineur =
            choisirMineurSelfish(
                honestHashrate,
                attackerHashrate
            );


        // =================================================
        // L'ATTAQUANT TROUVE UN BLOC
        // =================================================

        if (mineur == 1) {

            printf("\nTour %d : ATTACKER mine un bloc secret.",
                   tours);

            minerUnBloc(
                &privateChain,
                "Transaction privee",
                "Attacker"
            );

            blocsAttaquant++;


            printf("\nBloc conserve SECRET.");

            printf("\nChaine publique : %d blocs",
                   publicChain.count);

            printf("\nChaine privee   : %d blocs\n",
                   privateChain.count);
        }


        // =================================================
        // LES HONNETES TROUVENT UN BLOC
        // =================================================

        else {

            printf("\nTour %d : HONEST MINER mine un bloc.",
                   tours);


            // ---------------------------------------------
            // CAS 1 :
            // Attaquant n'a aucune avance
            // ---------------------------------------------

            if (
                privateChain.count
                <= publicChain.count
            ) {

                minerUnBloc(
                    &publicChain,
                    "Transaction honnete",
                    "Honest Miner"
                );

                // La chaîne privée doit rester synchronisée
                copierBloc(
                    &privateChain.blocks[
                        privateChain.count
                    ],
                    &publicChain.blocks[
                        publicChain.count - 1
                    ]
                );

                privateChain.count =
                    publicChain.count;


                blocsHonnêtes++;
            }


            // ---------------------------------------------
            // CAS 2 :
            // Attaquant possède une avance
            // ---------------------------------------------

            else {

                int avance =
                    privateChain.count
                    - publicChain.count;


                printf("\nAttaquant possede une avance de %d bloc(s).",
                       avance);


                // =========================================
                // AVANCE >= 2
                // =========================================

                if (avance >= 2) {

                    printf("\nAttaquant publie un bloc.");

                    copierBloc(
                        &publicChain.blocks[
                            publicChain.count
                        ],
                        &privateChain.blocks[
                            publicChain.count
                        ]
                    );

                    publicChain.count++;

                    blocsPublies++;

                    printf("\nUn bloc secret devient public.");
                }


                // =========================================
                // AVANCE = 1
                // =========================================

                else {

                    printf("\nPublication strategique.");

                    /*
                     * Les deux chaînes sont en compétition.
                     *
                     * Pour simplifier la simulation :
                     * l'attaquant publie son bloc secret
                     * afin de provoquer une compétition.
                     */

                    copierBloc(
                        &publicChain.blocks[
                            publicChain.count
                        ],
                        &privateChain.blocks[
                            publicChain.count
                        ]
                    );

                    publicChain.count++;

                    blocsPublies++;

                    printf("\nBloc secret publie.");
                }
            }
        }


        afficherEtatSelfish(
            publicChain.count,
            privateChain.count
        );


        // --------------------------------------------------
        // SECURITE : EVITER LE DEPASSEMENT
        // --------------------------------------------------

        if (
            publicChain.count >= MAX_BLOCKS
            ||
            privateChain.count >= MAX_BLOCKS
        ) {
            break;
        }
    }


    // ==================================================
    // ETAPE 3 : PUBLICATION FINALE
    // ==================================================

    printf("\n\n============================================\n");
    printf("       PUBLICATION FINALE\n");
    printf("============================================\n");


    while (
        privateChain.count > publicChain.count
        &&
        publicChain.count < MAX_BLOCKS
    ) {

        printf("\nAttaquant publie un bloc secret...");

        copierBloc(
            &publicChain.blocks[
                publicChain.count
            ],
            &privateChain.blocks[
                publicChain.count
            ]
        );

        publicChain.count++;

        blocsPublies++;
    }


    // ==================================================
    // ETAPE 4 : RESULTATS
    // ==================================================

    printf("\n\n============================================\n");
    printf("          RESULTATS SELFISH MINING\n");
    printf("============================================\n");


    printf("\nBlocs mines par attaquant : %d",
           blocsAttaquant);

    printf("\nBlocs mines par honnetes   : %d",
           blocsHonnêtes);

    printf("\nBlocs secrets publies      : %d",
           blocsPublies);


    printf("\n\nTaille chaine publique finale : %d",
           publicChain.count);

    printf("\nTaille chaine privee finale   : %d\n",
           privateChain.count);


    // ==================================================
    // VERIFICATION
    // ==================================================

    printf("\n\n============================================\n");
    printf("          VERIFICATION\n");
    printf("============================================\n");


    if (
        verifierBlockchain(&publicChain)
    ) {

        printf("\nBlockchain finale valide !\n");

    } else {

        printf("\nBlockchain finale invalide !\n");
    }


    // ==================================================
    // AFFICHAGE
    // ==================================================

    afficherBlockchain(
        &publicChain
    );


    printf("\n============================================\n");
    printf("        FIN DU SELFISH MINING\n");
    printf("============================================\n");
}