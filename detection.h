
#ifndef DETECTION_H
#define DETECTION_H

#define ATTACKER_SHARE_THRESHOLD 50.0
#define ORPHAN_RATE_THRESHOLD 10.0
#define REWARD_ADVANTAGE_THRESHOLD 10.0
#define PROPAGATION_DELAY_THRESHOLD 300.0

typedef struct {

    // Blocs minés
    int totalBlocks;
    int honestBlocks;
    int attackerBlocks;

    // Publication et rétention
    int attackerWithheldBlocks;
    int attackerPublishedBlocks;
    int strategicPublications;

    // État du réseau
    int orphanBlocks;
    int forks;
    int reorganizations;
    int chainInconsistencies;
    int doubleSpendingEvents;

    // Comportement des nœuds
    int suspiciousNodeEvents;

    // Récompenses
    int honestRewards;
    int attackerRewards;

    // Délais de propagation simulés (ms)
    double totalPropagationDelay;
    int propagationSamples;

} DetectionMetrics;


typedef struct {

    int attackDetected;
    int score;

    double attackerShare;
    double orphanRate;
    double attackerRewardShare;
    double averagePropagationDelay;

    char attackType[50];
    char severity[20];

} DetectionResult;


// Initialisation
void detectionInit(DetectionMetrics *m);

// Collecte automatique des événements
void detectionRecordMined(
    DetectionMetrics *m,
    int isAttacker,
    int published
);

void detectionRecordPublished(
    DetectionMetrics *m,
    int isAttacker
);

void detectionRecordAccepted(
    DetectionMetrics *m,
    int isAttacker
);

void detectionRecordOrphan(
    DetectionMetrics *m
);

void detectionRecordFork(
    DetectionMetrics *m
);

void detectionRecordReorganization(
    DetectionMetrics *m
);

void detectionRecordDoubleSpend(
    DetectionMetrics *m
);

void detectionRecordInconsistency(
    DetectionMetrics *m
);

void detectionRecordPropagation(
    DetectionMetrics *m,
    double delayMs
);

void detectionRecordSuspiciousNode(
    DetectionMetrics *m
);


// Calcul des indicateurs
double detectionAttackerShare(
    const DetectionMetrics *m
);

double detectionOrphanRate(
    const DetectionMetrics *m
);

double detectionRewardShare(
    const DetectionMetrics *m
);

double detectionAveragePropagation(
    const DetectionMetrics *m
);


// Moteur de détection
DetectionResult detectionAnalyze(
    const DetectionMetrics *m
);


// Affichage
void detectionDisplayMetrics(
    const DetectionMetrics *m
);

void detectionDisplayResult(
    const DetectionResult *r
);

void simulerAttaque51(DetectionMetrics *metrics);
void simulerSelfishMining(DetectionMetrics *metrics);

#endif
