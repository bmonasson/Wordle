/* Wordle Hybride
On utilise d'abord la stratégie heuristique et ensuite élimine */
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

typedef struct H_Liste{
    char lettre;
    struct H_Liste* suiv;
}H_lst;

typedef struct E_Liste{
    char* mot;
    struct E_Liste* suiv;
}E_lst;

typedef struct Memoire{
    char* connus;
    H_lst* a_utiliser;
    bool** a_eviter; 
    E_lst* restants;
    int nb_restants;
}memoire;



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

//FONCTIONS COMMUNES
bool dans(char lettre, char* mot){
    //renvoie true si la lettre est présente dans le mot
    for (int i=0; i<5; i++){
        if (lettre == mot[i]){
            return true;
        }
    }
    return false;
}


bool egal(char* m1, char* m2){
    //renvoie true si les deux mots sont égaux
    for (int i=0; i<L_MOT; i++){
        if (m1[i] != m2[i]){
            return false;
        }
    }
    return true;
}

//FONCTIONS HEURISTIQUE
int indice(int i){
    //pour compenser le fait que ascii(a) = 97
    return i - 97;
}

int nb_lettres_diff(char* mot){
    //renvoie le nb de lettres différentes du mot
    int c = 0;
    for (int i=0; i<5; i++){
        bool en_double = false;
        for (int j=i+1; j<5; j++){
            if (mot[i] == mot[j]){
                en_double = true;
            }
        }
        if (!en_double){
            c ++;
        }
    }
    return c;
}

H_lst* ajouter_a_utiliser(char lettre, H_lst* liste){
    //ajoute la lettre à la liste des lettres à utiliser
    bool dedans = false;
    for (H_lst* l=liste; l!=NULL; l=l->suiv){
        if (lettre == l->lettre){
            dedans = true;
        }
    }
    if (dedans){ //si la lettres est deja dans la liste on ne fait rien 
        return liste;
    }
    //sinon on la rajoute au début 
    H_lst* new = malloc(sizeof(H_lst));
    new->lettre  = lettre;
    new->suiv = liste;
    return new;
}


coup* strategie_heuristique(char** dico, memoire* mem){
    /*
    Heuristique : 
        - +15 pts si on met une lettre à son emplacement connu (vert)
        - -5 pts si on met une lettre à un endroit qui ne peut pas être son emplacement
        - -5 pts par lettre, qu'on sait être dans le mot, non utilisée
        - +2 pts par lettre différente du mot
    */
    coup* meilleur = malloc(sizeof(coup));
    meilleur->sc = 0;

    for (int i=0; i<NB_MOTS; i++){
        float score = 0;
        for (int j=0; j<5; j++){
            if (dico[i][j] == mem->connus[j]){
                score = score + 15;
            }
            else if (mem->a_eviter[indice(dico[i][j])][j]){
                score = score - 5;
            }

        }

        for (H_lst* l = mem->a_utiliser; l != NULL; l = l->suiv){
            if (!dans(l->lettre, dico[i])){
                score = score - 5;
            }
        }

        score = score + 2*nb_lettres_diff(dico[i]);

        if (score > meilleur->sc){
            meilleur->mot = dico[i];
            meilleur->sc = score;
        }
    }
    return meilleur;

}



//FONCTIONS ELIMINATOIRES
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
        for (E_lst* l = mem->restants; l != NULL; l = l->suiv){
            if (compatible_mot(l->mot, mot, config)){
                compt ++;
            }
        }
        nb_eli += (mem->nb_restants - compt)*compt/(mem->nb_restants);
    }
    return nb_eli;
}

