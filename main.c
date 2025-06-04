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

    int Victoire = 0;
    int nbParties = 0;
    int state = 1;

    while (1){
    // for (int k=0; k<1; k++){

        int connect = connectToCGS("82.29.170.160", 15001, "Jack2");

        sendGameSettings("", &Gdata);
        // sendGameSettings("TRAINING NICE_BOT", &Gdata);

        
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
        // chooseObjectivesBot(&Mresult, &Mdata, &MyBot);
        chooseObjectivesBot2(&Mresult, &Mdata, &MyBot, &Gdata, routes_dispos);
                
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
            if(Mresult.state == WINNING_MOVE && state == 1){
                Victoire++;
                nbParties++;
                state = 0;
            }
            else if(Mresult.state == LOSING_MOVE && state == 1){
                nbParties++;
                state = 0;
            }
            if (!((Mresult.state == WINNING_MOVE) || (Mresult.state == LOSING_MOVE))){
                getMove(&Mdata, &Mresult);  // Attendre le coup de l'adversaire
                if (Mresult.replay == 1){
                    getMove(&Mdata, &Mresult);   // Si l'adversaire rejoue
                }
            }
            if(Mresult.state == WINNING_MOVE && state == 1){
                nbParties++;
                state = 0;
            }
            else if(Mresult.state == LOSING_MOVE && state == 1){
                nbParties++;
                Victoire++;
                state = 0;
            }
            if (Mdata.action == CLAIM_ROUTE){
                routes[MyBot.nbTracks_opp].city1 = Mdata.claimRoute.from;
                routes[MyBot.nbTracks_opp].city2 = Mdata.claimRoute.to;
                routes[MyBot.nbTracks_opp].color1 = Mdata.claimRoute.color;
                routes[MyBot.nbTracks_opp].owner = 1;
                MyBot.nbTracks_opp ++;
            }
            if (Mdata.action == CHOOSE_OBJECTIVES){
                for (int i=0; i<3; i++){
                    if (Mdata.chooseObjectives[i]){
                        MyBot.tab_obj_opp[MyBot.nb_obj_opp].score = Mresult.objectives[i].score;
                        MyBot.tab_obj_opp[MyBot.nb_obj_opp].city1 = Mresult.objectives[i].from;
                        MyBot.tab_obj_opp[MyBot.nb_obj_opp].city2 = Mresult.objectives[i].to;
                        MyBot.nb_obj_opp++;
                    }
                }
            }
            // if (((Mresult.state == WINNING_MOVE) || (Mresult.state == LOSING_MOVE))){
            //     for (int i=0; i<MyBot.nb_obj; i++){
            //         if (MyBot.tab_obj[i].done){
            //             MyBot.score += MyBot.tab_obj[i].score;
            //         }
            //         else{
            //             MyBot.score -= MyBot.tab_obj[i].score;
            //         }
            //     }
            //     for (int i = 0; i < MyBot.nb_obj_opp; i++) {
            //         MyBot.tab_obj_opp[i].done = objectifAtteint(MyBot.tab_obj_opp[i], routes_dispos, Gdata.nbTracks, Gdata.nbCities, 1);
            //         if (MyBot.tab_obj_opp[i].done){
            //             MyBot.score_opp += MyBot.tab_obj_opp[i].score;
            //         }
            //         else{
            //             MyBot.score_opp -= MyBot.tab_obj_opp[i].score;
            //         }
            //     }
            //     printf("Mon score : %d\n", MyBot.score);
            //     printf("Score adversaire : %d\n", MyBot.score_opp);
            //     if (MyBot.score > MyBot.score_opp){
            //         Victoire++;
            //         printf("Victoire !\n");
            //     }
            //     else if (MyBot.score == MyBot.score_opp){
            //         Victoire += 0.5;
            //         printf("Match nul\n");
            //     }
            //     else{
            //         printf("Défaite\n");
            //     }
            //     nbParties++;
            // 
            // }

        }
        quitGame();
        // printf("Nombre de victoires : %d\n", Victoire);
        // printf("Nombre de victoires : %d\n", nbParties - Victoire);

    }


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