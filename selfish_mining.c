
#include "blockchain.h"
#include "detection.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ======================================================
// SELFISH MINING
// ======================================================
//
// Parametres :
// - Honest Hashrate : 60%
// - Attacker Hashrate : 40%
// - Gamma : 50%
//
// Gamma represente la proportion des mineurs honnetes
// qui choisissent la branche de l'attaquant lorsqu'un
// fork est publie.
//
// Etat :
// lead = privateChain.count - publicChain.count
//
// lead = 0 : aucune avance
// lead = 1 : un bloc secret d'avance
// lead >= 2 : avance importante
//
// ======================================================


// ------------------------------------------------------
// COPIER UNE BLOCKCHAIN
// ------------------------------------------------------

static void copierBlockchain(
    Blockchain *destination,
    const Blockchain *source
) {
    destination->count = source->count;
    destination->consensus = source->consensus;

    for (int i = 0; i < source->count; i++) {
        copierBloc(
            &destination->blocks[i],
            &source->blocks[i]
        );
    }
}


// ------------------------------------------------------
// CALCULER L'AVANCE DE L'ATTAQUANT
// ------------------------------------------------------

static int calculerAvance(
    const Blockchain *privateChain,
    const Blockchain *publicChain
) {
    return privateChain->count - publicChain->count;
}


// ------------------------------------------------------
// CHOISIR LE MINEUR QUI TROUVE LE PROCHAIN BLOC
// ------------------------------------------------------
//
// Retour :
// 0 = Honest Miner
// 1 = Attacker
//
// ------------------------------------------------------

int choisirMineurSelfish(
    int honestHashrate,
    int attackerHashrate
) {
    int total = honestHashrate + attackerHashrate;

    int tirage = rand() % total;

    if (tirage < honestHashrate) {
        return 0;
    }

    return 1;
}


// ------------------------------------------------------
// AFFICHER L'ETAT DES CHAINES
// ------------------------------------------------------

static void afficherEtatSelfish(
    const Blockchain *publicChain,
    const Blockchain *privateChain
) {
    int avance = calculerAvance(
        privateChain,
        publicChain
    );

    printf("\n--------------------------------------------\n");

    printf("Chaine publique : %d blocs\n",
           publicChain->count);

    printf("Chaine privee   : %d blocs\n",
           privateChain->count);

    printf("Avance attaquant : %d bloc(s)\n",
           avance);

    printf("--------------------------------------------\n");
}


// ------------------------------------------------------
// ENREGISTRER UN BLOC HONNETE QUI EST DEVENU ORPHELIN
// ------------------------------------------------------

static void enregistrerBlocOrphelin(
    DetectionMetrics *metrics
) {
    if (metrics != NULL) {
        detectionRecordOrphan(metrics);
    }
}


// ------------------------------------------------------
// PUBLIER UN BLOC SECRET DE L'ATTAQUANT
// ------------------------------------------------------
//
// Le bloc est deja mine dans privateChain.
// On le copie dans la chaine publique.
//
// ------------------------------------------------------

static int publierBlocAttaquant(
    Blockchain *publicChain,
    const Blockchain *privateChain,
    DetectionMetrics *metrics
) {
    if (publicChain->count >= MAX_BLOCKS) {
        return 0;
    }

    if (publicChain->count >= privateChain->count) {
        return 0;
    }

    int index = publicChain->count;

    copierBloc(
        &publicChain->blocks[index],
        &privateChain->blocks[index]
    );

    publicChain->count++;

    if (metrics != NULL) {
        detectionRecordPublished(metrics, 1);
        detectionRecordAccepted(metrics, 1);
    }

    printf("\n[PUBLICATION] L'attaquant publie son bloc secret.");

    return 1;
}


// ======================================================
// SIMULATION SELFISH MINING
// ======================================================

