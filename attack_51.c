#include "blockchain.h"
#include "detection.h"

// ======================================================
// CHOISIR QUI MINE LE PROCHAIN BLOC
// ======================================================
//
// 40% -> mineur honnête
// 60% -> attaquant
//
// Retour :
// 0 = honnête
// 1 = attaquant
// ======================================================

int choisirMineur51(
    int honestHashrate,
    int attackerHashrate
) {

    int total =
        honestHashrate + attackerHashrate;

    int tirage = rand() % total;

    if (tirage < honestHashrate) {
        return 0;
    }

    return 1;
}


// ======================================================
// SIMULATION ATTAQUE 51%
// ======================================================

void simulerAttaque51(void) {

    Blockchain honestChain;
    Blockchain attackerChain;

    /*
     * Puissance de calcul :
     *
     * Honest miners = 40%
     * Attacker      = 60%
     */

    int honestHashrate = 40;
    int attackerHashrate = 60;


    // --------------------------------------------------
    // INITIALISATION
    // --------------------------------------------------

    initialiserBlockchain(&honestChain);
    initialiserBlockchain(&attackerChain);

    honestChain.consensus = 1;
    attackerChain.consensus = 1;


    /*
     * Seed fixe pour rendre l'expérience
     * reproductible.
     */
    srand(42);


    printf("\n\n");
    printf("============================================\n");
    printf("          ATTAQUE 51%% - SIMULATION\n");
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
        &honestChain,
        "Genesis",
        "Network"
    );

    /*
     * L'attaquant commence exactement
     * avec le même Genesis.
     */

    copierBloc(
        &attackerChain.blocks[0],
        &honestChain.blocks[0]
    );

    attackerChain.count = 1;


    printf("Genesis partage par les deux chaines.\n");


    // ==================================================
    // ETAPE 2 : TRANSACTION DOUBLE SPENDING
    // ==================================================

    printf("\n[2] Transaction initiale...\n");

    printf("\nAttaquant -> Bob : 50 coins\n");

    /*
     * La transaction est incluse dans
     * la chaîne honnête.
     */

    minerUnBloc(
        &honestChain,
        "Attaquant -> Bob : 50 coins",
        "Honest Miner"
    );


    printf("\nLa transaction est maintenant dans");
    printf(" la chaine honnete.\n");

    printf("Bob considere la transaction valide.\n");


    // ==================================================
    // ETAPE 3 : L'ATTAQUANT CREE SA BRANCHE SECRETE
    // ==================================================

    printf("\n[3] Creation de la branche secrete...\n");

    printf("\nIMPORTANT :\n");
    printf("L'attaquant ne met PAS la transaction\n");
    printf("'Attaquant -> Bob : 50 coins'\n");
    printf("dans sa branche.\n");


    // Première branche secrète

    minerUnBloc(
        &attackerChain,
        "Attaquant -> Boutique : 50 coins",
        "Attacker"
    );


    // ==================================================
    // ETAPE 4 : COMPETITION DE MINAGE
    // ==================================================

    printf("\n[4] Competition de minage...\n");

    printf("\n40%% -> Honest miners\n");
    printf("60%% -> Attacker\n");


    /*
     * On fait avancer les deux chaînes.
     *
     * L'attaquant a statistiquement plus de chances
     * de miner les prochains blocs.
     */

    int tours = 0;

    while (
        attackerChain.count <= honestChain.count
        && tours < 20
    ) {

        int mineur = choisirMineur51(
            honestHashrate,
            attackerHashrate
        );

        if (mineur == 0) {

            printf("\nTour %d : Honest Miner mine un bloc.",
                   tours + 1);

            minerUnBloc(
                &honestChain,
                "Transaction honnete",
                "Honest Miner"
            );

        } else {

            printf("\nTour %d : Attaquant mine un bloc.",
                   tours + 1);

            minerUnBloc(
                &attackerChain,
                "Transaction alternative",
                "Attacker"
            );
        }

        printf("\n  Chaine honnete   : %d blocs",
               honestChain.count);

        printf("\n  Chaine attaquante : %d blocs\n",
               attackerChain.count);

        tours++;
    }


    // ==================================================
    // ETAPE 5 : L'ATTAQUANT PREND DE L'AVANCE
    // ==================================================

    /*
     * Dans une vraie blockchain, la compétition est
     * probabiliste.
     *
     * Pour cette démonstration, si l'attaquant n'a
     * pas encore dépassé la chaîne honnête après
     * les tours précédents, on continue la simulation
     * jusqu'à ce qu'il possède une chaîne plus longue.
     */

    while (
        attackerChain.count <= honestChain.count
        && attackerChain.count < MAX_BLOCKS
    ) {

        printf("\nAttaquant mine un bloc supplementaire...\n");

        minerUnBloc(
            &attackerChain,
            "Transaction alternative",
            "Attacker"
        );
    }


    // ==================================================
    // ETAPE 6 : COMPARAISON
    // ==================================================

    printf("\n\n");
    printf("============================================\n");
    printf("       COMPARAISON DES DEUX CHAINES\n");
    printf("============================================\n");

    printf("\nChaine honnete   : %d blocs",
           honestChain.count);

    printf("\nChaine attaquante : %d blocs\n",
           attackerChain.count);


    // ==================================================
    // ETAPE 7 : CHAIN REORGANIZATION
    // ==================================================

    if (
        attackerChain.count >
        honestChain.count
    ) {

        printf("\n[5] L'attaquant possede la chaine");
        printf(" la plus longue !\n");

        printf("\nCHAIN REORGANIZATION\n");

        printf("--------------------\n");

        printf("Ancienne chaine principale :\n");

        printf("Genesis -> Paiement Bob -> ...\n");

        printf("\nNouvelle chaine principale :\n");

        printf("Genesis -> Branche attaquante -> ...\n");


        /*
         * La blockchain principale devient
         * la branche de l'attaquant.
         */

        honestChain = attackerChain;


        printf("\nLa chaine attaquante devient");
        printf(" la chaine principale.\n");


        // ==================================================
        // ETAPE 8 : DOUBLE SPENDING
        // ==================================================

        printf("\n============================================\n");
        printf("             DOUBLE SPENDING\n");
        printf("============================================\n");

        printf("\nTransaction originale :\n");

        printf("Attaquant -> Bob : 50 coins\n");

        printf("\nEtat avant reorganisation :\n");

        printf("Transaction presente dans");
        printf(" la chaine honnete.\n");

        printf("\nEtat apres reorganisation :\n");

        printf("Transaction absente de");
        printf(" la nouvelle chaine principale.\n");

        printf("\nBob pensait avoir recu les 50 coins,\n");

        printf("mais la transaction a disparu");
        printf(" de la chaine principale.\n");

        printf("\nL'attaquant peut alors depenser");
        printf(" les 50 coins ailleurs.\n");

    } else {

        printf("\n[5] L'attaque a echoue.\n");

        printf("La chaine honnete reste la chaine principale.\n");
    }


    // ==================================================
    // ETAPE 9 : VERIFICATION
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
    // ETAPE 10 : AFFICHAGE FINAL
    // ==================================================

    printf("\n\n");
    printf("============================================\n");
    printf("          CHAINE PRINCIPALE FINALE\n");
    printf("============================================\n");

    afficherBlockchain(&honestChain);


    printf("\n============================================\n");
    printf("          FIN DE L'ATTAQUE 51%%\n");
    printf("============================================\n");
}