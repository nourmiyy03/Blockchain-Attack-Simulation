#include "blockchain.h"
#include "blockchain.h"
#include "detection.h"

int main(void) {

    // Initialiser le générateur aléatoire
    srand((unsigned int)time(NULL));

    Blockchain chain;

    initialiserBlockchain(
        &chain
    );


    // VALIDATEURS PoS
    Validateur validateurs[] = {

        {"Alice", 10},
        {"Bob", 30},
        {"Charlie", 60}
    };

    int nombreValidateurs = 3;


    int choix;


    // MENU

    printf("=================================\n");
    printf("      BLOCKCHAIN FROM SCRATCH\n");
    printf("=================================\n");

    printf("\n1. Blockchain PoW\n");
    printf("2. Blockchain PoS\n");
    printf("3. Simuler une attaque 51%%\n");
    printf("4. Simuler Selfish Mining\n");
    printf("\n5. Tester le module de detection\n");
    printf("Votre choix : ");
    scanf("%d", &choix);



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


    // PROOF OF STAKE
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

    else if(choix ==3 ){
    simulerAttaque51();
    }

    else if (choix == 4) {
    simulerSelfishMining();
    }
    else if (choix == 5) {
    DetectionMetrics metrics;

    initialiserMetriques(&metrics);

    /*
     * Exemple de scenario suspect.
     *
     * Ces valeurs seront ensuite produites
     * automatiquement par nos simulations.
     */

    metrics.totalBlocks = 10;
    metrics.honestBlocks = 4;
    metrics.attackerBlocks = 6;
    metrics.orphanBlocks = 2;
    metrics.forks = 3;
    metrics.chainInconsistencies = 1;


    afficherMetriques(&metrics);

    DetectionResult result = detecterAttaque(&metrics);

    afficherDetection(&result);
    }

    else {

        printf(
            "\nChoix invalide !\n"
        );

        return 1;
    }


    // AFFICHER LA BLOCKCHAIN
    afficherBlockchain(&chain);


    // VERIFIER LA BLOCKCHAIN
    verifierBlockchain( &chain);


    return 0;
}