#include <stdio.h>
#include <stdlib.h>
#include "jeu.h"
#include "outils.h"
#include "data.h"
#include "bot.h"
#include "../tickettorideapi/ticketToRide.h"

extern int DEBUG_LEVEL;

// Main function for simulating one or more Ticket to Ride games
int main(){
    GameData Gdata;                   // General game configuration and state
    BoardState board;                // Current state of the visible board (cards, etc.)
    MoveData Mdata;                  // Structure used to send the bot's move
    MoveResult Mresult;             // Structure used to receive result of a move
    partie MyBot;                    // Data structure that holds information about our bot
    route routes[80];               // List of all routes claimed by the opponent
    route routes_dispos[80];        // List of all available routes in the game

    DEBUG_LEVEL = MESSAGE;          // Debug level for logging

    int wins = 0;               // Wins counter
    int nbGames = 0;            // Number of games played

    // Loop to simulate multiple games
    // while(1){
    for (int k=0; k<1; k++){        // Replace '1' with desired number of games
        int state = 1;              // Track game state (1 = in progress, 0 = over)

        // Connect to CGS server
        int connect = connectToCGS("82.29.170.160", 15001, "Jacques_ARNAULD");

        // Set game mode
        sendGameSettings("TRAINING NICE_BOT", &Gdata);

        printf("Connect : %d\n", connect);
        printf("Game name : %s\n", Gdata.gameName);

        printBoard();                              // Display initial map
        initPartie(&MyBot, &Gdata);                 // Initialize bot data structures
        initRoutesFromTrackData(Gdata, routes_dispos); // Parse route data from GameData

        // If the bot starts first, receive move from opponent (possibly twice)
        if (Gdata.starter == 1){
            getMove(&Mdata, &Mresult);
            if (Mresult.replay == 1){
                getMove(&Mdata, &Mresult);
            }
        }

        getBoardState(&board);             // Read board state
        printBoard();                      // Display it
        cardOnTheBoard(&board);            // Display available face-up cards
        chooseObjectivesBot2(&Mresult, &Mdata, &MyBot, &Gdata, routes_dispos); // Choose starting objectives

        // If bot plays second, wait for opponent’s move
        if (Gdata.starter == 0){
            getMove(&Mdata, &Mresult);
            if (Mresult.replay == 1){
                getMove(&Mdata, &Mresult);
            }
        }

        // First turn replay (if bot started and claimed a route)
        if (Gdata.starter == 1){
            getMove(&Mdata, &Mresult);
            if (Mresult.replay == 1){
                getMove(&Mdata, &Mresult);
            }
            if (Mdata.action == CLAIM_ROUTE){
                // Record opponent's claimed route
                routes[MyBot.nbTracks_opp].city1 = Mdata.claimRoute.from;
                routes[MyBot.nbTracks_opp].city2 = Mdata.claimRoute.to;
                routes[MyBot.nbTracks_opp].color1 = Mdata.claimRoute.color;
                routes[MyBot.nbTracks_opp].owner = 1;
                MyBot.nbTracks_opp++;
            }
        }

        // Game loop: repeat until win or loss
        while (!(Mresult.state == WINNING_MOVE || Mresult.state == LOSING_MOVE)) {
            majRoutesDispos(&MyBot, routes, routes_dispos); // Update available routes after each turn
            getBoardState(&board);
            printBoard();
            cardOnTheBoard(&board);

            // Bot plays its turn (either claim, draw cards, or choose objective)
            playBotTurn(&Mresult, &Mdata, &Gdata, &MyBot, routes_dispos);

            if (MyBot.state == 1){
                // Fallback: attempt to claim a long or strategic route
                claimer(&Mresult, &Mdata, &Gdata, &MyBot, routes_dispos);
            }

            // Track win/loss state (make sure counted only once)
            if (Mresult.state == WINNING_MOVE && state == 1){
                wins++;
                nbGames++;
                state = 0;
            } else if (Mresult.state == LOSING_MOVE && state == 1){
                nbGames++;
                state = 0;
            }

            // If game is not yet over, get opponent's move
            if (!(Mresult.state == WINNING_MOVE || Mresult.state == LOSING_MOVE)){
                getMove(&Mdata, &Mresult);
                if (Mresult.replay == 1){
                    getMove(&Mdata, &Mresult);
                }
            }

            // Track win/loss state again (after opponent's move)
            if (Mresult.state == WINNING_MOVE && state == 1){
                nbGames++;
                state = 0;
            } else if (Mresult.state == LOSING_MOVE && state == 1){
                nbGames++;
                wins++;
                state = 0;
            }

            // Save opponent’s claimed route
            if (Mdata.action == CLAIM_ROUTE){
                routes[MyBot.nbTracks_opp].city1 = Mdata.claimRoute.from;
                routes[MyBot.nbTracks_opp].city2 = Mdata.claimRoute.to;
                routes[MyBot.nbTracks_opp].color1 = Mdata.claimRoute.color;
                routes[MyBot.nbTracks_opp].owner = 1;
                MyBot.nbTracks_opp++;
            }

            // If opponent chose objectives, record them
            if (Mdata.action == CHOOSE_OBJECTIVES){
                for (int i = 0; i < 3; i++){
                    if (Mdata.chooseObjectives[i]){
                        MyBot.tab_obj_opp[MyBot.nb_obj_opp].score = Mresult.objectives[i].score;
                        MyBot.tab_obj_opp[MyBot.nb_obj_opp].city1 = Mresult.objectives[i].from;
                        MyBot.tab_obj_opp[MyBot.nb_obj_opp].city2 = Mresult.objectives[i].to;
                        MyBot.nb_obj_opp++;
                    }
                }
            }
        }

        // Game is over, disconnect from server
        quitGame();

        // Display final result for this game
        printf("Number of winning game : %d\n", wins);
        printf("Number of losing game : %d\n", nbGames - wins);
    }

    // Final summary
    printf("Final score [W,L] : [%d, %d]\n", wins, nbGames - wins);
    printf("Winning percentage : %.2f%%\n", (float)wins / nbGames * 100);

    return 0;
}
