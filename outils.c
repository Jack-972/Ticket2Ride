#include <stdio.h>
#include <stdlib.h>
#include "../tickettorideapi/ticketToRide.h"

// Prints the cards currently on the board
void cardOnTheBoard(BoardState *board){
    printf("Cards on the board are : ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", board->card[i]);
    }
    printf("\n");
}

// Prints the objectives available in the current move result
void printObjectives(MoveResult *Mresult){
    for (int i = 0; i < 3; i++) {
        printf("Objective %d : ", i);
        printf("City 1 : %d, City 2 : %d, Score : %d\n", Mresult->objectives[i].from, Mresult->objectives[i].to, Mresult->objectives[i].score);
    }
    printf("\n");
}