void simulerSelfishMining(DetectionMetrics *metrics) {

    Blockchain publicChain;
    Blockchain privateChain;
    Blockchain honestFork;

    // --------------------------------------------------
    // PARAMETRES DU RESEAU
    // --------------------------------------------------

    int honestHashrate = 60;
    int attackerHashrate = 40;

    // Pourcentage des mineurs honnetes qui suivent
    // la branche de l'attaquant pendant un fork.

    int gamma = 50;

    int nombreTours = 20;

    int tours = 0;

    int blocsAttaquant = 0;
    int blocsHonnetes = 0;
    int blocsPublies = 0;

    int forksCrees = 0;
    int reorganisations = 0;

    // --------------------------------------------------
    // INITIALISATION
    // --------------------------------------------------

    initialiserBlockchain(&publicChain);
    initialiserBlockchain(&privateChain);
    initialiserBlockchain(&honestFork);

    publicChain.consensus = 1;
    privateChain.consensus = 1;
    honestFork.consensus = 1;

    srand(42);

    printf("\n\n");
    printf("============================================\n");
    printf("          SELFISH MINING - SIMULATION\n");
    printf("============================================\n");

    printf("\nPuissance mineurs honnetes : %d%%\n",
           honestHashrate);

    printf("Puissance attaquant        : %d%%\n",
           attackerHashrate);

    printf("Gamma                      : %d%%\n",
           gamma);

    printf("Nombre de tours            : %d\n",
           nombreTours);


    // ==================================================
    // ETAPE 1 : GENESIS
    // ==================================================

    printf("\n[1] Creation du bloc Genesis...\n");

    minerUnBloc(
        &publicChain,
        "Genesis",
        "Network"
    );

    copierBlockchain(
        &privateChain,
        &publicChain
    );

    printf("\nGenesis partage par tous les mineurs.\n");


    // ==================================================
    // ETAPE 2 : MINAGE ET PUBLICATION STRATEGIQUE
    // ==================================================

    printf("\n[2] Debut du Selfish Mining...\n");


    while (
        tours < nombreTours &&
        publicChain.count < MAX_BLOCKS - 2 &&
        privateChain.count < MAX_BLOCKS - 2
    ) {

        tours++;

        printf("\n\n============================================\n");
        printf("TOUR %d\n", tours);
        printf("============================================\n");

        int mineur = choisirMineurSelfish(
            honestHashrate,
            attackerHashrate
        );

        int avance = calculerAvance(
            &privateChain,
            &publicChain
        );


        // =================================================
        // CAS 1 : L'ATTAQUANT TROUVE UN BLOC
        // =================================================

        if (mineur == 1) {

            printf("\n[ATTACKER] L'attaquant trouve un bloc.");

            if (privateChain.count >= MAX_BLOCKS) {
                break;
            }

            minerUnBloc(
                &privateChain,
                "Transaction privee",
                "Attacker"
            );

            blocsAttaquant++;

            if (metrics != NULL) {
                detectionRecordMined(metrics, 1, 0);
                detectionRecordPropagation(
                    metrics,
                    50 + rand() % 451
                );
            }

            printf("\n[ATTACKER] Bloc conserve SECRET.");

            printf("\nL'attaquant ne publie pas immediatement.");

        }


        // =================================================
        // CAS 2 : UN MINEUR HONNETE TROUVE UN BLOC
        // =================================================

        else {

            printf("\n[HONEST] Un mineur honnete trouve un bloc.");

            blocsHonnetes++;

            // ---------------------------------------------
            // CAS 2.1 : AUCUNE AVANCE DE L'ATTAQUANT
            // ---------------------------------------------

            if (avance == 0) {

                printf("\nAucune avance privee.");

                minerUnBloc(
                    &publicChain,
                    "Transaction honnete",
                    "Honest Miner"
                );

                if (metrics != NULL) {
                    detectionRecordMined(metrics, 0, 1);
                    detectionRecordAccepted(metrics, 0);

                    detectionRecordPropagation(
                        metrics,
                        50 + rand() % 451
                    );
                }

                // La chaine privee se synchronise avec
                // la chaine publique.

                copierBlockchain(
                    &privateChain,
                    &publicChain
                );

                printf("\nLe bloc honnete devient public.");

            }


            // ---------------------------------------------
            // CAS 2.2 : L'ATTAQUANT A UNE AVANCE D'UN BLOC
            // ---------------------------------------------

            else if (avance == 1) {

                printf("\n[WARNING] L'attaquant possede 1 bloc secret.");

                printf("\nUn mineur honnete trouve un bloc.");
                printf("\nUne competition entre deux branches commence.");

                forksCrees++;

                if (metrics != NULL) {
                    detectionRecordMined(metrics, 0, 1);
                    detectionRecordFork(metrics);

                    detectionRecordPropagation(
                        metrics,
                        50 + rand() % 451
                    );
                }

                // -----------------------------------------
                // CONSTRUIRE LA BRANCHE HONNETE
                // -----------------------------------------

                copierBlockchain(
                    &honestFork,
                    &publicChain
                );

                minerUnBloc(
                    &honestFork,
                    "Transaction honnete",
                    "Honest Miner"
                );

                // -----------------------------------------
                // PUBLIER LE BLOC SECRET
                // -----------------------------------------

                if (!publierBlocAttaquant(
                    &publicChain,
                    &privateChain,
                    metrics
                )) {
                    printf("\nErreur : impossible de publier le bloc.");
                    break;
                }

                blocsPublies++;

                printf("\n[FORK] Les deux branches sont en competition.");

                // -----------------------------------------
                // RESOLUTION DU FORK
                // -----------------------------------------
                //
                // Probabilite de victoire de la branche
                // attaquante :
                //
                // alpha + gamma * (1 - alpha)
                //
                // Avec alpha = 40% et gamma = 50% :
                // probabilite = 70%.
                //
                // -----------------------------------------

                int tirage = rand() % 100;

                int probabiliteAttaquant =
                    attackerHashrate +
                    (gamma * honestHashrate) / 100;

                if (tirage < probabiliteAttaquant) {

                    // =====================================
                    // BRANCHE ATTAQUANTE GAGNANTE
                    // =====================================

                    printf("\n[RESULTAT] La branche de l'attaquant gagne.");

                    printf("\nLe bloc honnete concurrent devient orphelin.");

                    enregistrerBlocOrphelin(metrics);

                    reorganisations++;

                    if (metrics != NULL) {
                        detectionRecordReorganization(metrics);
                    }

                    // Un nouveau bloc est mine sur la branche
                    // attaquante pour confirmer sa victoire.

                    if (tirage < attackerHashrate) {

                        minerUnBloc(
                            &publicChain,
                            "Confirmation attaquante",
                            "Attacker"
                        );

                        blocsAttaquant++;

                        if (metrics != NULL) {
                            detectionRecordMined(metrics, 1, 1);
                            detectionRecordAccepted(metrics, 1);

                            detectionRecordPropagation(
                                metrics,
                                50 + rand() % 451
                            );
                        }

                        printf("\nL'attaquant mine le bloc suivant.");

                    } else {

                        minerUnBloc(
                            &publicChain,
                            "Confirmation honnete sur branche attaquante",
                            "Honest Miner"
                        );

                        blocsHonnetes++;

                        if (metrics != NULL) {
                            detectionRecordMined(metrics, 0, 1);
                            detectionRecordAccepted(metrics, 0);

                            detectionRecordPropagation(
                                metrics,
                                50 + rand() % 451
                            );
                        }

                        printf("\nUn mineur honnete suit la branche attaquante.");
                    }

                    // La branche privee devient la branche publique.

                    copierBlockchain(
                        &privateChain,
                        &publicChain
                    );

                } else {

                    // =====================================
                    // BRANCHE HONNETE GAGNANTE
                    // =====================================

                    printf("\n[RESULTAT] La branche honnete gagne.");

                    printf("\nLe bloc secret de l'attaquant devient orphelin.");

                    enregistrerBlocOrphelin(metrics);

                    // La branche honnete devient canonique.

                    copierBlockchain(
                        &publicChain,
                        &honestFork
                    );

                    if (metrics != NULL) {
                        detectionRecordAccepted(metrics, 0);
                    }

                    // Un second bloc est mine sur la branche
                    // honnete pour confirmer sa victoire.

                    minerUnBloc(
                        &publicChain,
                        "Confirmation honnete",
                        "Honest Miner"
                    );

                    blocsHonnetes++;

                    if (metrics != NULL) {
                        detectionRecordMined(metrics, 0, 1);
                        detectionRecordAccepted(metrics, 0);

                        detectionRecordPropagation(
                            metrics,
                            50 + rand() % 451
                        );
                    }

                    // L'attaquant perd son ancienne branche privee.

                    copierBlockchain(
                        &privateChain,
                        &publicChain
                    );
                }
            }


            // ---------------------------------------------
            // CAS 2.3 : L'ATTAQUANT A AU MOINS DEUX BLOCS
            // ---------------------------------------------

            else if (avance >= 2) {

                printf("\n[ADVANTAGE] L'attaquant possede %d blocs d'avance.",
                       avance);

                printf("\nL'attaquant publie strategiquement un bloc.");

                // Le mineur honnete a trouve un bloc sur
                // l'ancienne branche publique.
                //
                // Mais l'attaquant publie son bloc secret
                // avant que ce bloc honnete soit accepte.

                if (!publierBlocAttaquant(
                    &publicChain,
                    &privateChain,
                    metrics
                )) {
                    printf("\nErreur : publication impossible.");
                    break;
                }

                blocsPublies++;

                // Le bloc honnete concurrent est abandonne.

                enregistrerBlocOrphelin(metrics);

                printf("\nLe bloc honnete concurrent devient orphelin.");

                printf("\nL'attaquant conserve une partie de son avance.");

            }
        }


        afficherEtatSelfish(
            &publicChain,
            &privateChain
        );


        // --------------------------------------------------
        // VERIFICATION DES LIMITES
        // --------------------------------------------------

        if (
            publicChain.count >= MAX_BLOCKS ||
            privateChain.count >= MAX_BLOCKS
        ) {
            printf("\nLimite de blocs atteinte.");
            break;
        }
    }


    // ==================================================
    // ETAPE 3 : PUBLICATION FINALE
    // ==================================================

    printf("\n\n============================================\n");
    printf("          PUBLICATION FINALE\n");
    printf("============================================\n");

    int avanceFinale = calculerAvance(
        &privateChain,
        &publicChain
    );

    if (avanceFinale > 0) {

        printf("\nL'attaquant possede encore %d bloc(s) secret(s).",
               avanceFinale);

        printf("\nPublication de la branche privee restante...");

        while (
            privateChain.count > publicChain.count &&
            publicChain.count < MAX_BLOCKS
        ) {

            if (!publierBlocAttaquant(
                &publicChain,
                &privateChain,
                metrics
            )) {
                break;
            }

            blocsPublies++;
        }

        copierBlockchain(
            &privateChain,
            &publicChain
        );

        printf("\nLes blocs secrets restants sont publies.");

    } else {

        printf("\nAucun bloc secret restant a publier.");
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
           blocsHonnetes);

    printf("\nBlocs secrets publies     : %d",
           blocsPublies);

    printf("\nForks crees                : %d",
           forksCrees);

    printf("\nReorganisations            : %d",
           reorganisations);

    printf("\n\nTaille chaine publique finale : %d",
           publicChain.count);

    printf("\nTaille chaine privee finale   : %d\n",
           privateChain.count);


    // ==================================================
    // ETAPE 5 : VERIFICATION
    // ==================================================

    printf("\n\n============================================\n");
    printf("             VERIFICATION\n");
    printf("============================================\n");

    if (verifierBlockchain(&publicChain)) {

        printf("\nBlockchain finale valide !\n");

    } else {

        printf("\nBlockchain finale invalide !\n");
    }


    // ==================================================
    // ETAPE 6 : AFFICHAGE
    // ==================================================

    printf("\n\n============================================\n");
    printf("          BLOCKCHAIN FINALE\n");
    printf("============================================\n");

    afficherBlockchain(&publicChain);


    // ==================================================
    // ETAPE 7 : DETECTION AUTOMATIQUE
    // ==================================================

    if (metrics != NULL) {

        printf("\n\n============================================\n");
        printf("          DETECTION AUTOMATIQUE\n");
        printf("============================================\n");

        DetectionResult resultat =
            detectionAnalyze(metrics);

        detectionDisplayMetrics(metrics);

        detectionDisplayResult(&resultat);
    }


    printf("\n============================================\n");
    printf("        FIN DU SELFISH MINING\n");
    printf("============================================\n");
}
