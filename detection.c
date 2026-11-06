
#include "detection.h"

#include <stdio.h>
#include <string.h>


// ======================================================
// 1. INITIALISATION
// ======================================================

void detectionInit(DetectionMetrics *m) {

    memset(m, 0, sizeof(DetectionMetrics));
}


// ======================================================
// 2. COLLECTE DES EVENEMENTS
// ======================================================

void detectionRecordMined(
    DetectionMetrics *m,
    int isAttacker,
    int published
) {

    m->totalBlocks++;

    if (isAttacker) {

        m->attackerBlocks++;

        if (!published) {
            m->attackerWithheldBlocks++;
        }

    } else {

        m->honestBlocks++;
    }
}


void detectionRecordPublished(
    DetectionMetrics *m,
    int isAttacker
) {

    if (isAttacker) {

        m->attackerPublishedBlocks++;

        if (m->attackerWithheldBlocks > 0) {
            m->attackerWithheldBlocks--;
        }

        m->strategicPublications++;
    }
}


void detectionRecordAccepted(
    DetectionMetrics *m,
    int isAttacker
) {

    if (isAttacker) {
        m->attackerRewards++;
    } else {
        m->honestRewards++;
    }
}


void detectionRecordOrphan(DetectionMetrics *m) {
    m->orphanBlocks++;
}


void detectionRecordFork(DetectionMetrics *m) {
    m->forks++;
}


void detectionRecordReorganization(DetectionMetrics *m) {
    m->reorganizations++;
}


void detectionRecordDoubleSpend(DetectionMetrics *m) {
    m->doubleSpendingEvents++;
}


void detectionRecordInconsistency(DetectionMetrics *m) {
    m->chainInconsistencies++;
}


void detectionRecordPropagation(
    DetectionMetrics *m,
    double delayMs
) {

    if (delayMs < 0) {
        return;
    }

    m->totalPropagationDelay += delayMs;
    m->propagationSamples++;
}


void detectionRecordSuspiciousNode(DetectionMetrics *m) {
    m->suspiciousNodeEvents++;
}


// ======================================================
// 3. CALCUL DES METRIQUES
// ======================================================

double detectionAttackerShare(
    const DetectionMetrics *m
) {

    if (m->totalBlocks == 0) {
        return 0.0;
    }

    return
        (double)m->attackerBlocks
        / m->totalBlocks * 100.0;
}


double detectionOrphanRate(
    const DetectionMetrics *m
) {

    if (m->totalBlocks == 0) {
        return 0.0;
    }

    return
        (double)m->orphanBlocks
        / m->totalBlocks * 100.0;
}


double detectionRewardShare(
    const DetectionMetrics *m
) {

    int totalRewards =
        m->honestRewards + m->attackerRewards;

    if (totalRewards == 0) {
        return 0.0;
    }

    return
        (double)m->attackerRewards
        / totalRewards * 100.0;
}


double detectionAveragePropagation(
    const DetectionMetrics *m
) {

    if (m->propagationSamples == 0) {
        return 0.0;
    }

    return
        m->totalPropagationDelay
        / m->propagationSamples;
}


// ======================================================
// 4. MOTEUR DE DETECTION RULE-BASED
// ======================================================


