#include <stdio.h>
#include <stdlib.h>
#include "../tickettorideapi/ticketToRide.h"

typedef struct route_ {
    unsigned int city1;      // Première ville de la route (correspond à 'from' dans ClaimRouteMove)
    unsigned int city2;      // Deuxième ville de la route (correspond à 'to')
    unsigned int length;     // Nb de Wagons entre les 2 villes
    CardColor color1;        // Première couleur de la route
    CardColor color2;        // Deuxième couleur (0 si monochrome)
    int owner;               // Joueur qui a revendiqué la route (-1 si libre)
} route;

typedef struct obj_ {
    unsigned int city1;      // Ville de départ de l'objectif (correspond à 'from' dans Objective)
    unsigned int city2;      // Ville d'arrivée de l'objectif (correspond à 'to')
    unsigned int score;      // Score associé à l'objectif
} obj;

typedef struct partie_ {
    int player;              // Joueur actuel (0 ou 1)
    int cardByColor[10];      // Nombre de cartes par couleur (suivant l'enum CardColor)
    obj tab_obj[10];         // Tableau des objectifs
    int nb_obj;              // Nombre d'objectifs possédés
    CardColor cardToPick[5]; // Cartes visibles à piocher (utilise l'enum CardColor)
    int wagons, wagons_opp;  // Nombre de wagons restants pour le joueur et l'adversaire
    int state;               // État actuel du jeu (peut correspondre à Mresult.state)
} partie;

void initPartie(partie MyBot, GameData Gdata){
    MyBot.player = 0;
    MyBot.nb_obj = 0;
    MyBot.wagons = 45;
    MyBot.wagons_opp = 45;
    for (int i=0; i < 10; i++){
        MyBot.cardByColor[i] = 0;
    }
    for (int i=0; i < 4; i++){
        MyBot.cardByColor[Gdata.cards[i]] += 1;
    }

}
