#include "blockchain.h"
#include "detection.h"

#include <stdio.h>
#include <stdlib.h>


// ======================================================
// CHOISIR QUI MINE LE PROCHAIN BLOC
// ======================================================
//
// 40% -> Mineurs honnetes
// 60% -> Attaquant
//
// Retour :
// 0 = honnete
// 1 = attaquant
// ======================================================

int choisirMineur51(int honestHashrate, int attackerHashrate) {

    int total = honestHashrate + attackerHashrate;

    if (total <= 0 || honestHashrate < 0 || attackerHashrate < 0) {
        return 0;
    }

    int tirage = rand() % total;

    if (tirage < honestHashrate) {
        return 0;
    }

    return 1;
}


// ======================================================
// ENREGISTRER UN BLOC HONNETE
// ======================================================

static void enregistrerBlocHonnete(DetectionMetrics *metrics) {

    if (metrics == NULL) {
        return;
    }

    // Le bloc est mine et diffuse.
    // Son acceptation sera comptabilisee a la fin
    // selon la branche qui devient principale.

    detectionRecordMined(metrics, 0, 1);

    // Delai de propagation simule : 50 a 500 ms.
    detectionRecordPropagation(
        metrics,
        50 + rand() % 451
    );
}


// ======================================================
// ENREGISTRER UN BLOC SECRET DE L'ATTAQUANT
// ======================================================

static void enregistrerBlocSecret(DetectionMetrics *metrics) {

    if (metrics == NULL) {
        return;
    }

    // Le bloc est mine mais reste secret.
    detectionRecordMined(metrics, 1, 0);
}


// ======================================================
// SIMULATION ATTAQUE 51%
// ======================================================

