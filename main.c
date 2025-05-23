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
    route routes[50];
    DEBUG_LEVEL = MESSAGE;
    int connect = connectToCGS("82.29.170.160", 15001, "Jack");

    sendGameSettings("TRAINING NICE_BOT timeout=1000 start=0 map=small", &Gdata);
    
    printf("Connect : %d\n", connect);
    printf("Game name : %s\n", Gdata.gameName);

    printBoard();
    initPartie(MyBot, Gdata);

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
    chooseObjectivesBot(Mresult, Mdata, MyBot);               
    getMove(&Mdata, &Mresult);  // Attendre le coup de l'adversaire
    getMove(&Mdata, &Mresult);


    while (!((Mresult.state == WINNING_MOVE) || (Mresult.state == LOSING_MOVE))) {
        initRoutesFromTrackData(Gdata, routes);
        getBoardState(&board);      // Récupérer l'état du plateau
        printBoard();
        cardOnTheBoard(&board);     // Afficher les cartes disponibles
        // playTurn();                 // Laisser le joueur jouer
        playBotTurn(Mresult, Mdata, Gdata, MyBot, routes);
        getMove(&Mdata, &Mresult);  // Attendre le coup de l'adversaire
        if (Mresult.replay == 1){
            getMove(&Mdata, &Mresult);   // Si l'adversaire rejoue
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