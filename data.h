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
} obj;

typedef struct partie_ {
    int player;
    int cardByColor[10];
    obj tab_obj[10];
    int nb_obj;
    CardColor cardToPick[5];
    int wagons, wagons_opp;
    int state;
} partie;

void initPartie(partie MyBot, GameData Gdata);

#endif
