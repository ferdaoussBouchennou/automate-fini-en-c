
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define EPSILON '$'

typedef struct {
    int numEtat;
} Etat;

typedef struct Transition {
    Etat de;
    Etat vers;
    char etiquette;
    struct Transition *suivant;
} Transition;

typedef struct {
    Etat *etats_initiaux; // Pointeur vers un tableau des états initiaux
    int nb_initiaux;      // Nombre d'états initiaux
    Etat *etats_finaux;   // Pointeur vers un tableau des états finaux
    int nb_finaux;        // Nombre d'états finaux
    Transition *transitions; // Liste chaînée des transitions
    int nb_transitions;   // Nombre de transitions
    int nb_etats;         // Nombre total d'états dans l'automate
} Automate;

Transition* creer_transition(Etat de, Etat vers, char etiquette) {
    Transition *nouvelle = (Transition*) malloc(sizeof(Transition));
    if (!nouvelle) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }
    nouvelle->de = de;
    nouvelle->vers = vers;
    nouvelle->etiquette = etiquette;
    nouvelle->suivant = NULL;
    return nouvelle;
}

void ajouter_transition(Automate *automate, Etat de, Etat vers, char etiquette) {
    Transition *nouvelle = creer_transition(de, vers, etiquette);
    if (!automate->transitions) {
        automate->transitions = nouvelle;
    } else {
        Transition *courant = automate->transitions;
        while (courant->suivant) {
            courant = courant->suivant;
        }
        courant->suivant = nouvelle;
    }
    automate->nb_transitions++;

    // Mettre à jour nb_etats si nécessaire
    int etats[100] = {0};
    Transition *c = automate->transitions;
    while (c) {
        etats[c->de.numEtat] = 1;
        etats[c->vers.numEtat] = 1;
        c = c->suivant;
    }

    int nb_etats = 0;
    for (int i = 0; i < 100; i++) {
        if (etats[i]) nb_etats++;
    }
    automate->nb_etats = nb_etats;
}

void liberer_transitions(Transition *transition) {
    Transition *courant = transition;
    while (courant != NULL) {
        Transition *suivant = courant->suivant;// Garder une reference vers le suivant
        free(courant);// Liberer la transition actuelle
        courant = suivant;
    }
}