DetectionResult detectionAnalyze(const DetectionMetrics *m) {

    DetectionResult r;

    memset(&r, 0, sizeof(r));

    strcpy(r.attackType, "Aucune attaque");
    strcpy(r.severity, "Faible");

    // ==========================================
    // 1. CALCUL DES INDICATEURS
    // ==========================================

    r.attackerShare = detectionAttackerShare(m);
    r.orphanRate = detectionOrphanRate(m);
    r.attackerRewardShare = detectionRewardShare(m);
    r.averagePropagationDelay = detectionAveragePropagation(m);


    // ==========================================
    // 2. CALCUL DU SCORE DE SUSPICION
    // ==========================================

    // Règle 1 : proportion élevée de blocs attaquants
    if (r.attackerShare > ATTACKER_SHARE_THRESHOLD) {
        r.score += 2;
    }

    // Règle 2 : taux élevé de blocs orphelins
    if (r.orphanRate >= ORPHAN_RATE_THRESHOLD) {
        r.score += 2;
    }

    // Règle 3 : rétention ou publications stratégiques
    if (m->attackerWithheldBlocks > 0) {
        r.score += 2;
    }

    if (m->strategicPublications >= 2) {
        r.score += 2;
    }

    // Règle 4 : forks
    if (m->forks >= 2) {
        r.score += 1;
    }

    // Règle 5 : réorganisations
    if (m->reorganizations > 0) {
        r.score += 2;
    }

    // Règle 6 : incohérences de la blockchain
    if (m->chainInconsistencies > 0) {
        r.score += 2;
    }

    // Règle 7 : double spending
    if (m->doubleSpendingEvents > 0) {
        r.score += 3;
    }

    // Règle 8 : comportement suspect des nœuds
    if (m->suspiciousNodeEvents >= 2) {
        r.score += 1;
    }

    // Règle 9 : avantage économique de l'attaquant
    if (
        r.attackerRewardShare >
        r.attackerShare + REWARD_ADVANTAGE_THRESHOLD
    ) {
        r.score += 2;
    }

    // Règle 10 : délai de propagation anormal
    if (
        m->propagationSamples > 0 &&
        r.averagePropagationDelay > PROPAGATION_DELAY_THRESHOLD
    ) {
        r.score += 1;
    }


    // ==========================================
    // 3. CLASSIFICATION DE L'ATTAQUE
    // ==========================================

    // PRIORITÉ 1 : ATTAQUE 51 %

    if (
        m->reorganizations > 0 &&
        (
            m->doubleSpendingEvents > 0 ||
            (
                r.attackerShare > ATTACKER_SHARE_THRESHOLD &&
                m->chainInconsistencies > 0
            )
        )
    ) {

        r.attackDetected = 1;
        strcpy(r.attackType, "51% Attack");
    }


    // PRIORITÉ 2 : SELFISH MINING

    else if (
        (
            m->attackerWithheldBlocks > 0 ||
            m->strategicPublications >= 2
        ) &&
        m->forks > 0 &&
        r.orphanRate >= ORPHAN_RATE_THRESHOLD
    ) {

        r.attackDetected = 1;
        strcpy(r.attackType, "Selfish Mining");
    }


    // PRIORITÉ 3 : COMPORTEMENT SUSPECT

    else if (r.score >= 5) {

        r.attackDetected = 1;
        strcpy(r.attackType, "Comportement suspect");
    }


    // ==========================================
    // 4. NIVEAU DE GRAVITÉ
    // ==========================================

    if (r.score >= 9) {

        strcpy(r.severity, "Critique");

    } else if (r.score >= 6) {

        strcpy(r.severity, "Eleve");

    } else if (r.score >= 3) {

        strcpy(r.severity, "Modere");

    } else {

        strcpy(r.severity, "Faible");
    }

    return r;
}

// ======================================================
// 5. AFFICHAGE DES METRIQUES
// ======================================================

void detectionDisplayMetrics(
    const DetectionMetrics *m
) {

    printf("\n");
    printf("============================================\n");
    printf("        NETWORK SECURITY METRICS\n");
    printf("============================================\n");

    printf("\nTotal blocks mined       : %d", m->totalBlocks);
    printf("\nHonest blocks            : %d", m->honestBlocks);
    printf("\nAttacker blocks          : %d", m->attackerBlocks);

    printf("\nWithheld blocks          : %d", m->attackerWithheldBlocks);
    printf("\nPublished attacker blocks: %d", m->attackerPublishedBlocks);

    printf("\nOrphan blocks            : %d", m->orphanBlocks);
    printf("\nForks                    : %d", m->forks);
    printf("\nReorganizations          : %d", m->reorganizations);

    printf("\nChain inconsistencies    : %d", m->chainInconsistencies);
    printf("\nDouble spending events   : %d", m->doubleSpendingEvents);

    printf("\nSuspicious node events   : %d", m->suspiciousNodeEvents);

    printf("\nHonest rewards           : %d", m->honestRewards);
    printf("\nAttacker rewards         : %d", m->attackerRewards);

    printf("\n\nAttacker block share     : %.2f%%",
           detectionAttackerShare(m));

    printf("\nOrphan block rate        : %.2f%%",
           detectionOrphanRate(m));

    printf("\nAttacker reward share    : %.2f%%",
           detectionRewardShare(m));

    printf("\nAverage propagation delay: %.2f ms\n",
           detectionAveragePropagation(m));
}


// ======================================================
// 6. AFFICHAGE DU RESULTAT
// ======================================================

void detectionDisplayResult(
    const DetectionResult *r
) {

    printf("\n");
    printf("============================================\n");
    printf("           DETECTION REPORT\n");
    printf("============================================\n");

    printf("\nAttack detected : %s",
           r->attackDetected ? "YES" : "NO");

    printf("\nAttack type     : %s",
           r->attackType);

    printf("\nSuspicion score : %d",
           r->score);

    printf("\nSeverity        : %s",
           r->severity);

    printf("\n\n============================================\n");
}
