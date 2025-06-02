#include <stdio.h>
#include <stdlib.h>
#include "jeu.h"
#include "outils.h"
#include "data.h"
#include "bot.h"
#include "../tickettorideapi/ticketToRide.h"

extern int DEBUG_LEVEL;


int main(){
    GameData Gdata;
    BoardState board;
    MoveData Mdata;
    MoveResult Mresult;
    partie MyBot;
    route routes[80];
    route routes_dispos[80];
    DEBUG_LEVEL = MESSAGE;
    int connect = connectToCGS("82.29.170.160", 15001, "Jack");

    // sendGameSettings("", &Gdata);
    sendGameSettings("TRAINING PLAY_RANDOM", &Gdata);

    
    printf("Connect : %d\n", connect);
    printf("Game name : %s\n", Gdata.gameName);

    printBoard();
    initPartie(&MyBot, Gdata);
    initRoutesFromTrackData(Gdata, routes_dispos);

    if (Gdata.starter == 1){
        getMove(&Mdata, &Mresult);
        if (Mresult.replay == 1){
            getMove(&Mdata, &Mresult);   // Si l'adversaire rejoue
        }
    }

    getBoardState(&board);      // Récupérer l'état du plateau
    printBoard();
    cardOnTheBoard(&board);     // Afficher les cartes disponibles
    // startObj();
    chooseObjectivesBot(&Mresult, &Mdata, &MyBot);
               
    if (Gdata.starter == 0){
        getMove(&Mdata, &Mresult);
        if (Mresult.replay == 1){
            getMove(&Mdata, &Mresult);   // Si l'adversaire rejoue
        }
    }

    if (Gdata.starter == 1){
        getMove(&Mdata, &Mresult);
        if (Mresult.replay == 1){
            getMove(&Mdata, &Mresult);   // Si l'adversaire rejoue
        }
        if (Mdata.action == CLAIM_ROUTE){
            routes[MyBot.nbTracks_opp].city1 = Mdata.claimRoute.from;
            routes[MyBot.nbTracks_opp].city2 = Mdata.claimRoute.to;
            routes[MyBot.nbTracks_opp].color1 = Mdata.claimRoute.color;
            routes[MyBot.nbTracks_opp].owner = 1;
            MyBot.nbTracks_opp ++;
        }
    }

    while (!((Mresult.state == WINNING_MOVE) || (Mresult.state == LOSING_MOVE))) {
        majRoutesDispos(&MyBot, routes, routes_dispos);
        getBoardState(&board);      // Récupérer l'état du plateau
        printBoard();
        cardOnTheBoard(&board);     // Afficher les cartes disponibles
        // playTurn(Mresult, Mdata);                 // Laisser le joueur jouer
        playBotTurn(&Mresult, &Mdata, &Gdata, &MyBot, routes_dispos);
        if (MyBot.state == 1){
            claimer(&Mresult, &Mdata, &Gdata, &MyBot, routes_dispos);
        }
        getMove(&Mdata, &Mresult);  // Attendre le coup de l'adversaire
        if (Mresult.replay == 1){
            getMove(&Mdata, &Mresult);   // Si l'adversaire rejoue
        }
        if (Mdata.action == CLAIM_ROUTE){
            routes[MyBot.nbTracks_opp].city1 = Mdata.claimRoute.from;
            routes[MyBot.nbTracks_opp].city2 = Mdata.claimRoute.to;
            routes[MyBot.nbTracks_opp].color1 = Mdata.claimRoute.color;
            routes[MyBot.nbTracks_opp].owner = 1;
            MyBot.nbTracks_opp ++;
        }
    }
    quitGame();


    return 0;
}



/*

pré-requis pour avoir un bot :
1) programme capable de jouer manuellement (scanf)
2) structure de données
3) mettre à jour les données -> afficher les données
4) bot 1 (Random Player)

typedef struct route_{
    ville 1
    ville 2
    couleur 1
    couleur 2
    par qui elle est prise
}route;

typedef struct obj_{
    ville 1
    ville 2
    score
}obj

typedef struct partie_{
    qui joue
    nb de carte par couleur
    tableau d'obj + nb d'obj
    tableau de 5 cartes à piocher
    nb de wagons + nb de wagons adv
    etat
}partie

Conseils :
Choisir objectif entre Nord Ouest et Sud Ouest, ou Est et Ouest
Pas choisir obj dans l'Est

*/