void lire_fichier(const char *nom_fichier, Automate *automate) {
    // Réinitialiser l'automate
    if (automate->transitions != NULL) {
        liberer_transitions(automate->transitions);
    }
    if (automate->etats_initiaux != NULL) {
        free(automate->etats_initiaux);
    }
    if (automate->etats_finaux != NULL) {
        free(automate->etats_finaux);
    }

    automate->transitions = NULL;
    automate->nb_transitions = 0;
    automate->etats_initiaux = NULL;
    automate->nb_initiaux = 0;
    automate->etats_finaux = NULL;
    automate->nb_finaux = 0;
    automate->nb_etats = 0;

    FILE *fichier = fopen(nom_fichier, "r");
    if (!fichier) {
        perror("Erreur d'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    char ligne[100];
    int etats[100] = {0}; // Tableau pour stocker les états uniques
    int nb_etats = 0;

    while (fgets(ligne, sizeof(ligne), fichier)) {
        int de, vers;
        char etiquette_str[10]; // Pour lire l'étiquette comme une chaîne de caractères
        if (sscanf(ligne, " %d->%d [label=%[^]]", &de, &vers, etiquette_str) == 3) {
            // Supprimer les guillemets de l'étiquette si nécessaire
            char *etiquette_ptr = etiquette_str;
            if (etiquette_str[0] == '"') {
                etiquette_ptr++; // Ignorer le premier guillemet
                etiquette_str[strlen(etiquette_str) - 1] = '\0'; // Supprimer le dernier guillemet
            }

            // Convertir l'étiquette en caractère
            char etiquette;
            if (strcmp(etiquette_ptr, "ε") == 0 || strcmp(etiquette_ptr, "$") == 0) {
                etiquette = EPSILON; // Utiliser EPSILON pour représenter les transitions epsilon
            } else {
                etiquette = etiquette_ptr[0]; // Prendre le premier caractère de l'étiquette
            }

            // Ajouter la transition
            Etat etat_initial = {de};
            Etat etat_final = {vers};
            ajouter_transition(automate, etat_initial, etat_final, etiquette);

            // Mettre à jour les états uniques
            if (!etats[de]) {
                etats[de] = 1;
                nb_etats++;
            }
            if (!etats[vers]) {
                etats[vers] = 1;
                nb_etats++;
            }
        } else if (sscanf(ligne, " initial->%d", &de) == 1) {
            // Ajouter l'état initial
            automate->etats_initiaux = realloc(automate->etats_initiaux, (automate->nb_initiaux + 1) * sizeof(Etat));
            automate->etats_initiaux[automate->nb_initiaux].numEtat = de;
            automate->nb_initiaux++;

            // Mettre à jour les états uniques
            if (!etats[de]) {
                etats[de] = 1;
                nb_etats++;
            }
        } else if (sscanf(ligne, " %d->final", &de) == 1) {
            // Ajouter l'état final
            automate->etats_finaux = realloc(automate->etats_finaux, (automate->nb_finaux + 1) * sizeof(Etat));
            automate->etats_finaux[automate->nb_finaux].numEtat = de;
            automate->nb_finaux++;

            // Mettre à jour les états uniques
            if (!etats[de]) {
                etats[de] = 1;
                nb_etats++;
            }
        }
    }

    automate->nb_etats = nb_etats; // Mettre à jour le nombre d'états
    fclose(fichier);
}

void afficher_automate(const Automate *automate) {
    printf("Etats initiaux: { ");
    for (int i = 0; i < automate->nb_initiaux; i++) {
        printf("%d ", automate->etats_initiaux[i].numEtat);
    }
    printf("}\n");

    printf(" Etats finaux: { ");
    for (int i = 0; i < automate->nb_finaux; i++) {
        printf("%d ", automate->etats_finaux[i].numEtat);
    }
    printf("}\n");

    printf(" Transitions:\n");
    Transition *courant = automate->transitions;

    int etats[20], nb_etats = 0;
    char alphabet[20];
    int nb_alphabets = 0;

    while (courant) {
        printf(" &(%d, %c) = {%d}\n", courant->de.numEtat, courant->etiquette, courant->vers.numEtat);

        int i;
        for (i = 0; i < nb_etats; i++) {
            if (etats[i] == courant->de.numEtat) break;
        }
        if (i == nb_etats) etats[nb_etats++] = courant->de.numEtat;

        for (i = 0; i < nb_etats; i++) {
            if (etats[i] == courant->vers.numEtat) break;
        }
        if (i == nb_etats) etats[nb_etats++] = courant->vers.numEtat;

        for (i = 0; i < nb_alphabets; i++) {
            if (alphabet[i] == courant->etiquette) break;
        }
        if (i == nb_alphabets) alphabet[nb_alphabets++] = courant->etiquette;

        courant = courant->suivant;
    }

    printf(" Etats de l'automate: { ");
    for (int i = 0; i < nb_etats; i++) {
        printf("%d ", etats[i]);
    }
    printf("}\n");

    printf(" Les alphabets: { ");
    for (int i = 0; i < nb_alphabets; i++) {
        printf("%c ", alphabet[i]);
    }
    printf("}\n");
}

void generer_fichier(const char *nom_fichier, Automate *automate) {
    FILE *fichier = fopen(nom_fichier, "w");
    if (!fichier) {
        perror("Erreur d'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier,"digraph {\n");
    for (int i = 0; i < automate->nb_initiaux; i++) {
        fprintf(fichier, "initial [shape = none, label = \"\"]\n");
    }
    for (int i = 0; i < automate->nb_finaux; i++) {
        fprintf(fichier, "final [shape = none, label = \"\"]\n");
    }
    for (int i = 0; i < automate->nb_initiaux; i++) {
        fprintf(fichier,"initial->%d\n",automate->etats_initiaux[i].numEtat);
    }

    Transition *courant = automate->transitions;
    while (courant != NULL) {
        if (courant->etiquette == '$') {  // Si epsilon
            fprintf(fichier, "%d->%d [label=\"ε\"]\n", courant->de.numEtat, courant->vers.numEtat);
        } else {
            fprintf(fichier, "%d->%d [label=%c]\n", courant->de.numEtat, courant->vers.numEtat, courant->etiquette);
        }
        courant = courant->suivant;
    }

    for (int i = 0; i < automate->nb_finaux; i++) {
        fprintf(fichier,"%d->final\n",automate->etats_finaux[i].numEtat);
    }
    fprintf(fichier,"}\n");

    fflush(fichier);
    fclose(fichier);

    printf("Fichier '%s' genere avec succes.\n", nom_fichier);
}

void afficherEtat_maxTransitions(Automate *automate) {
    int maxTransitions = 0;//Stocke le nombre maximal de transitions sortantes trouvees.
    int transitionsPar_Etat[100] = {0};//Tableau qui compte le nombre de transitions sortantes pour chaque etat

    Transition *courant = automate->transitions;
    while (courant) {
        transitionsPar_Etat[courant->de.numEtat]++;
        if (transitionsPar_Etat[courant->de.numEtat] > maxTransitions) {
            maxTransitions = transitionsPar_Etat[courant->de.numEtat];
        }
        courant = courant->suivant;
    }

    int etats_max[100];//stocker les etats ayant le maximum de transitions.
    int count_max = 0;
    for (int i = 0; i < 100; i++) {
        if (transitionsPar_Etat[i] == maxTransitions && maxTransitions > 0) {
            etats_max[count_max++] = i;
        }
    }

    if (count_max == 0) {
        printf("Aucune transition trouvee.\n");
    } else if (count_max == 1) {
        printf("L'etat avec le plus de transitions sortantes est : %d avec %d transitions.\n", etats_max[0], maxTransitions);
    } else {
        printf("Les etats avec le plus de transitions sortantes (%d transition(s)) sont : ", maxTransitions);
        for (int i = 0; i < count_max; i++) {
            printf("%d ", etats_max[i]);
        }
        printf("\n");
    }
}

void afficherEtats_TransitionEtiquette(Automate *automate, char etiquette) {
    printf("Entrez l'etiquette a verifier : ");
    scanf(" %c", &etiquette);

    int etatsAvecEtiquette[100] = {0};//Tableau qui va stocker les numeros des etats ayant une transition sortante avec cette etiquette.
    int count = 0;//Nombre d'etats stockes dans etatsAvecEtiquette.

    Transition *courant = automate->transitions;
    while (courant) {
        if (courant->etiquette == etiquette) {
            int etat_de = courant->de.numEtat;
            int deja_ajoute = 0;
            for (int j = 0; j < count; j++) {
                if (etatsAvecEtiquette[j] == etat_de) {
                    deja_ajoute = 1;
                    break;
                }
            }
            if (!deja_ajoute) {
                etatsAvecEtiquette[count++] = etat_de;
            }
        }
        courant = courant->suivant;
    }

    if (count == 0) {
        printf("Aucun etat avec transition sortante par l'etiquette '%c'.\n", etiquette);
    } else {
        printf("Etats avec transition sortante par l'etiquette '%c' : ", etiquette);
        for (int i = 0; i < count; i++) {
            printf("%d ", etatsAvecEtiquette[i]);
        }
        printf("\n");
    }
}

//Verifie si un etat donne est un etat final dans l'automate
int est_etat_final(Automate *automate, int etat) {
    // Parcours de tous les etats finaux de l'automate
    for (int i = 0; i < automate->nb_finaux; i++) {
        // Si l'etat courant correspond à un etat final, retourne 1
        if (automate->etats_finaux[i].numEtat == etat) {
            return 1;
        }
    }
    // Si l'etat n'est pas trouve dans les etats finaux, retourne 0
    return 0;
}


//Teste si un mot est accepte par l'automate.
int tester_mot(Automate *automate, char *mot) {
    int longueur_mot = strlen(mot);

    // Parcours de tous les etats initiaux de l'automate
    for (int i = 0; i < automate->nb_initiaux; i++) {
        int etat_actuel = automate->etats_initiaux[i].numEtat;
        int j = 0;

        // Correction: condition de la boucle while
        while (j < longueur_mot) {
            int transition_trouvee = 0;
            Transition *courant = automate->transitions;

            // Parcours de toutes les transitions de l'automate
            while (courant != NULL) {
                // Si la transition part de l'etat actuel
                if (courant->de.numEtat == etat_actuel) {
                    // Si le caractere courant du mot correspond a l'etiquette de la transition
                    if (courant->etiquette == mot[j]) {
                        etat_actuel = courant->vers.numEtat;
                        transition_trouvee = 1;
                        j++;
                        break;
                    }
                    // Si la transition est une transition vide (epsilon)
                    else if (courant->etiquette == '$') {
                        etat_actuel = courant->vers.numEtat;
                        transition_trouvee = 1;
                        break;
                    }
                }
                courant = courant->suivant;
            }

            // Si aucune transition n'a ete trouvee, on sort de la boucle
            if (!transition_trouvee){
                break;
            }
        }

        // Si on a parcouru tout le mot et que l'etat actuel est un etat final, le mot est accepte
        if (j == longueur_mot && est_etat_final(automate, etat_actuel)) {
            return 1;
        }
    }
    // Si aucun chemin n'a mene a un etat final, le mot n'est pas accepte
    return 0;
}

//Teste une liste de mots contenus dans un fichier texte.
void tester_liste_mots(const char *nom_fichier, Automate *automate) {
    // Ouverture du fichier en mode lecture
    FILE *fichier = fopen(nom_fichier, "r");
    if (!fichier) {
        printf("Erreur d'ouverture du fichier.\n");
        return;
    }

    char mot[100];
    // Lecture de chaque mot dans le fichier
    while (fscanf(fichier, "%s", mot) != EOF) {
        // Teste si le mot est accepte par l'automate
        if (tester_mot(automate, mot)) {
            printf("Le mot '%s' est accepte.\n", mot);
        } else {
            printf("Le mot '%s' n'est pas accepte.\n", mot);
        }
    }

    fclose(fichier);
}

//Fonction principale pour tester des mots ou des fichiers avec l'automate.
void tester(const char *nom_fichier, Automate *automate) {
    int choix;
    char str[100];

    // Boucle de menu pour choisir entre tester un mot ou un fichier
    do {
        printf("\n1. Tester un mot\n2. Tester un fichier .txt\n0. Retour\nChoix : ");
        scanf("%d", &choix);
        if (choix == 1) {
            // Demande a l'utilisateur d'entrer un mot a tester
            printf("Entrez le mot : ");
            scanf("%s", str);

            // Teste si le mot est accepte par l'automate
            if (tester_mot(automate, str)) {
                printf("Le mot est accepte.\n");
            } else {
                printf("Le mot n'est pas accepte.\n");
            }
        } else if (choix == 2) {
            // Demande a l'utilisateur d'entrer le nom d'un fichier a tester
            printf("Entrez le nom du fichier .txt: ");
            scanf("%s", str);

            // Teste tous les mots du fichier
            tester_liste_mots(str, automate);
        }
    } while (choix != 0);
}

Automate* concatenation(Automate *a, Automate *b) {

    //  automate pour stocker le resultat
    Automate *resultat = (Automate*) malloc(sizeof(Automate));
    if (!resultat) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    // Initialisation des proprietes de l'automate resultat
    resultat->nb_etats = a->nb_etats + b->nb_etats;
    resultat->nb_transitions = a->nb_transitions + b->nb_transitions + (a->nb_finaux * b->nb_initiaux);
    resultat->nb_initiaux = a->nb_initiaux;
    resultat->nb_finaux = b->nb_finaux;
    resultat->transitions = NULL;

    // Copie des transitions de a
    Transition *courant = a->transitions;
    while (courant) {
        ajouter_transition(resultat, courant->de, courant->vers, courant->etiquette);
        courant = courant->suivant;
    }

    // Copie des transitions de b
    courant = b->transitions;
    while (courant) {
        Etat de = courant->de;
        Etat vers = courant->vers;
        de.numEtat += a->nb_etats;
        vers.numEtat += a->nb_etats;
        ajouter_transition(resultat, de, vers, courant->etiquette);
        courant = courant->suivant;
    }

    // Ajouter des transitions epsilon entre tous les etats finaux de A et tous les etats initiaux de B
    for (int i = 0; i < a->nb_finaux; i++) {
        Etat etat_final_A = a->etats_finaux[i];
        for (int j = 0; j < b->nb_initiaux; j++) {
            Etat etat_initial_B = b->etats_initiaux[j];
            etat_initial_B.numEtat += a->nb_etats;
            ajouter_transition(resultat, etat_final_A, etat_initial_B, EPSILON);
        }
    }

    // Mise a jour des etats initiaux et finaux
    resultat->etats_initiaux = (Etat*) malloc(resultat->nb_initiaux * sizeof(Etat));
    for (int i = 0; i < a->nb_initiaux; i++) {
        resultat->etats_initiaux[i] = a->etats_initiaux[i];
    }

    resultat->etats_finaux = (Etat*) malloc(resultat->nb_finaux * sizeof(Etat));
    for (int i = 0; i < b->nb_finaux; i++) {
        resultat->etats_finaux[i] = b->etats_finaux[i];
        resultat->etats_finaux[i].numEtat += a->nb_etats;
    }

    return resultat;
}

Automate* union_automates(Automate *a, Automate *b) {

    Automate *resultat = (Automate*) malloc(sizeof(Automate));
    if (!resultat) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    resultat->nb_etats = a->nb_etats + b->nb_etats + 2; // +2 pour les nouveaux etats initial et final
    resultat->nb_transitions = a->nb_transitions + b->nb_transitions + a->nb_initiaux + b->nb_initiaux + a->nb_finaux + b->nb_finaux;
    resultat->nb_initiaux = 1;
    resultat->nb_finaux = 1;
    resultat->transitions = NULL;

    //  les nouveaux etats initial et final
    Etat nouvel_initial = {0};
    Etat nouvel_final = {a->nb_etats + b->nb_etats + 1};

    resultat->etats_initiaux = (Etat*) malloc(sizeof(Etat));
    resultat->etats_finaux = (Etat*) malloc(sizeof(Etat));
    if (!resultat->etats_initiaux || !resultat->etats_finaux) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    resultat->etats_initiaux[0] = nouvel_initial;
    resultat->etats_finaux[0] = nouvel_final;

    // Copie des transitions de a
    Transition *courant = a->transitions;
    while (courant) {
        Etat de = courant->de;
        Etat vers = courant->vers;
        de.numEtat += 1;
        vers.numEtat += 1;
        ajouter_transition(resultat, de, vers, courant->etiquette);
        courant = courant->suivant;
    }

    // Copie des transitions de B avec décalage des IDs
    courant = b->transitions;
    while (courant) {
        Etat de = courant->de;
        Etat vers = courant->vers;
        de.numEtat += a->nb_etats + 1;
        vers.numEtat += a->nb_etats + 1;
        ajouter_transition(resultat, de, vers, courant->etiquette);
        courant = courant->suivant;
    }

    // Ajouter des transitions epsilon
    for (int i = 0; i < a->nb_initiaux; i++) {
        Etat etat_initial_A = a->etats_initiaux[i];
        etat_initial_A.numEtat += 1;
        ajouter_transition(resultat, nouvel_initial, etat_initial_A, EPSILON);
    }
    for (int i = 0; i < b->nb_initiaux; i++) {
        Etat etat_initial_B = b->etats_initiaux[i];
        etat_initial_B.numEtat += a->nb_etats + 1;
        ajouter_transition(resultat, nouvel_initial, etat_initial_B, EPSILON);
    }

    // Ajouter des transitions epsilon de tous les etats finaux de A et B au nouvel etat final
    for (int i = 0; i < a->nb_finaux; i++) {
        Etat etat_final_A = a->etats_finaux[i];
        etat_final_A.numEtat += 1;
        ajouter_transition(resultat, etat_final_A, nouvel_final, EPSILON);
    }
    for (int i = 0; i < b->nb_finaux; i++) {
        Etat etat_final_B = b->etats_finaux[i];
        etat_final_B.numEtat += a->nb_etats + 1;
        ajouter_transition(resultat, etat_final_B, nouvel_final, EPSILON);
    }
    return resultat;
}

Automate* kleene_etoile(Automate *A) {

    Automate *resultat = (Automate*) malloc(sizeof(Automate));
    if (!resultat) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    resultat->nb_etats = A->nb_etats + 2; // +2 pour les nouveaux etats initial et final
    resultat->nb_initiaux = 1;
    resultat->nb_finaux = 1;
    resultat->nb_transitions = A->nb_transitions + A->nb_initiaux + A->nb_finaux + (A->nb_finaux * A->nb_initiaux) + 1;
    resultat->transitions = NULL;

    // les nouveaux etats initial et final
    Etat nouvel_initial = {0};
    Etat nouvel_final = {A->nb_etats + 1};

    resultat->etats_initiaux = (Etat*) malloc(sizeof(Etat));
    resultat->etats_finaux = (Etat*) malloc(sizeof(Etat));
    if (!resultat->etats_initiaux || !resultat->etats_finaux) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    resultat->etats_initiaux[0] = nouvel_initial;
    resultat->etats_finaux[0] = nouvel_final;

    // Copier les transitions de A
    Transition *courant = A->transitions;
    while (courant) {
        Etat de = courant->de;
        Etat vers = courant->vers;
        de.numEtat += 1;
        vers.numEtat += 1;
        ajouter_transition(resultat, de, vers, courant->etiquette);
        courant = courant->suivant;
    }

    // Ajouter des transitions epsilon du nouvel etat initial a tous les etats initiaux de A
    for (int i = 0; i < A->nb_initiaux; i++) {
        Etat etat_initial_A = A->etats_initiaux[i];
        etat_initial_A.numEtat += 1;
        ajouter_transition(resultat, nouvel_initial, etat_initial_A, EPSILON);
    }

    // Ajouter des transitions epsilon de tous les etats finaux de A au nouvel etat final
    for (int i = 0; i < A->nb_finaux; i++) {
        Etat etat_final_A = A->etats_finaux[i];
        etat_final_A.numEtat += 1;
        ajouter_transition(resultat, etat_final_A, nouvel_final, EPSILON);
    }

    // Ajouter des transitions epsilon de tous les etats finaux de A a tous les etats initiaux de A
    for (int i = 0; i < A->nb_finaux; i++) {
        Etat etat_final_A = A->etats_finaux[i];
        etat_final_A.numEtat += 1;
        for (int j = 0; j < A->nb_initiaux; j++) {
            Etat etat_initial_A = A->etats_initiaux[j];
            etat_initial_A.numEtat += 1;
            ajouter_transition(resultat, etat_final_A, etat_initial_A, EPSILON);
        }
    }
    // Ajouter une transition epsilon du nouvel etat initial au nouvel etat final
    ajouter_transition(resultat, nouvel_initial, nouvel_final, EPSILON);

    return resultat;
}
Automate* creer_automate_base(char c) {
    Automate *automate = (Automate*) malloc(sizeof(Automate));
    if (!automate) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    automate->nb_etats = 2;
    automate->nb_initiaux = 1;
    automate->nb_finaux = 1;
    automate->nb_transitions = 1;

    automate->etats_initiaux = (Etat*) malloc(sizeof(Etat));
    automate->etats_finaux = (Etat*) malloc(sizeof(Etat));
    if (!automate->etats_initiaux || !automate->etats_finaux) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    automate->etats_initiaux[0].numEtat = 0;
    automate->etats_finaux[0].numEtat = 1;

    automate->transitions = creer_transition(automate->etats_initiaux[0], automate->etats_finaux[0], c);

    return automate;
}

void supprimer_transition_epsilon(Automate *automate) {

    Transition *courant = automate->transitions;
    Transition *precedent = NULL;

    while (courant) {
        // Vérifier si la transition est une transition epsilon (etiquette '$') et si elle va de l'état initial à l'état final
        if (courant->etiquette == EPSILON &&
            courant->de.numEtat == automate->etats_initiaux[0].numEtat &&
            courant->vers.numEtat == automate->etats_finaux[0].numEtat) {
            // Supprimer cette transition
            if (precedent) {
                precedent->suivant = courant->suivant;
            } else {
                automate->transitions = courant->suivant;
            }
            free(courant);
            break;
            }
        precedent = courant;
        courant = courant->suivant;
    }
}
Automate* expression_to_automate_recursive(const char *expression, int *index) {
    if (!expression || *index >= strlen(expression)) {
        return NULL;
    }
    Automate *resultat = NULL;

    // Traitement d'un caractere ou sous-expression
    char c = expression[*index];

    if (c == '(') {
        (*index)++;
        resultat = expression_to_automate_recursive(expression, index);
        if (expression[*index] != ')') {
            printf("Parenthese fermante n'existe pas.\n");
            return NULL;
        }
        (*index)++;
    } else if (c != '|' && c != '.' && c != '*' && c != '+' && c != ')') {
        // Caractere simple
        resultat = creer_automate_base(c);
        (*index)++;
    } else {
        printf("Erreur : Caractere inattendu '%c' a la position %d.\n", c, *index);
        return NULL;
    }

    // Traitement des operateurs suivants
    if (*index < strlen(expression)) {
        c = expression[*index];

        // Opérateurs unaires appliqués directement à l'automate précédent
        if (c == '*') {
            (*index)++;
            resultat = kleene_etoile(resultat);

            // Vérifier si d'autres opérateurs suivent
            if (*index < strlen(expression)) {
                c = expression[*index];
            }
        } else if (c == '+') {
            (*index)++;
            // Pour l'opérateur +, on applique l'étoile de Kleene puis on supprime la transition epsilon directe
            Automate *kleene = kleene_etoile(resultat);
            supprimer_transition_epsilon(kleene);
            resultat = kleene;

            // Vérifier si d'autres opérateurs suivent
            if (*index < strlen(expression)) {
                c = expression[*index];
            }
        }

        // Opérateurs binaires nécessitant un deuxième automate
        if (c == '|') {
            (*index)++;
            Automate *droit = expression_to_automate_recursive(expression, index);
            if (!droit) {
                return NULL;
            }
            resultat = union_automates(resultat, droit);
        } else if (c == '.') {
            (*index)++;
            Automate *droit = expression_to_automate_recursive(expression, index);
            if (!droit) {
                return NULL;
            }
            resultat = concatenation(resultat, droit);
        } else if (*index < strlen(expression) && expression[*index] != ')') {
            // Si ce n'est pas un opérateur et que ce n'est pas une parenthèse fermante,
            // on suppose que c'est une concaténation implicite
            Automate *droit = expression_to_automate_recursive(expression, index);
            if (!droit) {
                return NULL;
            }
            resultat = concatenation(resultat, droit);
        }
    }

    return resultat;
}

void epsilonClosure(Automate *automate, int etat, bool *visite, bool *closure) {
    visite[etat] = true;
    closure[etat] = true;

    Transition *courant = automate->transitions;
    while (courant != NULL) {
        if (courant->de.numEtat == etat && courant->etiquette == EPSILON && !visite[courant->vers.numEtat]) {
            epsilonClosure(automate, courant->vers.numEtat, visite, closure);
        }
        courant = courant->suivant;
    }
}

Automate supprimerEpsilonTransitions(Automate *automate) {
    Automate resultat = {NULL, 0, NULL, 0, NULL, 0, 0};

    // Etape 1: Cloture transitive des ε-transitions
    bool *closure = (bool *)calloc(automate->nb_etats, sizeof(bool));
    bool *visite = (bool *)calloc(automate->nb_etats, sizeof(bool));

    // Tableau pour marquer les etats accessibles
    bool *etats_accessibles = (bool *)calloc(automate->nb_etats, sizeof(bool));

    // Etape 2: Marquer les etats initiaux comme accessibles
    for (int i = 0; i < automate->nb_initiaux; i++) {
        etats_accessibles[automate->etats_initiaux[i].numEtat] = true;
    }

    // Etape 3: Cloture transitive des ε-transitions et marquage des etats accessibles
    for (int i = 0; i < automate->nb_etats; i++) {
        memset(visite, 0, automate->nb_etats * sizeof(bool));
        epsilonClosure(automate, i, visite, closure);

        // Marquer les etats accessibles via des transitions non-ε
        Transition *courant = automate->transitions;
        while (courant != NULL) {
            if (closure[courant->de.numEtat] && courant->etiquette != EPSILON) {
                etats_accessibles[courant->vers.numEtat] = true;
            }
            courant = courant->suivant;
        }

        // Etape 4: Ajouter des transitions non-ε
        courant = automate->transitions;
        while (courant != NULL) {
            if (closure[courant->de.numEtat] && courant->etiquette != EPSILON) {
                ajouter_transition(&resultat, (Etat){i}, courant->vers, courant->etiquette);
            }
            courant = courant->suivant;
        }

        // Etape 5: Marquer les etats finaux accessibles via ε-transitions
        for (int j = 0; j < automate->nb_etats; j++) {
            if (closure[j] && est_etat_final(automate, j)) {
                // Si l'état j est un état final accessible via ε-transitions, marquer l'état i comme final
                resultat.etats_finaux = realloc(resultat.etats_finaux, (resultat.nb_finaux + 1) * sizeof(Etat));
                resultat.etats_finaux[resultat.nb_finaux].numEtat = i;
                resultat.nb_finaux++;
                break; // Pas besoin de continuer, l'etat i est deja marque comme final
            }
        }

        memset(closure, 0, automate->nb_etats * sizeof(bool));
    }

    // Etape 6: Ajouter les etats initiaux au resultat
    resultat.etats_initiaux = (Etat *)malloc(automate->nb_initiaux * sizeof(Etat));
    resultat.nb_initiaux = automate->nb_initiaux;
    for (int i = 0; i < automate->nb_initiaux; i++) {
        resultat.etats_initiaux[i] = automate->etats_initiaux[i];
    }

    // Etape 7: Ajouter les etats finaux accessibles via des transitions non-ε
    for (int i = 0; i < automate->nb_finaux; i++) {
        if (etats_accessibles[automate->etats_finaux[i].numEtat]) {
            resultat.etats_finaux = realloc(resultat.etats_finaux, (resultat.nb_finaux + 1) * sizeof(Etat));
            resultat.etats_finaux[resultat.nb_finaux] = automate->etats_finaux[i];
            resultat.nb_finaux++;
        }
    }

    // Etape 8: Supprimer les etats inaccessibles et les etats finaux isoles
    for (int i = 0; i < automate->nb_etats; i++) {
        if (!etats_accessibles[i]) {
            // Supprimer les transitions qui partent ou arrivent a cet etat
            Transition *courant = resultat.transitions;
            Transition *precedent = NULL;
            while (courant != NULL) {
                if (courant->de.numEtat == i || courant->vers.numEtat == i) {
                    if (precedent) {
                        precedent->suivant = courant->suivant;
                    } else {
                        resultat.transitions = courant->suivant;
                    }
                    Transition *a_supprimer = courant;
                    courant = courant->suivant;
                    free(a_supprimer);
                } else {
                    precedent = courant;
                    courant = courant->suivant;
                }
            }
        }
    }
    free(closure);
    free(visite);
    free(etats_accessibles);

    return resultat;
}

void supprimer_etats_finaux_isoles(Automate *automate) {
    if (!automate || !automate->etats_finaux || automate->nb_finaux == 0) {
        return;
    }

    // Tableau pour marquer les etats finaux a supprimer
    bool *a_supprimer = (bool *)calloc(automate->nb_finaux, sizeof(bool));
    if (!a_supprimer) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    // Parcourir tous les etats finaux
    for (int i = 0; i < automate->nb_finaux; i++) {
        int etat_final = automate->etats_finaux[i].numEtat;
        bool a_des_transitions = false;

        // Verifier si l'etat final a des transitions entrantes ou sortantes
        Transition *courant = automate->transitions;
        while (courant) {
            if (courant->de.numEtat == etat_final || courant->vers.numEtat == etat_final) {
                a_des_transitions = true;
                break;
            }
            courant = courant->suivant;
        }

        // Si l'etat final n'a aucune transition, il est isole et doit etre supprime
        if (!a_des_transitions) {
            a_supprimer[i] = true;
        }
    }
    int nouveaux_finaux = 0;
    for (int i = 0; i < automate->nb_finaux; i++) {
        if (!a_supprimer[i]) {
            automate->etats_finaux[nouveaux_finaux++] = automate->etats_finaux[i];
        }
    }
    automate->nb_finaux = nouveaux_finaux;
    automate->etats_finaux = (Etat *)realloc(automate->etats_finaux, nouveaux_finaux * sizeof(Etat));

    free(a_supprimer);
}
Automate determinisation(Automate *automate_src) {
    Automate automate_dest = {NULL, 0, NULL, 0, NULL, 0, 0};
    int est_deterministe = 1;

    if (automate_src->nb_initiaux > 1) {
        est_deterministe = 0;
    } else {
        Transition *t1 = automate_src->transitions;
        while (t1 != NULL && est_deterministe) {
            Transition *t2 = automate_src->transitions;
            while (t2 != NULL) {
                if (t1 != t2 && t1->de.numEtat == t2->de.numEtat &&
                    t1->etiquette == t2->etiquette) {
                    est_deterministe = 0;
                    break;
                }
                t2 = t2->suivant;
            }
            t1 = t1->suivant;
        }
    }

    if (est_deterministe) {
        printf("\nL'automate est deja déterministe.\n");
        automate_dest.nb_initiaux = automate_src->nb_initiaux;
        automate_dest.etats_initiaux = (Etat*)malloc(automate_src->nb_initiaux * sizeof(Etat));
        for (int i = 0; i < automate_src->nb_initiaux; i++) {
            automate_dest.etats_initiaux[i] = automate_src->etats_initiaux[i];
        }

        automate_dest.nb_finaux = automate_src->nb_finaux;
        automate_dest.etats_finaux = (Etat*)malloc(automate_src->nb_finaux * sizeof(Etat));
        for (int i = 0; i < automate_src->nb_finaux; i++) {
            automate_dest.etats_finaux[i] = automate_src->etats_finaux[i];
        }

        Transition *t = automate_src->transitions;
        while (t != NULL) {
            ajouter_transition(&automate_dest, t->de, t->vers, t->etiquette);
            t = t->suivant;
        }

        return automate_dest;
    }

    char alphabet[100];
    int nb_symboles = 0;

    Transition *t = automate_src->transitions;
    while (t != NULL) {
        int trouve = 0;
        for (int i = 0; i < nb_symboles; i++) {
            if (alphabet[i] == t->etiquette) {
                trouve = 1;
                break;
            }
        }
        if (!trouve) {
            alphabet[nb_symboles++] = t->etiquette;
        }
        t = t->suivant;
    }
    int sous_ensembles[100][100];
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            sous_ensembles[i][j] = -1;
        }
    }
    int nb_sous_ensembles = 1;
    int index = 0;

    for (int i = 0; i < automate_src->nb_initiaux; i++) {
        sous_ensembles[0][index++] = automate_src->etats_initiaux[i].numEtat;
    }

    automate_dest.nb_initiaux = 1;
    automate_dest.etats_initiaux = (Etat*)malloc(sizeof(Etat));
    automate_dest.etats_initiaux[0].numEtat = 0;

    int sous_ensemble_courant = 0;
    while (sous_ensemble_courant < nb_sous_ensembles) {
        for (int s = 0; s < nb_symboles; s++) {
            int etats_accessibles[100];
            for (int i = 0; i < 100; i++) etats_accessibles[i] = -1;
            int nb_accessibles = 0;

            int i = 0;
            while (sous_ensembles[sous_ensemble_courant][i] != -1) {
                int etat = sous_ensembles[sous_ensemble_courant][i];

                Transition *t = automate_src->transitions;
                while (t != NULL) {
                    if (t->de.numEtat == etat && t->etiquette == alphabet[s]) {
                        int deja_present = 0;
                        for (int j = 0; j < nb_accessibles; j++) {
                            if (etats_accessibles[j] == t->vers.numEtat) {
                                deja_present = 1;
                                break;
                            }
                        }
                        if (!deja_present) {
                            etats_accessibles[nb_accessibles++] = t->vers.numEtat;
                        }
                    }
                    t = t->suivant;
                }
                i++;
            }
            if (nb_accessibles > 0) {
                int sous_ensemble_existant = -1;
                for (int i = 0; i < nb_sous_ensembles; i++) {
                    int nb_etats_i = 0;
                    while (sous_ensembles[i][nb_etats_i] != -1) nb_etats_i++;

                    if (nb_etats_i == nb_accessibles) {
                        int meme_elements = 1;
                        for (int j = 0; j < nb_accessibles; j++) {
                            int trouve = 0;
                            for (int k = 0; k < nb_etats_i; k++) {
                                if (etats_accessibles[j] == sous_ensembles[i][k]) {
                                    trouve = 1;
                                    break;
                                }
                            }
                            if (!trouve) {
                                meme_elements = 0;
                                break;
                            }
                        }
                        if (meme_elements) {
                            sous_ensemble_existant = i;
                            break;
                        }
                    }
                }
                Etat etat_src = {sous_ensemble_courant};

                if (sous_ensemble_existant == -1) {
                    for (int i = 0; i < nb_accessibles; i++) {
                        sous_ensembles[nb_sous_ensembles][i] = etats_accessibles[i];
                    }

                    Etat etat_dest = {nb_sous_ensembles};
                    ajouter_transition(&automate_dest, etat_src, etat_dest, alphabet[s]);

                    nb_sous_ensembles++;
                } else {
                    Etat etat_dest = {sous_ensemble_existant};
                    ajouter_transition(&automate_dest, etat_src, etat_dest, alphabet[s]);
                }
            }
        }
        sous_ensemble_courant++;
    }
    automate_dest.nb_finaux = 0;
    automate_dest.etats_finaux = NULL;

    for (int i = 0; i < nb_sous_ensembles; i++) {
        int j = 0;
        while (sous_ensembles[i][j] != -1) {
            int etat = sous_ensembles[i][j];
            if (est_etat_final(automate_src, etat)) {
                automate_dest.nb_finaux++;
                automate_dest.etats_finaux = realloc(automate_dest.etats_finaux, automate_dest.nb_finaux * sizeof(Etat));
                automate_dest.etats_finaux[automate_dest.nb_finaux - 1].numEtat = i;
                break;
            }
            j++;
        }
    }

    return automate_dest;
}

Automate transposeAutomate(Automate *automate_src) {
    Automate automate_transpose = {NULL, 0, NULL, 0, NULL, 0, 0};

    automate_transpose.nb_initiaux = automate_src->nb_finaux;
    automate_transpose.etats_initiaux = (Etat*)malloc(automate_src->nb_finaux * sizeof(Etat));
    if (!automate_transpose.etats_initiaux) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < automate_src->nb_finaux; i++) {
        automate_transpose.etats_initiaux[i].numEtat = automate_src->etats_finaux[i].numEtat;
    }

    automate_transpose.nb_finaux = automate_src->nb_initiaux;
    automate_transpose.etats_finaux = (Etat*)malloc(automate_src->nb_initiaux * sizeof(Etat));
    if (!automate_transpose.etats_finaux) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < automate_src->nb_initiaux; i++) {
        automate_transpose.etats_finaux[i].numEtat = automate_src->etats_initiaux[i].numEtat;
    }
    Transition *t = automate_src->transitions;
    while (t != NULL) {
        Etat nouvelle_source = t->vers;
        Etat nouvelle_destination = t->de;
        ajouter_transition(&automate_transpose, nouvelle_source, nouvelle_destination, t->etiquette);

        t = t->suivant;
    }

    return automate_transpose;
}

Automate minimisation(Automate *automate_src) {
    // Etape 1: Renverser l'automate
    Automate automate_transpose = transposeAutomate(automate_src);

    // Etape 2: Determiniser l'automate renverse
    Automate automate_det_transpose = determinisation(&automate_transpose);

    // Liberer l'automate intermediaire
    if (automate_transpose.transitions) liberer_transitions(automate_transpose.transitions);
    if (automate_transpose.etats_initiaux) free(automate_transpose.etats_initiaux);
    if (automate_transpose.etats_finaux) free(automate_transpose.etats_finaux);

    // Etape 3: Renverser a nouveau l'automate
    Automate automate_double_transpose = transposeAutomate(&automate_det_transpose);

    // Liberer l'automate intermediaire
    if (automate_det_transpose.transitions) liberer_transitions(automate_det_transpose.transitions);
    if (automate_det_transpose.etats_initiaux) free(automate_det_transpose.etats_initiaux);
    if (automate_det_transpose.etats_finaux) free(automate_det_transpose.etats_finaux);

    // Etape 4: Determiniser l'automate obtenu
    Automate automate_minimal = determinisation(&automate_double_transpose);

    // Liberer l'automate intermediaire
    if (automate_double_transpose.transitions) liberer_transitions(automate_double_transpose.transitions);
    if (automate_double_transpose.etats_initiaux) free(automate_double_transpose.etats_initiaux);
    if (automate_double_transpose.etats_finaux) free(automate_double_transpose.etats_finaux);

    return automate_minimal;
}

void afficher_mots_du_fichier_et_acceptation(Automate *automate, const char *nom_fichier) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (!fichier) {
        perror("Erreur d'ouverture du fichier");
        return;
    }

    char mot[100];
    printf("\n--- Resultats pour le fichier '%s' ---\n", nom_fichier);

    while (fscanf(fichier, "%s", mot) != EOF) {
        if (tester_mot(automate, mot)) {
            printf("- '%s' : Accepte\n", mot);
        } else {
            printf("- '%s' : Rejete\n", mot);
        }
    }

    fclose(fichier);
}

