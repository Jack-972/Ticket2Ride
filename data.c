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
    int done;
} obj;

typedef struct partie_ {
    int player;              // Joueur actuel (0 ou 1)
    int cardByColor[10];      // Nombre de cartes par couleur (suivant l'enum CardColor)
    obj tab_obj[10];         // Tableau des objectifs
    int nb_obj;              // Nombre d'objectifs possédés
    CardColor cardToPick[5]; // Cartes visibles à piocher (utilise l'enum CardColor)
    int wagons, wagons_opp;  // Nombre de wagons restants pour le joueur et l'adversaire
    int nbTracks_tot, nbTracks_me, nbTracks_opp;
    int state;               // État actuel du jeu 
} partie;

void initPartie(partie* MyBot, GameData Gdata){
    MyBot->player = 0;
    MyBot->nb_obj = 0;
    MyBot->wagons = 45;
    MyBot->wagons_opp = 45;
    MyBot->nbTracks_tot = Gdata.nbTracks;
    MyBot->nbTracks_me = 0;
    MyBot->nbTracks_opp = 0;
    MyBot->state = 0;
    for (int i=0; i < 10; i++){
        MyBot->cardByColor[i] = 0;
    }
    for (int i=0; i < 4; i++){
        MyBot->cardByColor[Gdata.cards[i]] += 1;
    }
    for (int i=0; i < 10; i++){
        MyBot->tab_obj[i].done = 0;
    }

}

void majRoutesDispos(partie* MyBot, route routes[80], route routes_dispos[80]) {
    for (int i = 0; i < MyBot->nbTracks_tot; i++) {
        if (routes_dispos[i].owner != -1) {
            continue; // route déjà prise, on ne fait rien
        }

        // Vérifie si elle est dans les routes de l’adversaire
        for (int j = 0; j < MyBot->nbTracks_opp; j++) {
            if ((routes[j].city1 == routes_dispos[i].city1 && routes[j].city2 == routes_dispos[i].city2) ||
                (routes[j].city1 == routes_dispos[i].city2 && routes[j].city2 == routes_dispos[i].city1)) {
                if (routes_dispos[i].owner != 1){
                    routes_dispos[i].owner = 1;
                    MyBot->wagons_opp -= routes_dispos[i].length;
                    break;
                }
            }
        }

        // Vérifie si elle est dans les routes du bot
        for (int j = 0; j < MyBot->nbTracks_me; j++) {
            if ((routes[j].city1 == routes_dispos[i].city1 && routes[j].city2 == routes_dispos[i].city2) ||
                (routes[j].city1 == routes_dispos[i].city2 && routes[j].city2 == routes_dispos[i].city1)) {
                routes_dispos[i].owner = 0;
                break;
            }
        }
    }
}
