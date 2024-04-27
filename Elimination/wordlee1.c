/* Wordle Elimine V1
On parcourt l'entièreté des mots et on trouve celui qui en élimine un maximum */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>


int NB_REPONSES = 2315;
int NB_MOTS = 14855;
int L_MOT = 5;

typedef struct Coup{
    char* mot;
    float sc;
}coup;

typedef struct Liste{
    char* mot;
    struct Liste* suiv;
}lst;

typedef struct Memoire{
    lst* restants;
    int nb_restants;
}memoire;

void print_liste(lst* liste){
    int i=0;
    //printf("\n\n\n");
    for (lst* l=liste; l!=NULL; l=l->suiv){
        printf("(%d, %s) -> ", i, l->mot);
        i++;
    }
    printf("\n\n\n");
    return;
}

char** remplir_dictionnaire(){
    //Ouvre le fichier et récupère tous les mots du dictionnaire dans un tableau de str qui est renvoyé
    FILE *fptr;

    // Open a file in read mode
    fptr = fopen("dictionnaire.txt", "r");

    // Store the content of the file
    char** dico = malloc(sizeof(char*)*NB_MOTS);
    char* poubelle = malloc(sizeof(char)*10);
    for (int i=0; i<NB_MOTS; i++){
        char* mot = malloc(sizeof(char)*5);
        dico[i] = mot;
    }

    // If the file exist
    if(fptr != NULL) {

    // Read the content and print it
    int i=0;
    bool b = true;
    while(b) {
        if (i%2 == 0){
            b = fgets(dico[i/2], 6, fptr);
            //printf("%d %s\n", i, dico[i/2]);
        }
        else {
            b = fgets(poubelle, 6, fptr);
        }
        i++;
        
    }

    // If the file does not exist
    } else {
    printf("Not able to open the file.");
    }

    // Close the file
    fclose(fptr);
    return dico;
}

bool egal(char* mot1, char* mot2){
    for (int i=0; i<L_MOT; i++){
        if (mot1[i] != mot2[i]){
            return false;
        }
    }
    return true;
}

bool dans(char lettre, char* mot){
    //renvoie true si la lettre est présente dans le mot
    for (int i=0; i<L_MOT; i++){
        if (lettre == mot[i]){
            return true;
        }
    }
    return false;
}


bool compatible_lettre(char* m1, char* m2, int i_lettre, int statut){
    //on verifie si le mot m2 est compatible avec une couleur d'une lettre de m1
    //m1 : essai et m2 : mot
    //l'entier statut correspond à si la lettre est grise (0) / jaune (1) ou verte (2)
    
   if (statut == 0){
       return (!dans(m1[i_lettre], m2));
   }
   if (statut == 1){
       return (dans(m1[i_lettre], m2) && m1[i_lettre] != m2[i_lettre]);
   }
   if (statut == 2){
       return (m1[i_lettre] == m2[i_lettre]);
   }
   else return false;

}

bool compatible_mot(char* mot, char* essai, char* objectif){
    //renvoie si le mot "mot" est compatible avec les informations obtenues avec l'essai "essai"
    // bool comp = true;
    // for (int i=0; i<L_MOT; i++){
    //     if (essai[i] == objectif[i]){ //lettre verte
    //         comp = compatible_lettre(essai, mot, i, 2);
    //     }
    //     else {
    //         if (dans(essai[i], objectif)){ //lettre jaune
    //             comp = compatible_lettre(essai, mot, i, 1);
    //         }
    //         else { //lettre grise
    //             comp = compatible_lettre(essai, mot, i, 0);
    //         }
    //     }
    //     if (!comp){
    //         return false;
    //     }
    // }
    //return true;
    //bool comp = true;
    for (int i=0; i<L_MOT; i++){
        if (essai[i] == objectif[i]){ //lettre verte
            if (!(essai[i] == mot[i])){
                return false;
            }
        }
        else if (dans(essai[i], objectif)){ //lettre jaune
            if (!(dans(essai[i], mot) && essai[i] != mot[i])){
                return false;
            }
        }
        else { //lettre grise
            if (dans(essai[i], mot)){
                return false;
            }
        }
    }
    return true;
}

float nb_elimines(char* mot, memoire* mem){
    assert(mem->nb_restants != 0);
    float nb_eli = 0;
    for (int i=0; i<L_MOT; i++){
        for (int j=0; j<3; j++){
            float compt = 0;
            for (lst* l = mem->restants; l != NULL; l = l->suiv){
                if (compatible_lettre(mot, l->mot, i, j)){
                    compt ++;
                }
            }
            nb_eli += (mem->nb_restants - compt)*compt/(mem->nb_restants);
        }
    }
    return nb_eli;
}

lst* maj_restants(char* essai, char* objectif, memoire* mem){
    lst* liste = mem->restants;
    //printf("AVANT :\n");
    //print_liste(liste);
    //tant que le premier mot n'est pas compatible on le supprime de la liste
    while (liste != NULL && !compatible_mot(liste->mot, essai, objectif)){
        //printf("%s, %d\n", liste->mot, mem->nb_restants);
        liste = liste->suiv;
        mem->nb_restants --;
    }
    //si la liste est vide on la renvoie
    //printf("nb rest %d\n", mem->nb_restants);
    //printf("intermediaire\n");
    //print_liste(liste);
    if (liste == NULL){ 
        //printf("Liste vide\n");
        return liste;
    }
    //printf("liste non nulle\n");
    //liste possède au moins un élément donc on peut lancer cette super boucle
    //on sait que le premier élément est forcément compatible sinon il aurait été supprimé 
    //par le premier while (et cela va rester vrai tout au long de la boucle l->mot est compatible)
    lst* l=liste; 
    int i=0;
    while (l->suiv != NULL){
        //si le prochain mot n'est pas compatible, on le supprime de la liste
        if (!compatible_mot(l->suiv->mot, essai, objectif)){
            l->suiv = l->suiv->suiv;
            mem->nb_restants --;
            i++;
        }
        //sinon on passe au mot d'apres
        else {
            l = l->suiv;
        }
    }
    //printf("nb restants : %d, nb supps %d\n", mem->nb_restants, i);
    //printf("Apres\n");
    //print_liste(liste);
    return liste;
}

