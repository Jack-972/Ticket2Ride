#ifndef DATA_H
#define DATA_H

#include "../tickettorideapi/ticketToRide.h"

typedef struct route_ {
    unsigned int city1;
    unsigned int city2;
    unsigned int length;
    CardColor color1;
    CardColor color2;
    int owner;
} route;

typedef struct obj_ {
    unsigned int city1;
    unsigned int city2;
    unsigned int score;
    int done;
} obj;

typedef struct partie_ {
    int player;
    int cardByColor[10];
    obj tab_obj[10];
    int nb_obj;
    CardColor cardToPick[5];
    int wagons, wagons_opp;
    int nbTracks_tot, nbTracks_me, nbTracks_opp;
    int nbCards;
    int state;
} partie;

void initPartie(partie* MyBot, GameData Gdata);
void majRoutesDispos(partie* MyBot, route routes[80], route routes_dispos[80]);


#endif
