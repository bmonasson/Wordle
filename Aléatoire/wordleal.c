/* Wordle Aléatoire
Même principe que la stratégie élimine mais quand le nombre de mots est trop important, 
on en sélectionne une partie au hasard et on trouve le meilleur mot parmis ce sous ensemble
au lieu de trouver le meilleur mot parmis l'entièreté du dictionnaire*/



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>


int NB_REPONSES = 2315;
//int NB_MOTS = 2315;
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

typedef struct Liste_int{
    int val;
    struct Liste_int* suiv;
}int_lst;


void print_tab(int* tab){
    for (int i=0; i<L_MOT; i++){
        printf("%d", tab[i]);
    }
    printf("\n");
}



void print_liste(lst* liste){
    //fonction oour imprimer une liste chainee
    int i=0;
    //printf("\n\n\n");
    for (lst* l=liste; l!=NULL; l=l->suiv){
        printf("(%d, %s) -> ", i, l->mot);
        i++;
    }
    printf("\n\n\n");
    return;
}

void print_int_liste(int_lst* liste){
    for (int_lst* l = liste; l!=NULL; l=l->suiv){
        printf("%d -> ", l->val);
    }
    printf("\n");
}

char** remplir_dictionnaire(){
    //fonction qui renvoie un tableau de str qui correspond a la liste de tous les mots du dictionnaire : "dictionnaire.txt"
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
    while(b && i < 2*NB_MOTS) {
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
    //renvoie si deux chaines de caracteres sont egales
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
    //on verifie si le mot m2 (mot quelconque) est compatible avec une couleur d'une lettre de m1 (essai)
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

bool compatible_mot(char* mot, char* essai, int* config){
    //renvoie si le mot "mot" est compatible avec les informations obtenues avec l'essai "essai"
    for (int i=0; i<L_MOT; i++){
        if (!compatible_lettre(essai, mot, i, config[i])){
            return false;
        }
    }
    return true;
}

int* configuration(char* essai, char* objectif){
    //renvoie un tableau de 5 chiffres qui correspond à la réponse (configuration) du jeu pour l'essai essai
    // par exemple 00120 --> gris gris jaune vert gris
    int* config = malloc(sizeof(int)*5);
    for (int i=0; i<L_MOT; i++){
        if (essai[i] == objectif[i]){ //lettre verte
            config[i] = 2;
        }
        else if (dans(essai[i], objectif)){ //lettre jaune
            config[i] = 1;
        }
        else { //lettre grise
            config[i] = 0;
        }
    }
    return config;
}

bool prochaine_configuration(int* config){
    //modifie la configuration pour passer à celle d'apres
    //si on est à la derniere on renvoie false sinon true
    //on effectue une incrementation de 1 de config en base 3
    config[4] ++;
    int i=4;
    while (i > 0 && config[i] == 3){
        config[i] = 0;
        config[i-1] ++;
        i--;
    }
    if (config[0] == 3){
        return false;
    }
    return true;
}

float nb_elimines(char* mot, memoire* mem){
    //renvoie le nb de mots elimines en moyenne si on choisit de prendre mot comme essai
    assert(mem->nb_restants != 0);
    float nb_eli = 0;
    int config[5] = {0,0,0,0,-1};
    while (prochaine_configuration(config)){
        float compt = 0;
        for (lst* l = mem->restants; l != NULL; l = l->suiv){
            if (compatible_mot(l->mot, mot, config)){
                compt ++;
            }
        }
        nb_eli += (mem->nb_restants - compt)*compt/(mem->nb_restants);
    }
    return nb_eli;
}

lst* maj_restants(char* essai, char* objectif, memoire* mem){
    lst* liste = mem->restants;
    //tant que le premier mot n'est pas compatible on le supprime de la liste
    while (liste != NULL && !compatible_mot(liste->mot, essai, configuration(essai, objectif))){
        //printf("%s, %d\n", liste->mot, mem->nb_restants);
        liste = liste->suiv;
        mem->nb_restants --;
    }
    //si la liste est vide on la renvoie
    //printf("nb rest %d\n", mem->nb_restants);
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
    while (l->suiv != NULL){
        //si le prochain mot n'est pas compatible, on le supprime de la liste
        if (!compatible_mot(l->suiv->mot, essai, configuration(essai, objectif))){
            l->suiv = l->suiv->suiv;
            mem->nb_restants --;
            
        }
        //sinon on passe au mot d'apres
        else {
            l = l->suiv;
        }
    }
    //printf("nb restants : %d\n", mem->nb_restants);
    //printf("maj :\n");
    //print_liste(liste);
    return liste;
}


int_lst* liste_aleatoire(int n){
    //renvoie une liste aléatoire de n nombres sans doublons
    //(ici compris entre 0 et NB_MOTS)
    assert(n<NB_MOTS);
    int* decalage = malloc(sizeof(int)*NB_MOTS);
    for (int i=0; i<NB_MOTS; i++){
        decalage[i] = 0;
    }
    int_lst* res = NULL;
    int r; 
    int tirage;
    for (int i=0; i<n; i++){
        r = rand()%NB_MOTS;
        tirage = r - decalage[r];
        while (tirage < 0){
            r = rand()%NB_MOTS;
            tirage = r - decalage[r];
        }
        for (int j=0; j<=tirage; j++){
            decalage[j] ++;
        }
        int_lst* new = malloc(sizeof(int_lst));
        new->val = tirage;
        new->suiv = res;
        res = new;
    }
    return res;

};


bool essai(char* objectif, char** dictionnaire,  memoire* mem, int div){
    //printf("debut essai\n");
    //correspond a un essai (sur les 5 possibles)
    coup* meilleur_coup = malloc(sizeof(coup));
    meilleur_coup->sc = -1;

    if (mem->nb_restants == 1){ //s'il ne reste plus qu'un seul mot possible on le joue
        meilleur_coup->mot = mem->restants->mot;
    }
    else if (mem->nb_restants == NB_MOTS && NB_MOTS == 14855){ //pour eviter le premier calcul qui est tres long
        meilleur_coup->mot = "lares";
    }
    else if (mem->nb_restants == NB_MOTS && NB_MOTS == 2315){ //pour eviter le premier calcul qui est tres long
        meilleur_coup->mot = "roate";
    }
    else if (mem->nb_restants <= 100){
        for (int i=0; i<NB_MOTS; i++){
            // a chaque mot on attribue un score qui correspond au nb de mots qu'il va elimier en moyenne
            float score = 0;
            score = nb_elimines(dictionnaire[i], mem);
            // if (mem->nb_restants == 2){
            //     printf("mot : %s, score %f\n", dictionnaire[i], score);
            // }
            //printf("score %d\n", score);
            if (score > meilleur_coup->sc){
                meilleur_coup->sc = score;
                meilleur_coup->mot = dictionnaire[i];
            }

        }
    }
    else {
        int_lst* liste = liste_aleatoire(NB_MOTS/div);
        for (int_lst* l=liste; l!=NULL; l=l->suiv){
            int i = l->val;
            // a chaque mot on attribue un score qui correspond au nb de mots qu'il va elimier en moyenne
            float score = 0;
            score = nb_elimines(dictionnaire[i], mem);
            // if (mem->nb_restants == 2){
            //     printf("mot : %s, score %f\n", dictionnaire[i], score);
            // }
            //printf("score %d\n", score);
            if (score > meilleur_coup->sc){
                meilleur_coup->sc = score;
                meilleur_coup->mot = dictionnaire[i];
            }

        }
    }

    //printf("%s, score : %f\n", meilleur_coup->mot, meilleur_coup->sc);
    if(egal(meilleur_coup->mot, objectif)){ //on a trouve le mot
        return true;
    }
    else{ //on met a jour la memoire
        mem->restants = maj_restants(meilleur_coup->mot, objectif, mem);
    }
    return false;

}



void statistiques(char** dico, int nb_mots, int div){
    //fonction qui teste l'algo sur les nb_mots premiers mots du dictionnaire et qui effectue des stats sur le nb d'essais pris a chaque fois
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
    for (int mot=0; mot<nb_mots; mot++){
        printf("%d\n", mot);
        objectif = dico[mot];

        mem->nb_restants = nb_mots;
        //on remplit la liste des mots restants avec tous les mots du dictionnaire
        lst* l = malloc(sizeof(lst));
        mem->restants = l;
        for (int i=0; i<nb_mots; i++){
            l->mot = dico[i];
            if (i == nb_mots-1){
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
            //printf("       -> %d  (nb restants : %d)\n", nb_essais, mem->nb_restants);
            trouve = essai(objectif, dico, mem, div);
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



void partie(char** dico, int div){
    //fonction qui simule une partir de wordle
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


    clock_t start, end;
    double temps;

    bool trouve = false;
    int nb_essais = 1;
    while (!trouve && nb_essais<15){
        start = clock();
        printf("Essai numéro %d : ", nb_essais);
        trouve = essai(objectif, dico, mem, div);
        end = clock();
        temps = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Temps pris : %f\n", temps);
        nb_essais++;
    }
}

int main(){
    srand(time(NULL));

    //on récupère la liste de tous les mots
    char** dico = malloc(sizeof(char*)*NB_MOTS);
    dico = remplir_dictionnaire();
    

    //statistiques(dico, NB_REPONSES);
    partie(dico, 10);

    /*
    for (int i=0; i<5; i++){
        print_int_liste(liste_aleatoire(10));
        printf("\n\n");
    }
    print_int_liste(liste_aleatoire(NB_MOTS/10));
    */


    /*
    // int* config = malloc(sizeof(int)*5);
    // for (int i=0; i<4; i++){
    //     config[i] = 0;
    // }
    // config[4] = -1;
    int config[5] = {0,0,0,0,-1};
    int cont = 0;
    while (prochaine_configuration(config)){
        print_tab(config);
        cont ++;
    }
    printf("nb total de configs : %d\n", cont);
    */
}