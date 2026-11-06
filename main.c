#include "blockchain.h"
#include "detection.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main(void) {

    // ==========================================
    // INITIALISATION
    // ==========================================

    srand((unsigned int)time(NULL));

    Blockchain chain;

    initialiserBlockchain(&chain);


    // ==========================================
    // VALIDATEURS PoS
    // ==========================================

    Validateur validateurs[] = {
        {"Alice", 10},
        {"Bob", 30},
        {"Charlie", 60}
    };

    int nombreValidateurs = 3;

    int choix;


    // ==========================================
    // MENU PRINCIPAL
    // ==========================================

    printf("\n=================================\n");
    printf("      BLOCKCHAIN FROM SCRATCH\n");
    printf("=================================\n");

    printf("\n1. Blockchain PoW\n");
    printf("2. Blockchain PoS\n");
    printf("3. Simuler une attaque 51%%\n");
    printf("4. Simuler Selfish Mining\n");
    printf("5. Tester le module de detection\n");

    printf("\nVotre choix : ");

    if (scanf("%d", &choix) != 1) {
        printf("\nEntree invalide !\n");
        return 1;
    }


    // ==========================================
    // OPTION 1 : PROOF OF WORK
    // ==========================================

    if (choix == 1) {

        chain.consensus = 1;

        printf("\n===== PROOF OF WORK =====\n");

        ajouterBlocPoW(
            &chain,
            "Alice -> Bob : 10 coins"
        );

        ajouterBlocPoW(
            &chain,
            "Bob -> Charlie : 5 coins"
        );

        ajouterBlocPoW(
            &chain,
            "Charlie -> Alice : 2 coins"
        );
    }


    // ==========================================
    // OPTION 2 : PROOF OF STAKE
    // ==========================================

    else if (choix == 2) {

        chain.consensus = 2;

        printf("\n===== PROOF OF STAKE =====\n");

        ajouterBlocPoS(
            &chain,
            "Alice -> Bob : 10 coins",
            validateurs,
            nombreValidateurs
        );

        ajouterBlocPoS(
            &chain,
            "Bob -> Charlie : 5 coins",
            validateurs,
            nombreValidateurs
        );

        ajouterBlocPoS(
            &chain,
            "Charlie -> Alice : 2 coins",
            validateurs,
            nombreValidateurs
        );
    }


    // ==========================================
    // OPTION 3 : ATTAQUE 51%
    // ==========================================

    else if (choix == 3) {

        DetectionMetrics metrics;

        detectionInit(&metrics);

        simulerAttaque51(&metrics);

        return 0;
    }


    // ==========================================
    // OPTION 4 : SELFISH MINING
    // ==========================================

    else if (choix == 4) {

        DetectionMetrics metrics;

        detectionInit(&metrics);

        simulerSelfishMining(&metrics);

        return 0;
    }


    // ==========================================
    // OPTION 5 : TEST DU MODULE DE DETECTION
    // ==========================================

    else if (choix == 5) {

        DetectionMetrics metrics;

        detectionInit(&metrics);

        printf("\n=================================\n");
        printf("       TEST DU DETECTEUR\n");
        printf("=================================\n");

        /*
         * Scenario de test synthetique :
         *
         * 4 blocs honnetes.
         * 6 blocs mines par l'attaquant.
         * Publication des blocs de l'attaquant.
         * Presence de forks, d'une reorganisation
         * et d'une double depense.
         *
         * Les evenements sont enregistres avec
         * les fonctions du module de detection.
         */


        // --------------------------------------
        // 1. Blocs honnetes
        // --------------------------------------

        for (int i = 0; i < 4; i++) {

            detectionRecordMined(
                &metrics,
                0,
                1
            );

            detectionRecordAccepted(
                &metrics,
                0
            );
        }


        // --------------------------------------
        // 2. Blocs mines par l'attaquant
        // --------------------------------------

        for (int i = 0; i < 6; i++) {

            detectionRecordMined(
                &metrics,
                1,
                0
            );
        }


        // --------------------------------------
        // 3. Publication des blocs attaquants
        // --------------------------------------

        for (int i = 0; i < 6; i++) {

            detectionRecordPublished(
                &metrics,
                1
            );

            detectionRecordAccepted(
                &metrics,
                1
            );
        }


        // --------------------------------------
        // 4. Evenements suspects
        // --------------------------------------

        detectionRecordFork(&metrics);
        detectionRecordFork(&metrics);
        detectionRecordFork(&metrics);

        detectionRecordOrphan(&metrics);
        detectionRecordOrphan(&metrics);

        detectionRecordReorganization(&metrics);

        detectionRecordDoubleSpend(&metrics);

        detectionRecordInconsistency(&metrics);


        // --------------------------------------
        // 5. Analyse automatique
        // --------------------------------------

        printf("\nAnalyse des metriques...\n");

        DetectionResult result =
            detectionAnalyze(&metrics);


        // --------------------------------------
        // 6. Affichage des resultats
        // --------------------------------------

        detectionDisplayMetrics(&metrics);

        detectionDisplayResult(&result);

        return 0;
    }


    // ==========================================
    // CHOIX INVALIDE
    // ==========================================

    else {

        printf("\nChoix invalide !\n");

        return 1;
    }


    // ==========================================
    // AFFICHER LA BLOCKCHAIN PoW / PoS
    // ==========================================

    printf("\n\n===== BLOCKCHAIN FINALE =====\n");

    afficherBlockchain(&chain);


    // ==========================================
    // VERIFICATION DE LA BLOCKCHAIN
    // ==========================================

    printf("\n===== VERIFICATION =====\n");

    if (verifierBlockchain(&chain)) {

        printf("\nBlockchain valide !\n");

    } else {

        printf("\nBlockchain invalide !\n");
    }


    return 0;
}
