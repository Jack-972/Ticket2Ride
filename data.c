#include <stdio.h>
#include <stdlib.h>
#include "../tickettorideapi/ticketToRide.h"

// Define a structure for a route between two cities
typedef struct route_ {
    unsigned int city1;      // First city of the route (corresponds to 'from' in ClaimRouteMove)
    unsigned int city2;      // Second city of the route (corresponds to 'to')
    unsigned int length;     // Number of wagons needed to claim the route
    CardColor color1;        // First color of the route
    CardColor color2;        // Second color (NONE if the route is monochrome)
    int owner;               // Route ownership: -1 = free, 0 = bot, 1 = opponent
} route;

// Define a structure for an objective
typedef struct obj_ {
    unsigned int city1;      // Starting city (corresponds to 'from' in Objective)
    unsigned int city2;      // Destination city (corresponds to 'to')
    unsigned int score;      // Points awarded if the objective is completed
    int done;                // 1 if the objective is completed, 0 otherwise
    int index;               // Index used for identification or sorting
    int length;              // Estimated number of wagons required to fulfill the objective
} obj;

// Define a structure for maintaining the game state
typedef struct partie_ {
    int player;              // Current player ID (0 = bot, 1 = opponent)
    int cardByColor[10];     // Number of cards held by color (CardColor enum index)
    obj tab_obj[10];         // Array of the bot's objectives
    obj tab_obj_opp[10];     // Array of the opponent's objectives
    int nb_obj;              // Number of objectives held by the bot
    int nb_obj_opp;          // Number of objectives held by the opponent
    CardColor cardToPick[5]; // Cards visible on the board (draw pile)
    int wagons;              // Remaining wagons for the bot
    int wagons_opp;          // Remaining wagons for the opponent
    int nbTracks_tot;        // Total number of routes in the game
    int nbTracks_me;         // Number of routes claimed by the bot
    int nbTracks_opp;        // Number of routes claimed by the opponent
    int score;               // Score of the bot
    int score_opp;           // Score of the opponent
    int nbCards;             // Total number of cards held by the bot
    int state;               // Current state (custom use, e.g. 0 = playing, 1 = waiting)
} partie;

// Initializes the game state for the bot at the beginning of a game
void initPartie(partie* MyBot, GameData Gdata){
    MyBot->player = 0;
    MyBot->nb_obj = 0;
    MyBot->nb_obj_opp = 0;
    MyBot->wagons = 45;         // Initial number of wagons
    MyBot->wagons_opp = 45;
    MyBot->nbTracks_tot = Gdata.nbTracks;
    MyBot->nbTracks_me = 0;
    MyBot->nbTracks_opp = 0;
    MyBot->state = 0;
    MyBot->nbCards = 4;         // Bot starts with 4 cards
    MyBot->score = 0;
    MyBot->score_opp = 0;

    // Initialize all card colors to 0
    for (int i = 0; i < 10; i++){
        MyBot->cardByColor[i] = 0;
    }

    // Count the initial 4 cards dealt to the bot
    for (int i = 0; i < 4; i++){
        MyBot->cardByColor[Gdata.cards[i]] += 1;
    }

    // Mark all objectives as incomplete
    for (int i = 0; i < 10; i++){
        MyBot->tab_obj[i].done = 0;
    }
}

// Calculates score based on the length of a claimed route
int calcul(int a){
    switch (a)
    {
    case 1: return 1;
    case 2: return 2;
    case 3: return 4;
    case 4: return 7;
    case 5: return 10;
    case 6: return 15;
    default: return 0;
    }
}

// Updates the routes_dispos array based on claimed routes
void majRoutesDispos(partie* MyBot, route routes[80], route routes_dispos[80]) {
    for (int i = 0; i < MyBot->nbTracks_tot; i++) {

        if (routes_dispos[i].owner != -1) {
            continue; // Route already claimed, skip it
        }

        // Check if the opponent claimed this route
        for (int j = 0; j < MyBot->nbTracks_opp; j++) {
            if ((routes[j].city1 == routes_dispos[i].city1 && routes[j].city2 == routes_dispos[i].city2) ||
                (routes[j].city1 == routes_dispos[i].city2 && routes[j].city2 == routes_dispos[i].city1)) {

                // If it wasn't already marked, update ownership and opponent's state
                if (routes_dispos[i].owner != 1){
                    routes_dispos[i].owner = 1;
                    MyBot->wagons_opp -= routes_dispos[i].length;
                    MyBot->score_opp += calcul(routes_dispos[i].length);
                    printf("score opp : %d\n", MyBot->score_opp);
                    break;
                }
            }
        }

        // Check if the bot claimed this route
        for (int j = 0; j < MyBot->nbTracks_me; j++) {
            if ((routes[j].city1 == routes_dispos[i].city1 && routes[j].city2 == routes_dispos[i].city2) ||
                (routes[j].city1 == routes_dispos[i].city2 && routes[j].city2 == routes_dispos[i].city1)) {

                routes_dispos[i].owner = 0; // Mark as owned by the bot
                break;
            }
        }
    }
}
