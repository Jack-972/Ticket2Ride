#include <stdio.h>
#include <stdlib.h>
#include "outils.h"
#include "../tickettorideapi/ticketToRide.h"

extern int DEBUG_LEVEL;

// Function that allows the player to draw and choose objectives manually
void startObj() {
    int choix = 3;  // Always draw 3 objectives (standard)
    MoveData Mdata;
    MoveResult Mresult;

    printf("%d", choix);

    int obj1, obj2, obj3;

    // Request the objectives from the server
    Mdata.action = DRAW_OBJECTIVES;
    sendMove(&Mdata, &Mresult);

    // Prepare to choose which objectives to keep
    Mdata.action = CHOOSE_OBJECTIVES;
    printObjectives(&Mresult);

    // Ask the user to choose which objectives to keep (1 = yes, 0 = no)
    printf("Choose your objectives :");
    printf("\nObjective 1 (type 1 to keep, 0 otherwise): ");
    scanf("%d", &obj1);
    printf("Objective 2 (type 1 to keep, 0 otherwise): ");
    scanf("%d", &obj2);
    printf("Objective 3 (type 1 to keep, 0 otherwise): ");
    scanf("%d", &obj3);

    // Store the choices in the MoveData structure
    Mdata.chooseObjectives[0] = obj1;
    Mdata.chooseObjectives[1] = obj2;
    Mdata.chooseObjectives[2] = obj3;

    // Send the final choice to the server
    sendMove(&Mdata, &Mresult);
}

// Function to play a manual turn (for a human player)
void playTurn(MoveResult Mresult, MoveData Mdata) {
    int choix = 0;

    // Display menu of actions
    printf("\nYour turn :\n");
    printf("1. Draw a card\n");
    printf("2. Claim a route\n");
    printf("3. Draw objectives\n");
    printf("Your choice : ");
    scanf("%d", &choix);
    printf("%d", choix);

    // Action 1: Draw cards
    if (choix == 1) {
        BoardState board;
        getBoardState(&board);    // Get current state of visible cards
        cardOnTheBoard(&board);   // Display them

        int choixCarte;
        int coups = 0;            // Number of draws this turn (max 2)

        while (coups < 2) {
            printf("Choose a card on the board (0-4) or hidden (5) : ");
            scanf("%d", &choixCarte);

            if (choixCarte >= 0 && choixCarte < 5) {
                Mdata.action = DRAW_CARD;
                Mdata.drawCard = board.card[choixCarte];

                // If it's a visible locomotive, it ends your turn
                if (board.card[choixCarte] == LOCOMOTIVE) {
                    coups += 2;
                } else {
                    coups += 1;
                }

                sendMove(&Mdata, &Mresult);
            } else if (choixCarte == 5) {
                Mdata.action = DRAW_BLIND_CARD;  // Draw a card blindly
                coups += 1;
                sendMove(&Mdata, &Mresult);
            } else {
                printf("Error : choose a number between 0 and 5\n");
            }
        }

    }
    // Action 2: Claim a route
    else if (choix == 2) {
        int city1, city2, wagons, color;
        Mdata.action = CLAIM_ROUTE;

        printf("City 1 : ");
        scanf("%d", &city1);
        printf("City 2 : ");
        scanf("%d", &city2);
        printf("Number of locomotives : ");
        scanf("%d", &wagons);
        printf("Color : ");
        scanf("%d", &color);

        // Set up the claimRoute structure
        Mdata.claimRoute.from = city1;
        Mdata.claimRoute.to = city2;
        Mdata.claimRoute.nbLocomotives = wagons;
        Mdata.claimRoute.color = color;

        // Send claim to the server
        sendMove(&Mdata, &Mresult);
    }
    // Action 3: Draw objectives
    else if (choix == 3) {
        int obj1, obj2, obj3;
        Mdata.action = DRAW_OBJECTIVES;
        sendMove(&Mdata, &Mresult);

        Mdata.action = CHOOSE_OBJECTIVES;
        printObjectives(&Mresult);

        printf("Choose your objectives :");
        printf("\nObjective 1 (type 1 to keep, 0 otherwise): ");
        scanf("%d", &obj1);
        printf("Objective 2 (type 1 to keep, 0 otherwise): ");
        scanf("%d", &obj2);
        printf("Objective 3 (type 1 to keep, 0 otherwise): ");
        scanf("%d", &obj3);

        Mdata.chooseObjectives[0] = obj1;
        Mdata.chooseObjectives[1] = obj2;
        Mdata.chooseObjectives[2] = obj3;

        sendMove(&Mdata, &Mresult);
    }
    // Invalid action
    else {
        printf("Unrecognized action! Choose a number between 1 and 3!\n");
        return;
    }

    printf("Successfully delivered!\n");  // Acknowledge turn completion
}