void menu_mots_automate_minimal(Automate *automate_min) {
    if (automate_min->nb_initiaux == 0) {
        printf("\nErreur : Aucun automate minimal charge. Utilisez d'abord l'option 13.\n");
        return;
    }

    char nom_fichier[100];
    printf("\nEntrez le nom du fichier texte : ");
    scanf("%s", nom_fichier);

    afficher_mots_du_fichier_et_acceptation(automate_min, nom_fichier);
}

Automate intersection_automates(Automate *a1, Automate *a2) {
    Automate resultat = {NULL, 0, NULL, 0, NULL, 0, 0};

    // Tableau pour suivre les etats crees et leur nouveau numero
    int etat_map[100][100];
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 100; j++)
            etat_map[i][j] = -1;

    int next_etat_num = 0;

    // Creer les etats initiaux (produit des etats initiaux)
    for (int i = 0; i < a1->nb_initiaux; i++) {
        for (int j = 0; j < a2->nb_initiaux; j++) {
            int a1_etat = a1->etats_initiaux[i].numEtat;
            int a2_etat = a2->etats_initiaux[j].numEtat;

            if (etat_map[a1_etat][a2_etat] == -1) {
                etat_map[a1_etat][a2_etat] = next_etat_num++;

                // Ajouter comme etat initial
                resultat.etats_initiaux = realloc(resultat.etats_initiaux,
                    (resultat.nb_initiaux + 1) * sizeof(Etat));
                resultat.etats_initiaux[resultat.nb_initiaux].numEtat = etat_map[a1_etat][a2_etat];
                resultat.nb_initiaux++;
            }
        }
    }

    // Parcourir tous les etats accessibles
    int changed;
    do {
        changed = 0;

        for (int q1 = 0; q1 < 100; q1++) {
            for (int q2 = 0; q2 < 100; q2++) {
                if (etat_map[q1][q2] == -1) continue;

                Transition *t1 = a1->transitions;
                while (t1 != NULL) {
                    if (t1->de.numEtat == q1) {

                        Transition *t2 = a2->transitions;
                        while (t2 != NULL) {
                            if (t2->de.numEtat == q2 && t2->etiquette == t1->etiquette) {
                                int new_q1 = t1->vers.numEtat;
                                int new_q2 = t2->vers.numEtat;

                                if (etat_map[new_q1][new_q2] == -1) {
                                    etat_map[new_q1][new_q2] = next_etat_num++;
                                    changed = 1;
                                }

                                Etat de = {etat_map[q1][q2]};
                                Etat vers = {etat_map[new_q1][new_q2]};

                                int existe_deja = 0;
                                Transition *t = resultat.transitions;
                                while (t != NULL) {
                                    if (t->de.numEtat == de.numEtat &&
                                        t->vers.numEtat == vers.numEtat &&
                                        t->etiquette == t1->etiquette) {
                                        existe_deja = 1;
                                        break;
                                    }
                                    t = t->suivant;
                                }

                                if (!existe_deja) {
                                    ajouter_transition(&resultat, de, vers, t1->etiquette);
                                }
                            }
                            t2 = t2->suivant;
                        }
                    }
                    t1 = t1->suivant;
                }
            }
        }
    } while (changed);

    for (int i = 0; i < a1->nb_finaux; i++) {
        for (int j = 0; j < a2->nb_finaux; j++) {
            int a1_etat = a1->etats_finaux[i].numEtat;
            int a2_etat = a2->etats_finaux[j].numEtat;

            if (etat_map[a1_etat][a2_etat] != -1) {
                resultat.etats_finaux = realloc(resultat.etats_finaux,
                    (resultat.nb_finaux + 1) * sizeof(Etat));
                resultat.etats_finaux[resultat.nb_finaux].numEtat = etat_map[a1_etat][a2_etat];
                resultat.nb_finaux++;
            }
        }
    }

    resultat.nb_etats = next_etat_num;
    return resultat;
}

