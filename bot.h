#ifndef BOT_H
#define BOT_H

#include "data.h"
#include "../tickettorideapi/ticketToRide.h"

void chooseObjectivesBot(MoveResult* Mresult, MoveData* Mdata, partie* MyBot); 
void chooseObjectivesBot2(MoveResult* Mresult, MoveData* Mdata, partie* MyBot, GameData* Gdata, route routes[80]);
void initRoutesFromTrackData(GameData Gdata, route routes[80]);
void claimer(MoveResult* Mresult, MoveData* Mdata, GameData* Gdata, partie* MyBot, route routes[80]);
void nbWagons(route route[80], GameData* Gdata, int G[36][36]);
void dijkstra(int src, route routes_dispos[80], GameData* Gdata, int D[36], int Prec[36]);
int distanceMini(int D[36], int visite[36], int N);
void afficherChemin(int src, int dest, int Prec[36]);
int dfs(int src, int dest, int visite[], int nbCities, route routes[], int nbRoutes, int player);
int objectifAtteint(obj objectif, route routes[], int nbRoutes, int nbCities, int player);
void playBotTurn(MoveResult* Mresult, MoveData* Mdata, GameData* Gdata, partie* MyBot, route routes[80]);

#endif
