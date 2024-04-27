/* Wordle Heuristique V1
On parcourt l'entièreté des mots et on trouve celui qui raporte le plus de points */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int NB_REPONSES = 2315;
int NB_MOTS = 14855;
int L_MOT = 5;

typedef struct Coup{
    char* mot;
    int sc;
}coup;

typedef struct Liste{
    char lettre;
    struct Liste* suiv;
}lst;

typedef struct Memoire{
    char* connus;
    lst* a_utiliser;
    bool** a_eviter; 
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

bool dans(char lettre, char* mot){
    //renvoie true si la lettre est présente dans le mot
    for (int i=0; i<5; i++){
        if (lettre == mot[i]){
            return true;
        }
    }
    return false;
}

int indice(int i){
    //pour compenser le fait que ascii(a) = 97
    return i - 97;
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

lst* ajouter_a_utiliser(char lettre, lst* liste){
    //ajoute la lettre à la liste des lettres à utiliser
    bool dedans = false;
    for (lst* l=liste; l!=NULL; l=l->suiv){
        if (lettre == l->lettre){
            dedans = true;
        }
    }
    if (dedans){ //si la lettres est deja dans la liste on ne fait rien 
        return liste;
    }
    //sinon on la rajoute au début 
    lst* new = malloc(sizeof(lst));
    new->lettre  = lettre;
    new->suiv = liste;
    return new;
}

bool essai(char* objectif, char** dico, memoire* mem){
    //fonction qui simule un essai et qui renvoie un bool
    // true -> le mot objectif est trouvé
    // false -> l'essai ne correspond pas à l'objectif
    
    
    //tout d'abord on parcourt tous les mots à la recherche du meilleur
    /*
    Heuristique : 
        - +15 pts si on met une lettre à son emplacement connu (vert)
        - -5 pts si on met une lettre à un endroit qui ne peut pas être son emplacement
        - -5 pts par lettre, qu'on sait être dans le mot, non utilisée
        - +2 pts par lettre différente du mot
    */
    coup* meilleur = malloc(sizeof(coup));
    meilleur->sc = 0;

    for (int i=0; i<NB_REPONSES; i++){
        int score = 0;
        for (int j=0; j<5; j++){
            if (dico[i][j] == mem->connus[j]){
                score = score + 15;
            }
            else if (mem->a_eviter[indice(dico[i][j])][j]){
                score = score - 5;
            }

        }

        for (lst* l = mem->a_utiliser; l != NULL; l = l->suiv){
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

    //printf("%s\n", meilleur->mot);

    if (egal(meilleur->mot, objectif)){
        return true; //on a trouvé le mot recherché
    }

    //on met à jour la mémoire
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

    return false; //on n'a pas trouvé le mot recherché

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
    
    printf("\nObjectif : %s\n", objectif);

    //on initialise la mémoire
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

    bool trouve = false;
    int nb_essais = 1;
    while (!trouve && nb_essais<15){
        printf("Essai numéro %d : ", nb_essais);
        trouve = essai(objectif, dico, mem);
        nb_essais++;
    }
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
    mem->connus = malloc(sizeof(char)*L_MOT);
    mem->a_eviter = malloc(sizeof(bool*)*26);

    end_pre = clock();

    start = clock();
    for (int mot=0; mot<NB_REPONSES; mot++){
        //printf("%d\n", mot);
        objectif = dico[mot];

        for(int i=0; i<L_MOT; i++){
            mem->connus[i] = '*';
        }
        free(mem->a_utiliser);
        mem->a_utiliser = NULL;
        for (int i=0; i<26; i++){
            bool* ligne = malloc(sizeof(bool)*L_MOT);
            for (int j=0; j<L_MOT; j++){
                ligne[j] = false;
            }
            mem->a_eviter[i] = ligne;
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
        if(nb_essais > 11){
            printf("%d essais, %s\n", nb_essais -1, objectif);
        }
        /*
        else {
            stat[7] ++;
        }
        */
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

    //partie(dico);
    statistiques(dico);
    

}