void simulerAttaque51(DetectionMetrics *metrics) {

    Blockchain honestChain;
    Blockchain attackerChain;

    // Puissance de calcul.
    int honestHashrate = 40;
    int attackerHashrate = 60;

    int tours = 0;
    int attaqueReussie = 0;

    // Initialisation des deux blockchains.
    initialiserBlockchain(&honestChain);
    initialiserBlockchain(&attackerChain);

    honestChain.consensus = 1;
    attackerChain.consensus = 1;


    printf("\n\n");
    printf("============================================\n");
    printf("          ATTAQUE 51%% - SIMULATION\n");
    printf("============================================\n");

    printf("\nPuissance mineurs honnetes : %d%%\n",
           honestHashrate);

    printf("Puissance attaquant        : %d%%\n",
           attackerHashrate);


    // ==================================================
    // ETAPE 1 : CREATION DU GENESIS
    // ==================================================

    printf("\n[1] Creation du bloc Genesis...\n");

    minerUnBloc(
        &honestChain,
        "Genesis",
        "Network"
    );

    // Verifier que le Genesis a bien ete cree.
    if (honestChain.count != 1) {
        printf("\nErreur : impossible de creer le Genesis.\n");
        return;
    }

    // Les deux branches partagent le meme Genesis.
    copierBloc(
        &attackerChain.blocks[0],
        &honestChain.blocks[0]
    );

    attackerChain.count = 1;

    printf("Genesis partage par les deux chaines.\n");


    // ==================================================
    // ETAPE 2 : TRANSACTION INITIALE
    // ==================================================

    printf("\n[2] Transaction initiale...\n");

    printf("\nAttaquant -> Bob : 50 coins\n");

    // La transaction est ajoutee a la branche honnete.
    minerUnBloc(
        &honestChain,
        "Attaquant -> Bob : 50 coins",
        "Honest Miner"
    );

    if (honestChain.count != 2) {
        printf("\nErreur : impossible de miner le bloc honnete.\n");
        return;
    }

    enregistrerBlocHonnete(metrics);

    printf("\nLa transaction est maintenant dans la chaine honnete.\n");
    printf("Bob considere la transaction valide.\n");


    // ==================================================
    // ETAPE 3 : CREATION DE LA BRANCHE SECRETE
    // ==================================================

    printf("\n[3] Creation de la branche secrete...\n");

    printf("\nIMPORTANT :\n");
    printf("L'attaquant ne met PAS la transaction\n");
    printf("'Attaquant -> Bob : 50 coins'\n");
    printf("dans sa branche.\n");

    // L'attaquant cree une transaction alternative.
    minerUnBloc(
        &attackerChain,
        "Attaquant -> Boutique : 50 coins",
        "Attacker"
    );

    if (attackerChain.count != 2) {
        printf("\nErreur : impossible de creer la branche secrete.\n");
        return;
    }

    enregistrerBlocSecret(metrics);

    // Une branche concurrente est maintenant presente.
    if (metrics != NULL) {
        detectionRecordFork(metrics);
    }

    printf("\nDeux branches concurrentes sont presentes.\n");


    // ==================================================
    // ETAPE 4 : COMPETITION DE MINAGE
    // ==================================================

    printf("\n[4] Competition de minage...\n");

    printf("\n40%% -> Honest miners\n");
    printf("60%% -> Attacker\n");

    /*
     * Chaque tour represente un nouveau bloc mine
     * par un mineur honnete ou par l'attaquant.
     *
     * La simulation s'arrete lorsque :
     * - la branche attaquante devient plus longue ;
     * - 20 tours sont atteints ;
     * - une blockchain atteint sa capacite maximale.
     */

    while (
        attackerChain.count <= honestChain.count
        && tours < 20
        && honestChain.count < MAX_BLOCKS
        && attackerChain.count < MAX_BLOCKS
    ) {

        int mineur = choisirMineur51(
            honestHashrate,
            attackerHashrate
        );

        if (mineur == 0) {

            printf(
                "\nTour %d : Honest Miner mine un bloc.",
                tours + 1
            );

            int ancienCount = honestChain.count;

            minerUnBloc(
                &honestChain,
                "Transaction honnete",
                "Honest Miner"
            );

            if (honestChain.count > ancienCount) {
                enregistrerBlocHonnete(metrics);
            }

        } else {

            printf(
                "\nTour %d : Attaquant mine un bloc.",
                tours + 1
            );

            int ancienCount = attackerChain.count;

            minerUnBloc(
                &attackerChain,
                "Transaction alternative",
                "Attacker"
            );

            if (attackerChain.count > ancienCount) {
                enregistrerBlocSecret(metrics);
            }
        }

        printf(
            "\n  Chaine honnete    : %d blocs",
            honestChain.count
        );

        printf(
            "\n  Chaine attaquante : %d blocs\n",
            attackerChain.count
        );

        tours++;
    }


    // ==================================================
    // ETAPE 5 : COMPARAISON DES DEUX CHAINES
    // ==================================================

    printf("\n\n");
    printf("============================================\n");
    printf("       COMPARAISON DES DEUX CHAINES\n");
    printf("============================================\n");

    printf(
        "\nChaine honnete    : %d blocs",
        honestChain.count
    );

    printf(
        "\nChaine attaquante : %d blocs\n",
        attackerChain.count
    );


    // ==================================================
    // ETAPE 6 : DECISION DE LA BRANCHE PRINCIPALE
    // ==================================================

    if (attackerChain.count > honestChain.count) {

        attaqueReussie = 1;

        printf("\nL'attaquant possede la chaine la plus longue !\n");

        printf("\nCHAIN REORGANIZATION\n");
        printf("--------------------\n");

        printf("Ancienne chaine principale :\n");
        printf("Genesis -> Paiement Bob -> ...\n");

        printf("\nNouvelle chaine principale :\n");
        printf("Genesis -> Branche attaquante -> ...\n");


        // --------------------------------------------------
        // 1. BLOCS HONNETES DEVENUS ORPHELINS
        // --------------------------------------------------

        if (metrics != NULL) {

            for (int i = 1; i < honestChain.count; i++) {
                detectionRecordOrphan(metrics);
            }
        }


        // --------------------------------------------------
        // 2. PUBLICATION ET ACCEPTATION DES BLOCS ATTAQUANTS
        // --------------------------------------------------

        if (metrics != NULL) {

            for (int i = 1; i < attackerChain.count; i++) {

                // Le bloc secret est maintenant publie.
                detectionRecordPublished(metrics, 1);

                // Le bloc est accepte dans la chaine principale.
                detectionRecordAccepted(metrics, 1);

                detectionRecordPropagation(
                    metrics,
                    50 + rand() % 451
                );
            }

            // Enregistrer les evenements de securite.
            detectionRecordReorganization(metrics);

            // La transaction de Bob disparait de la chaine.
            detectionRecordDoubleSpend(metrics);

            // Incoherence entre les deux branches.
            detectionRecordInconsistency(metrics);
        }


        // --------------------------------------------------
        // 3. REMPLACEMENT DE LA CHAINE PRINCIPALE
        // --------------------------------------------------

        honestChain = attackerChain;

        printf("\nLa chaine attaquante devient");
        printf(" la chaine principale.\n");


        // ==================================================
        // ETAPE 7 : DOUBLE SPENDING
        // ==================================================

        printf("\n============================================\n");
        printf("             DOUBLE SPENDING\n");
        printf("============================================\n");

        printf("\nTransaction originale :\n");
        printf("Attaquant -> Bob : 50 coins\n");

        printf("\nEtat avant reorganisation :\n");
        printf("Transaction presente dans la chaine honnete.\n");

        printf("\nEtat apres reorganisation :\n");
        printf("Transaction absente de la nouvelle chaine principale.\n");

        printf("\nBob pensait avoir recu les 50 coins,\n");
        printf("mais la transaction a disparu de la chaine principale.\n");

        printf("\nL'attaquant peut alors depenser les 50 coins ailleurs.\n");

    } else {

        printf("\n[6] L'attaque a echoue.\n");
        printf("La chaine honnete reste la chaine principale.\n");

        // --------------------------------------------------
        // 1. BLOCS SECRETS ABANDONNES
        // --------------------------------------------------

        if (metrics != NULL) {

            for (int i = 1; i < attackerChain.count; i++) {

                detectionRecordOrphan(metrics);

                // Ces blocs ne sont pas publies.
                // Ils ne doivent donc plus etre comptes
                // comme blocs actuellement retenus en secret.
                if (metrics->attackerWithheldBlocks > 0) {
                    metrics->attackerWithheldBlocks--;
                }
            }

            // Les blocs honnetes deviennent la branche acceptee.
            for (int i = 1; i < honestChain.count; i++) {
                detectionRecordAccepted(metrics, 0);
            }
        }

        printf("\nLa branche honnete conserve la transaction de Bob.\n");
    }


    // ==================================================
    // ETAPE 8 : RECOMPENSES
    // ==================================================

    if (attaqueReussie) {

        printf("\nRecompenses : branche attaquante acceptee.\n");

    } else {

        printf("\nRecompenses : branche honnete acceptee.\n");
    }


    // ==================================================
    // ETAPE 9 : VERIFICATION DE LA BLOCKCHAIN
    // ==================================================

    printf("\n\n");
    printf("============================================\n");
    printf("       VERIFICATION DE LA BLOCKCHAIN\n");
    printf("============================================\n");

    if (verifierBlockchain(&honestChain)) {
        printf("\nBlockchain finale valide !\n");
    } else {
        printf("\nBlockchain finale invalide !\n");
    }


    // ==================================================
    // ETAPE 10 : AFFICHAGE DE LA CHAINE FINALE
    // ==================================================

    printf("\n\n");
    printf("============================================\n");
    printf("          CHAINE PRINCIPALE FINALE\n");
    printf("============================================\n");

    afficherBlockchain(&honestChain);


    // ==================================================
    // ETAPE 11 : DETECTION AUTOMATIQUE
    // ==================================================

    if (metrics != NULL) {

        printf("\n\n");
        printf("============================================\n");
        printf("       MODULE DE DETECTION AUTOMATIQUE\n");
        printf("============================================\n");

        detectionDisplayMetrics(metrics);

        DetectionResult result = detectionAnalyze(metrics);

        detectionDisplayResult(&result);
    }


    printf("\n============================================\n");
    printf("          FIN DE L'ATTAQUE 51%%\n");
    printf("============================================\n");
}