void menu() {
    Automate automate = {NULL, 0, NULL, 0, NULL, 0};
    Automate automate_det = {NULL, 0, NULL, 0, NULL, 0}; // Pour l'automate déterminisé
    Automate automate_min = {NULL, 0, NULL, 0, NULL, 0}; // Pour l'automate minimisé
    int choix;
    FILE *fichier;
    char nom_fichier[50];
    char etiquette;
    const char *expression = "(a+|b*)b+";
    int index = 0;
    int automate_det_genere = 0;
    int automate_min_genere = 0;

    do {
        printf("=====================");
        printf("\n        Menu\n");
        printf("=====================\n");
        printf("1. Lire un automate depuis un fichier .dot\n");
        printf("2. Afficher un automate depuis un fichier .dot\n");
        printf("3. Generer un fichier .dot\n");
        printf("4. Etat avec le plus de transitions sortantes\n");
        printf("5. Etats avec une etiquette specifique\n");
        printf("6. Tester si un mot/fichier est accepte par l'automate initial\n");
        printf("7. Concatenation de deux automates\n");
        printf("8. Union de deux automates\n");
        printf("9. Etoile de Kleene\n");
        printf("10. Construire un automate a partir d'une expression reguliere\n");
        printf("11. Supprimer les epsilon-transitions d'un automate\n");
        printf("12. Determiniser l'automate\n");
        printf("13. Minimiser l'automate\n");
        printf("14. Afficher les mots acceptes par l'automate minimal\n");
        printf("15. Produits de deux automates\n");
        printf("16. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix);
        switch (choix) {
           case 1:
                do {
                    printf("Entrez le nom du fichier .dot : ");
                    scanf("%s", nom_fichier);

                    fichier = fopen(nom_fichier, "r");
                    if (!fichier) {
                        printf("\nErreur d'ouverture du fichier \"%s\": %s\n", nom_fichier, strerror(errno));
                        printf("Veuillez entrer un autre fichier.\n\n");
                    }
                } while (!fichier);

                fclose(fichier); // Fermer le fichier avant de le reouvrir dans lire_fichier
                lire_fichier(nom_fichier, &automate);
                printf("Fichier '%s' lu avec succes.\n", nom_fichier);
                break;
            case 2:
                afficher_automate(&automate);
                break;
            case 3:
                int sous_choix;
                printf("Quel type d'automate voulez-vous generer ?\n");
                printf("1. Automate initial\n");
                printf("2. Automate deterministe\n");
                printf("3. Automate minimal\n");
                printf("4. Tous les automates\n");
                printf("Votre choix : ");
                scanf("%d", &sous_choix);

                switch (sous_choix) {
                    case 1:
                        printf("Entrez le nom du fichier .dot pour l'automate initial : ");
                        scanf("%s", nom_fichier);
                        generer_fichier(nom_fichier, &automate);
                        break;
                    case 2:
                        if (!automate_det_genere) {
                            printf("Vous devez d'abord determiniser l'automate (option 12).\n");
                        } else {
                            printf("Entrez le nom du fichier .dot pour l'automate deterministe : ");
                            scanf("%s", nom_fichier);
                            generer_fichier(nom_fichier, &automate_det);
                        }
                        break;
                    case 3:
                        if (!automate_min_genere) {
                            printf("Vous devez d'abord minimiser l'automate (option 13).\n");
                        } else {
                            printf("Entrez le nom du fichier .dot pour l'automate minimal : ");
                            scanf("%s", nom_fichier);
                            generer_fichier(nom_fichier, &automate_min);
                        }
                        break;
                    case 4:
                        printf("Entrez le prefixe pour les fichiers .dot : ");
                        scanf("%s", nom_fichier);

                        char nom_initial[100], nom_det[100], nom_min[100];
                        sprintf(nom_initial, "%s_initial.dot", nom_fichier);
                        sprintf(nom_det, "%s_deterministe.dot", nom_fichier);
                        sprintf(nom_min, "%s_minimal.dot", nom_fichier);

                        generer_fichier(nom_initial, &automate);

                        if (!automate_det_genere) {
                            printf("L'automate deterministe n'est pas disponible. Generez-le d'abord.\n");
                        } else {
                            generer_fichier(nom_det, &automate_det);
                        }

                        if (!automate_min_genere) {
                            printf("L'automate minimal n'est pas disponible. Generez-le d'abord.\n");
                        } else {
                            generer_fichier(nom_min, &automate_min);
                        }
                        break;
                    default:
                        printf("Choix invalide. Retour au menu principal.\n");
                }
                break;
             case 4:
                afficherEtat_maxTransitions(&automate);
                break;
            case 5:
                afficherEtats_TransitionEtiquette(&automate, etiquette);
                break;
            case 6:
                tester(nom_fichier, &automate);
                break;
            case 7: {
                // Concaténation
                if (automate.nb_initiaux == 0) {
                    printf("Veuvez d'abord charger un automate (option 1)!\n");
                    break;
                }

                char fichierB[50];
                printf("Nom du deuxieme automate (.dot): ");
                scanf("%s", fichierB);

                // Charger le deuxieme automate
                Automate automateB = {NULL, 0, NULL, 0, NULL, 0};
                lire_fichier(fichierB, &automateB);

                // Effectuer la concaténation
                 Automate* resultat = concatenation(&automate, &automateB);

                if (resultat) {
                    // Nettoyer l'ancien automate
                    liberer_transitions(automate.transitions);
                    free(automate.etats_initiaux);
                    free(automate.etats_finaux);

                    // Remplacer par le résultat
                    automate.transitions = resultat->transitions;
                    automate.etats_initiaux = resultat->etats_initiaux;
                    automate.etats_finaux = resultat->etats_finaux;
                    automate.nb_initiaux = resultat->nb_initiaux;
                    automate.nb_finaux = resultat->nb_finaux;
                    automate.nb_transitions = resultat->nb_transitions;

                    free(resultat); // On ne garde que les pointeurs internes
                    printf("Concatenation reussie!\n");
                }

                // Nettoyer automateB
                liberer_transitions(automateB.transitions);
                free(automateB.etats_initiaux);
                free(automateB.etats_finaux);
                break;
            }
            case 8: {
                if (automate.nb_initiaux == 0) {
                    printf("Veuvez d'abord charger un automate (option 1)!\n");
                    break;
                }

                char fichierB[50];
                printf("Nom du deuxieme automate (.dot): ");
                scanf("%s", fichierB);

                // Charger le deuxieme automate
                Automate automateB = {NULL, 0, NULL, 0, NULL, 0};
                lire_fichier(fichierB, &automateB);

                // Effectuer la concaténation
                Automate* resultat = union_automates(&automate, &automateB);

                if (resultat) {
                    // Nettoyer l'ancien automate
                    liberer_transitions(automate.transitions);
                    free(automate.etats_initiaux);
                    free(automate.etats_finaux);

                    // Remplacer par le résultat
                    automate.transitions = resultat->transitions;
                    automate.etats_initiaux = resultat->etats_initiaux;
                    automate.etats_finaux = resultat->etats_finaux;
                    automate.nb_initiaux = resultat->nb_initiaux;
                    automate.nb_finaux = resultat->nb_finaux;
                    automate.nb_transitions = resultat->nb_transitions;

                    free(resultat); // On ne garde que les pointeurs internes
                    printf("Union reussie!\n");
                }

                // Nettoyer automateB
                liberer_transitions(automateB.transitions);
                free(automateB.etats_initiaux);
                free(automateB.etats_finaux);
                break;
            }
            case 9: {
                if (automate.nb_initiaux == 0) {
                    printf("Veuvez d'abord charger un automate (option 1)!\n");
                    break;
                }

                // Appliquer l'étoile de Kleene
                Automate* resultat = kleene_etoile(&automate);

                if (resultat) {
                    // Nettoyer l'ancien automate
                    liberer_transitions(automate.transitions);
                    free(automate.etats_initiaux);
                    free(automate.etats_finaux);

                    // Remplacer par le résultat
                    automate.transitions = resultat->transitions;
                    automate.etats_initiaux = resultat->etats_initiaux;
                    automate.etats_finaux = resultat->etats_finaux;
                    automate.nb_initiaux = resultat->nb_initiaux;
                    automate.nb_finaux = resultat->nb_finaux;
                    automate.nb_transitions = resultat->nb_transitions;

                    free(resultat); // On ne garde que les pointeurs internes
                    printf("Etoile de Kleene appliquee avec succes!\n");
                }
                break;
            }
            case 10: {
                    // Libérer l'automate existant s'il y en a un
                    if (automate.transitions) liberer_transitions(automate.transitions);
                    if (automate.etats_initiaux) free(automate.etats_initiaux);
                    if (automate.etats_finaux) free(automate.etats_finaux);

                    // Réinitialiser l'automate
                    automate = (Automate){NULL, 0, NULL, 0, NULL, 0};

                    // Générer un nouvel automate à partir de l'expression régulière
                    index = 0; // Réinitialiser l'index pour la récursion
                    Automate *nouvel_automate = expression_to_automate_recursive(expression, &index);

                    if (nouvel_automate) {
                        // Copier le nouvel automate dans la variable automate du menu
                        automate = *nouvel_automate;
                        printf("Automate genere avec succes pour l'expression : %s\n", expression);
                    } else {
                        printf("Erreur lors de la generation de l'automate.\n");
                    }
                    break;
            }
            case 11: {
                // Supprimer les ε-transitions
                if (automate.nb_initiaux == 0) {
                    printf("Aucun automate charge. Veuillez d'abord charger un automate (option 1).\n");
                }else {
                    Automate automate_sans_epsilon = supprimerEpsilonTransitions(&automate);
                    printf("Les epsilons-transitions ont ete supprimees avec succes.\n");

                    // Supprimer les états finaux isolés
                    supprimer_etats_finaux_isoles(&automate_sans_epsilon);

                    // Libérer l'automate original et l'automate intermédiaire
                    liberer_transitions(automate.transitions);
                    free(automate.etats_initiaux);
                    free(automate.etats_finaux);

                    // Remplacer l'automate par le nouvel automate sans états redondants
                    automate = automate_sans_epsilon;
                }
                break;
            }
            case 12: {
                // Determinisation
                automate_det = determinisation(&automate);
                printf("Automate determinise avec succes.\n");
                automate_det_genere = 1;
                automate=automate_det;
                break;
            }
            case 13: {
                // Minimisation
                automate_min = minimisation(&automate);
                printf("Automate minimise avec succes.\n");
                automate_min_genere = 1;
                automate = automate_min;
                break;
            }
            case 14:
                menu_mots_automate_minimal(&automate_min);
            break;

            case 15: {
                    if (automate.nb_initiaux == 0) {
                        printf("Veuillez d'abord charger un automate (option 1)!\n");
                        break;
                    }

                    char fichierB[50];
                    printf("Nom du deuxieme automate (.dot): ");
                    scanf("%s", fichierB);

                    // Charger le deuxieme automate
                    Automate automateB = {NULL, 0, NULL, 0, NULL, 0};
                    lire_fichier(fichierB, &automateB);

                    // Effectuer l'intersection
                    Automate resultat = intersection_automates(&automate, &automateB);

                    printf("Intersection reussie!\n");
                    afficher_automate(&resultat);

                    // Demander s’il veut enregistrer le résultat
                    char save_nom[50];
                    printf("Voulez-vous sauvegarder l'automate d'intersection dans un fichier .dot ? (nom ou \"non\") : ");
                    scanf("%s", save_nom);

                    if (strcmp(save_nom, "non") != 0) {
                        generer_fichier(save_nom, &resultat);
                        printf("Fichier '%s' genere avec succes.\n", save_nom);
                    }

                    // Libération mémoire
                    liberer_transitions(automateB.transitions);
                    free(automateB.etats_initiaux);
                    free(automateB.etats_finaux);

                    liberer_transitions(resultat.transitions);
                    free(resultat.etats_initiaux);
                    free(resultat.etats_finaux);

                    break;

            }
            case 16:
                // Libération de la mémoire et sortie
                    if (automate.transitions) liberer_transitions(automate.transitions);
            if (automate.etats_initiaux) free(automate.etats_initiaux);
            if (automate.etats_finaux) free(automate.etats_finaux);

            if (automate_det.transitions) liberer_transitions(automate_det.transitions);
            if (automate_det.etats_initiaux) free(automate_det.etats_initiaux);
            if (automate_det.etats_finaux) free(automate_det.etats_finaux);

            if (automate_min.transitions) liberer_transitions(automate_min.transitions);
            if (automate_min.etats_initiaux) free(automate_min.etats_initiaux);
            if (automate_min.etats_finaux) free(automate_min.etats_finaux);

            printf("Au revoir !\n");
            break;

            default:
                printf("Choix invalide. Veuillez reessayer.\n");
        }
    } while (choix != 16);
}

int main() {
    menu();
    return 0;
}
