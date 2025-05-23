#ifndef BOT_H
#define BOT_H

#include "data.h"
#include "../tickettorideapi/ticketToRide.h"

void playBotTurn(MoveResult Mresult, MoveData Mdata, GameData Gdata, partie MyBot, route routes[50]);
void chooseObjectivesBot(MoveResult Mresult, MoveData Mdata, partie MyBot);  // choisir les objectifs
void initRoutesFromTrackData(GameData Gdata, route routes[50]);

#endif