E_lst* maj_restants(char* essai, char* objectif, memoire* mem){
    E_lst* liste = mem->restants;
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
    E_lst* l=liste; 
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


coup* strategie_eliminatoire(char** dictionnaire,  memoire* mem){
    coup* meilleur_coup = malloc(sizeof(coup));
    meilleur_coup->sc = -1;

    if (mem->nb_restants == 1){ //s'il ne reste plus qu'un seul mot possible on le joue
        meilleur_coup->mot = mem->restants->mot;
    }
    // else if (mem->nb_restants == NB_MOTS){ //pour eviter le premier calcul qui est tres long
    //     meilleur_coup->mot = "lares";
    // }
    else if (mem->nb_restants == NB_REPONSES){ //pour eviter le premier calcul qui est tres long
        meilleur_coup->mot = "roate";
    }
    else {
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
    return meilleur_coup;
}


void maj_memoire(coup* meilleur, memoire* mem, char* objectif){
    //maj strategie heuristique
    for (int i=0; i<L_MOT; i++){
        //vert : on connait l'emplacement d'une lettre du mot final
        if (meilleur->mot[i] == objectif[i]){
            mem->connus[i] = meilleur->mot[i];
        }
        else {
            //jaune : on connait une lettre du mot final mais pas de son emplacement
            if (dans(meilleur->mot[i], objectif)){
                mem->a_eviter[indice(meilleur->mot[i])][i] = true;
                mem->a_utiliser = ajouter_a_utiliser(meilleur->mot[i], mem->a_utiliser);

            }
            //gris : on sait que cette lettre n'est pas dans ce mot
            else {
                for (int j=0; j<L_MOT; j++){
                    mem->a_eviter[indice(meilleur->mot[i])][j] = true;
                }
            }
        }
    }

    
    //maj strategie eliminatoire
    mem->restants = maj_restants(meilleur->mot, objectif, mem);
}




bool essai(char* objectif, char** dico, memoire* mem, bool strat, bool partie){
    //fonction qui simule un essai et qui renvoie un bool
    // true -> le mot objectif est trouvé
    // false -> l'essai ne correspond pas à l'objectif
    
    
    //en fonction du bool strat, on va utiliser la strategie heuristique / eliminatoire
    coup* meilleur = malloc(sizeof(coup));
    if (strat){ //on utilise la stategie heuristique pour trouver le mot
        meilleur = strategie_heuristique(dico, mem);
    }
    else {
        meilleur = strategie_eliminatoire(dico, mem);
    }
    

    if (partie){
        printf("%s\n", meilleur->mot);
    }


    if (egal(meilleur->mot, objectif)){
        return true; //on a trouvé le mot recherché
    }

    //on met à jour la mémoire
    maj_memoire(meilleur, mem, objectif);
    

    return false; //on n'a pas trouvé le mot recherché

}

memoire* initialisation_mem(char** dico){
    memoire* mem = malloc(sizeof(memoire));
    mem->connus = malloc(sizeof(char)*L_MOT);
    for(int i=0; i<L_MOT; i++){
        mem->connus[i] = '*';
    }
    mem->a_utiliser = NULL;
    mem->a_eviter = malloc(sizeof(bool*)*26);
    for (int i=0; i<26; i++){
        bool* ligne = malloc(sizeof(bool)*L_MOT);
        for (int j=0; j<L_MOT; j++){
            ligne[j] = false;
        }
        mem->a_eviter[i] = ligne;
    }
    mem->nb_restants = NB_MOTS;
    //on remplit la liste des mots restants avec tous les mots du dictionnaire
    E_lst* l = malloc(sizeof(E_lst));
    mem->restants = l;
    for (int i=0; i<NB_MOTS; i++){
        l->mot = dico[i];
        if (i == NB_MOTS-1){
            l->suiv = NULL;
        }
        else {
            l->suiv = malloc(sizeof(E_lst));
        }
        l = l->suiv;
    }
    return mem;
}


void partie(char** dico, int k){
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
    
    printf("\nObjectif : %s\n", objectif);

    //on initialise la mémoire
    memoire* mem = malloc(sizeof(memoire));
    mem = initialisation_mem(dico);

    bool trouve = false;
    int nb_essais = 1;
    while (!trouve && nb_essais<15){
        printf("Essai numéro %d : ", nb_essais);
        trouve = essai(objectif, dico, mem, (nb_essais-k)<=0, true);
        nb_essais++;
    }
}


void statistiques(char** dico, int k){
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

        mem = initialisation_mem(dico);
        
        bool trouve = false;
        int nb_essais = 1;
        while (!trouve && nb_essais<30){
            trouve = essai(objectif, dico, mem, (nb_essais-k)<=0, false);
            nb_essais++;
        }
        if (trouve){
            stat[nb_essais - 1] ++;
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




int main(){
    srand(time(NULL));

    //on récupère la liste de tous les mots
    char** dico = malloc(sizeof(char*)*NB_MOTS);
    dico = remplir_dictionnaire();

    /*
    for (int i=0; i<100; i++){
        printf("%d %s\n", i, dico[i]);
    }
    */

    //partie(dico, 3);
    statistiques(dico,3);
    

}