# Wordle
Ce dossier contient l'ensemble du code que j'ai écrit en C dans le cadre de mon TIPE 2024.
Mon projet était de mettre au point différents algorithmes capables de jouer à Wordle (https://www.nytimes.com/games/wordle/index.html) un jeu dont l'objectif 
est de trouver un mot mystère en six essais ou moins.

Une explication détaillée de ce projet, mon rapport pour le concours des ENS, sera publiée à la fin des oraux.

Il y a 7 algorithmes différents :
- Heuristique 1, 2
- Elimine 1, 2, 3
- Hybride
- Aléatoire

J'ai aussi inclus dans ce dossier les dictionnaires sur lesquels j'ai effectué mes tests.

Tous ces algorithmes ont deux méthodes de fonctionnement :
- Tester l'algorithme sur un mot mystère, choisi aléatoirement ou donné par l'utilisateur : fonction partie()
- Tester l'algorithme sur l'entièreté des mots mystères du dictionnaire : fonction statistiques()
