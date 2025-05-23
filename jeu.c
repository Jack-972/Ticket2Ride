#include <stdio.h>
#include <stdlib.h>
#include "outils.h"
#include "../tickettorideapi/ticketToRide.h"

extern int DEBUG_LEVEL;

void startObj(){
    int choix = 3;
    MoveData Mdata;
    MoveResult Mresult;
    printf("%d", choix);
    int obj1, obj2, obj3;
    Mdata.action = DRAW_OBJECTIVES;  // Piocher des objectifs
    sendMove(&Mdata, &Mresult);
    Mdata.action = CHOOSE_OBJECTIVES;
    printObjectives(&Mresult);
    printf("Choose your objectives :");
    printf("\nObjective 1 (tape 1 if you want it, else 0) : ");
    scanf("%d", &obj1);
    printf("Objective 2 (tape 1 if you want it, else 0) : ");
    scanf("%d", &obj2);
    printf("Objective 3 (tape 1 if you want it, else 0) : ");
    scanf("%d", &obj3);
    Mdata.chooseObjectives[0] = obj1;
    Mdata.chooseObjectives[1] = obj2;
    Mdata.chooseObjectives[2] = obj3;
    sendMove(&Mdata, &Mresult);
}

void playTurn(MoveResult Mresult, MoveData Mdata){
    int choix = 0;
    
    printf("\nYour turn :\n");
    printf("1. Draw a card\n");
    printf("2. Claim a road\n");
    printf("3. Draw objectives\n");
    printf("Your choice : ");
    scanf("%d", &choix);
    printf("%d", choix);
    
    if (choix == 1) {           // Action : piocher une carte
        BoardState board;
        getBoardState(&board);
        cardOnTheBoard(&board);
        int choixCarte;
        int coups = 0;
        while (coups < 2){
            printf("Choose a card on the coard (0-4) or hiden (5) : ");
            scanf("%d", &choixCarte);
            if (choixCarte < 5 && choixCarte >= 0){
                Mdata.action = DRAW_CARD;
                Mdata.drawCard = board.card[choixCarte];
                if (board.card[choixCarte] == LOCOMOTIVE){
                    coups += 2;
                }
                else{
                    coups += 1;
                }
                sendMove(&Mdata, &Mresult);
            }
            else if (choixCarte == 5){
                Mdata.action = DRAW_BLIND_CARD;     // Action : piocher une carte cachée
                coups += 1;
                sendMove(&Mdata, &Mresult);
            }
            else{
                printf("Error : choose a number between 0 and 5\n");
            }
        } 
    }
    else if (choix == 2){
        int city1, city2, wagons;
        int color;
        Mdata.action = CLAIM_ROUTE;
        printf("City 1 : ");
        scanf("%d", &city1);
        printf("City 2 : ");
        scanf("%d", &city2);
        printf("Number of locomotives : ");
        scanf("%d", &wagons);
        printf("Color : ");
        scanf("%d", &color);
        Mdata.claimRoute.from = city1;
        Mdata.claimRoute.to = city2;
        Mdata.claimRoute.nbLocomotives = wagons;
        Mdata.claimRoute.color = color;
        sendMove(&Mdata, &Mresult);
    }
    else if (choix == 3) {
        int obj1, obj2, obj3;
        Mdata.action = DRAW_OBJECTIVES;  // Piocher des objectifs
        sendMove(&Mdata, &Mresult);
        Mdata.action = CHOOSE_OBJECTIVES;
        printObjectives(&Mresult);
        printf("Choose your objectives :");
        printf("\nObjective 1 (tape 1 if you want it, else 0) : ");
        scanf("%d", &obj1);
        printf("Objective 2 (tape 1 if you want it, else 0) : ");
        scanf("%d", &obj2);
        printf("Objective 3 (tape 1 if you want it, else 0) : ");
        scanf("%d", &obj3);
        Mdata.chooseObjectives[0] = obj1;
        Mdata.chooseObjectives[1] = obj2;
        Mdata.chooseObjectives[2] = obj3;
        sendMove(&Mdata, &Mresult);
    }
    else {
        printf("Unrecognized action ! Choose a number between 1 and 3 ! \n");
        return;
    }


    printf("Successfully delivered !\n");
    
}