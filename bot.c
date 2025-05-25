#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bot.h"
#include "data.h"
#include "outils.h"
#include "../tickettorideapi/ticketToRide.h"


extern int DEBUG_LEVEL;

void chooseObjectivesBot(MoveResult* Mresult, MoveData* Mdata, partie* MyBot) {

    Mdata->action = DRAW_OBJECTIVES;  // Piocher des objectifs
    sendMove(Mdata, Mresult);
    Mdata->action = CHOOSE_OBJECTIVES;

    // On garde au moins le 1er, et le plus court des 2 autres
    int minIndex = 1;
    if (Mresult->objectives[2].score < Mresult->objectives[1].score) {
        minIndex = 2;
    }

    Mdata->chooseObjectives[0] = 1;
    MyBot->tab_obj[MyBot->nb_obj].city1 = Mresult->objectives[0].from;
    MyBot->tab_obj[MyBot->nb_obj].city2 = Mresult->objectives[0].to;
    MyBot->tab_obj[MyBot->nb_obj].score = Mresult->objectives[0].score;
    printf("city1 : %d, city2 : %d, score : %d\n", MyBot->tab_obj[MyBot->nb_obj].city1, MyBot->tab_obj[MyBot->nb_obj].city2,MyBot->tab_obj[MyBot->nb_obj].score);
    MyBot->nb_obj += 1;

    if (minIndex == 1){
        Mdata->chooseObjectives[1] = (minIndex == 1);
        Mdata->chooseObjectives[2] = (minIndex == 2);
        MyBot->tab_obj[MyBot->nb_obj].city1 = Mresult->objectives[1].from;
        MyBot->tab_obj[MyBot->nb_obj].city2 = Mresult->objectives[1].to;
        MyBot->tab_obj[MyBot->nb_obj].score = Mresult->objectives[1].score;
        printf("city1 : %d, city2 : %d, score : %d\n", MyBot->tab_obj[MyBot->nb_obj].city1, MyBot->tab_obj[MyBot->nb_obj].city2,MyBot->tab_obj[MyBot->nb_obj].score);
        MyBot->nb_obj += 1;
    }
    else{
        Mdata->chooseObjectives[1] = (minIndex == 1);
        Mdata->chooseObjectives[2] = (minIndex == 2);
        MyBot->tab_obj[MyBot->nb_obj].city1 = Mresult->objectives[2].from;
        MyBot->tab_obj[MyBot->nb_obj].city2 = Mresult->objectives[2].to;
        MyBot->tab_obj[MyBot->nb_obj].score = Mresult->objectives[2].score;
        printf("city1 : %d, city2 : %d, score : %d\n", MyBot->tab_obj[MyBot->nb_obj].city1, MyBot->tab_obj[MyBot->nb_obj].city2,MyBot->tab_obj[MyBot->nb_obj].score);
        MyBot->nb_obj += 1;
    }

    sendMove(Mdata, Mresult);
}


void initRoutesFromTrackData(GameData Gdata, route routes_dispos[50]) {
    for (int i = 0; i < Gdata.nbTracks; i++) {
        int idx = i * 5;
        routes_dispos[i].city1 = Gdata.trackData[idx + 0];
        routes_dispos[i].city2 = Gdata.trackData[idx + 1];
        routes_dispos[i].length = Gdata.trackData[idx + 2];
        routes_dispos[i].color1 = Gdata.trackData[idx + 3];
        routes_dispos[i].color2 = Gdata.trackData[idx + 4];
        routes_dispos[i].owner = -1;
    }
}


void playBotTurn(MoveResult* Mresult, MoveData* Mdata, GameData* Gdata, partie* MyBot, route routes[50]) {
    int claimed = 0;
    printf("Etat des cartes : ");
    for (int i = 1; i <= 9; i++) {
        printf("%d:%d ", i, MyBot->cardByColor[i]);
    }
    printf(" | Wagons restants : %d\n", MyBot->wagons);
    printf(" Nombre routes bots : %d\n", MyBot->nbTracks_opp);
    printf(" Nombre routes dispos : %d\n", MyBot->nbTracks_tot);


    // On tente de poser une route sûre
    for (int i = 0; i < Gdata->nbTracks; i++) {
        if (routes[i].owner != -1) continue;  // Déjà prise

        int from = routes[i].city1;
        int to = routes[i].city2;
        CardColor color = routes[i].color1;
        
        int length = routes[i].length;

        int cards_in_color = MyBot->cardByColor[color];
        if (color == LOCOMOTIVE){
            cards_in_color = 0;
            for (int j=1; j <= 8; j++){
                if (MyBot->cardByColor[j] >= length - MyBot->cardByColor[LOCOMOTIVE]){
                    color = j;
                    cards_in_color = MyBot->cardByColor[j];
                    break;
                }
            }
        } 
        if (cards_in_color == 0) continue;  // aucune couleur suffisante trouvée
       
        int locomotives = MyBot->cardByColor[LOCOMOTIVE];
        int total = cards_in_color + locomotives;

        if (MyBot->wagons < length) continue;
        if (total < length) continue;

        // C'est safe, on joue
        Mdata->action = CLAIM_ROUTE;
        Mdata->claimRoute.from = from;
        Mdata->claimRoute.to = to;
        Mdata->claimRoute.color = color;
        Mdata->claimRoute.nbLocomotives = (locomotives >= (length - cards_in_color)) ? (length - cards_in_color) : locomotives;

        sendMove(Mdata, Mresult);

        // MAJ état du jeu local
        routes[i].owner = 0;
        MyBot->nbTracks_me ++;
        MyBot->wagons -= length;
        MyBot->cardByColor[color] -= (length - Mdata->claimRoute.nbLocomotives);
        MyBot->cardByColor[LOCOMOTIVE] -= Mdata->claimRoute.nbLocomotives;


        claimed = 1;
        printf("Tentative de claim route %d -> %d, couleur %d, longueur %d\n", from, to, color, length);
        printf("Cartes couleur: %d, locomotives: %d, total: %d, wagons: %d\n", cards_in_color, locomotives, total, MyBot->wagons);

        break;
    }

    if (!claimed) {
        // Sécurité : on pioche 2 cartes
        BoardState board;
        getBoardState(&board);

        int picked = 0;
        for (int i = 0; i < 5 && picked < 2; i++) {
            if (board.card[i] == LOCOMOTIVE && picked == 0) {
                Mdata->action = DRAW_CARD;
                Mdata->drawCard = LOCOMOTIVE;
                sendMove(Mdata, Mresult);
                MyBot->cardByColor[LOCOMOTIVE]++;
                picked += 2; // Fin du tour si on prend loco visible
            }
            else if (board.card[i] != LOCOMOTIVE) {
                Mdata->action = DRAW_CARD;
                Mdata->drawCard = board.card[i];
                sendMove(Mdata, Mresult);
                MyBot->cardByColor[board.card[i]]++;
                picked++;
            }
        }

        while (picked < 2) {
            Mdata->action = DRAW_BLIND_CARD;
            sendMove(Mdata, Mresult);
            MyBot->cardByColor[Mresult->card]++;
            picked++;
        }
    }
}