bool essai(char* objectif, char** dictionnaire,  memoire* mem){
    //printf("debut essai\n");
    coup* meilleur_coup = malloc(sizeof(coup));
    meilleur_coup->sc = -1;

    if (mem->nb_restants == 1){
        meilleur_coup->mot = mem->restants->mot;
    }
    else if (mem->nb_restants == NB_MOTS){
        meilleur_coup->mot = "sease";
    }
    else {
        for (int i=0; i<NB_MOTS; i++){

            float score = 0;
            score = nb_elimines(dictionnaire[i], mem);
            //printf("score %d\n", score);
            if (score > meilleur_coup->sc){
                meilleur_coup->sc = score;
                meilleur_coup->mot = dictionnaire[i];
            }

        }
    }

    //printf("%s\n", meilleur_coup->mot);
    if(egal(meilleur_coup->mot, objectif)){
        return true;
    }
    else{
        mem->restants = maj_restants(meilleur_coup->mot, objectif, mem);
    }
    return false;

}



void statistiques(char** dico){
    clock_t start_pre, end_pre, start, end;
    double temps_pre, temps;
    start_pre = clock();


    int* stat = malloc(sizeof(int)*27);
    for (int i=0; i<27; i++){
        stat[i] = 0;
    }
    char* objectif = malloc(sizeof(char)*5);
    memoire* mem = malloc(sizeof(memoire));

    end_pre = clock();

    start = clock();
    for (int mot=0; mot<NB_MOTS; mot++){
        printf("%d\n", mot);
        objectif = dico[mot];

        mem->nb_restants = NB_MOTS;
        //on remplit la liste des mots restants avec tous les mots du dictionnaire
        lst* l = malloc(sizeof(lst));
        mem->restants = l;
        for (int i=0; i<NB_MOTS; i++){
            l->mot = dico[i];
            if (i == NB_MOTS-1){
                l->suiv = NULL;
            }
            else {
                l->suiv = malloc(sizeof(lst));
            }
            l = l->suiv;
        }
        
        bool trouve = false;
        int nb_essais = 1;
        while (!trouve && nb_essais<30){
            trouve = essai(objectif, dico, mem);
            nb_essais++;
        }
        if (trouve){
            stat[nb_essais - 1] ++;
        }
       else { //si le mot n'a pas ete trouve
            stat[26] ++;
            printf("pas trouvé : %s\n", dico[mot]);
        }        

        //on affiche les statistiques tous les 500 mots
        if (mot%500 == 0){
            for (int i=1; i<26; i++){
                printf("%d essais : %d / ", i, stat[i]);
            }
            printf("pas trouvés : %d\n", stat[26]);
        }
    }
    end = clock();
    temps = ((double) (end - start)) / CLOCKS_PER_SEC;
    temps_pre = ((double) (end_pre - start_pre)) / CLOCKS_PER_SEC;
    printf("Temps utilisé total: %f s\n", temps + temps_pre);
    printf("Précalcul %f s\n", temps_pre);
    printf("Par mot : %f s\n", temps/NB_MOTS);
    int nb_essais_tot = 0;
    for (int i=0; i<26; i++){
        nb_essais_tot += i*stat[i];
    }
    printf("Par essai : %f s  (nb essais tot : %d)\n", temps/nb_essais_tot, nb_essais_tot);
    for (int i=1; i<26; i++){
        printf("Nombre de mots trouvés au bout de %d essais : %d\n", i, stat[i]);
    }
    printf("Nombre de mots pas trouvés : %d\n", stat[26]);

    
}



void partie(char** dico){
    //on choisit un mot au hasard : celui qu'on va devoir trouver
    int choix;
    printf("Voulez vous choisir le mot (0) ou aléatoire (1) ?\n");
    scanf("%d", &choix);
    char* objectif = malloc(sizeof(char)*5);
    if (choix == 0){
        printf("\nVeuillez entrer un mot de 5 lettres\n");
        scanf("%s", objectif);
    }
    else {
        objectif = dico[rand()%NB_REPONSES];
    }
    
    printf("Objectif : %s\n", objectif);

    //on initialise la mémoire
    //printf("debut initialisation memoire\n");
    memoire* mem = malloc(sizeof(memoire));
    mem->nb_restants = NB_MOTS;
    //on remplit la liste des mots restants avec tous les mots du dictionnaire
    lst* l = malloc(sizeof(lst));
    mem->restants = l;
    for (int i=0; i<NB_MOTS; i++){
        l->mot = dico[i];
        if (i == NB_MOTS-1){
            l->suiv = NULL;
        }
        else {
            l->suiv = malloc(sizeof(lst));
        }
        l = l->suiv;
    }
    //printf("fin initialisation memoire\n");

    bool trouve = false;
    int nb_essais = 1;
    while (!trouve && nb_essais<15){
        printf("Essai numéro %d : ", nb_essais);
        trouve = essai(objectif, dico, mem);
        nb_essais++;
    }
}

int main(){
    srand(time(NULL));

    //on récupère la liste de tous les mots
    char** dico = malloc(sizeof(char*)*NB_MOTS);
    dico = remplir_dictionnaire();

    statistiques(dico);
    //partie(dico);
}