#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bot.h"
#include "data.h"
#include "outils.h"
#include "../tickettorideapi/ticketToRide.h"


extern int DEBUG_LEVEL;


// Allows the bot to choose objectives based on game state
// Strategy: Select objectives that maximize score while considering feasibility
// - At the start, prioritize high-scoring objectives
// - In the mid-game, focus on objectives with shorter paths
// - Near the end, select objectives that can be completed quickly
void chooseObjectivesBot2(MoveResult* Mresult, MoveData* Mdata, partie* MyBot, GameData* Gdata, route routes[80]){

    // Request and receive 3 objectives
    Mdata->action = DRAW_OBJECTIVES;
    sendMove(Mdata, Mresult);
    Mdata->action = CHOOSE_OBJECTIVES;

    int D[36], Prec[36];  // Dijkstra arrays: distances and predecessors
    obj objs[3];          // Store data for the 3 objectives

    // Initialize objective info
    for (int i = 0; i < 3; i++) {
        objs[i].index = i;
        objs[i].score = Mresult->objectives[i].score;
        objs[i].city1 = Mresult->objectives[i].from;
        objs[i].city2 = Mresult->objectives[i].to;
        objs[i].done = 0;
        objs[i].length = 0;
    }

    // For each objective, compute the estimated distance to complete it
    for (int k = 0; k < 3; k++) {
        int src = objs[k].city1;
        int dest = objs[k].city2;

        dijkstra(src, routes, Gdata, D, Prec);  // Compute shortest paths
        afficherChemin(src, dest, Prec);        // Optional debug output

        int chemin[20];  // Path array (sequence of cities)
        int len = 0;
        int v = dest;
        while (v != src && v != -1) {
            chemin[len++] = v;
            v = Prec[v];
        }

        if (v != -1) {
            chemin[len++] = src;

            // Accumulate route lengths in the path if not yet claimed
            for (int i = len - 1; i > 0; i--) {
                unsigned int c1 = chemin[i];
                unsigned int c2 = chemin[i - 1];

                for (int j = 0; j < Gdata->nbTracks; j++) {
                    route r = routes[j];
                    if ((r.city1 == c1 && r.city2 == c2) || (r.city1 == c2 && r.city2 == c1)) {
                        if (r.owner != 0) {
                            objs[k].length += r.length;
                        }
                        break;
                    }
                }
            }
        } else {
            printf("Pas de chemin dispo\n"); // No valid path
        }
    }

    // Reset selection array
    for (int i = 0; i < 3; i++) {
        Mdata->chooseObjectives[i] = 0;
    }

    // Choose based on game phase
    if (MyBot->wagons == 45) {
        // Early game: pick top 2 scores
        if (objs[0].score > objs[1].score) {
            Mdata->chooseObjectives[objs[0].index] = 1;
            Mdata->chooseObjectives[(objs[1].score > objs[2].score) ? objs[1].index : objs[2].index] = 1;
        } else {
            Mdata->chooseObjectives[objs[1].index] = 1;
            Mdata->chooseObjectives[(objs[0].score > objs[2].score) ? objs[0].index : objs[2].index] = 1;
        }
    } else if (MyBot->wagons >= 14 && MyBot->wagons <= 45 && MyBot->wagons_opp > 18) {
        // Mid-game: pick 2 shortest paths
        if (objs[0].length > objs[1].length) {
            Mdata->chooseObjectives[objs[1].index] = 1;
            Mdata->chooseObjectives[(objs[0].length > objs[2].length) ? objs[2].index : objs[0].index] = 1;
        } else {
            Mdata->chooseObjectives[objs[0].index] = 1;
            Mdata->chooseObjectives[(objs[1].length > objs[2].length) ? objs[2].index : objs[1].index] = 1;
        }
    } else {
        // Late game: pick only the shortest path
        if (objs[0].length > objs[1].length) {
            Mdata->chooseObjectives[(objs[2].length > objs[1].length) ? objs[1].index : objs[2].index] = 1;
        } else {
            Mdata->chooseObjectives[(objs[0].length > objs[2].length) ? objs[2].index : objs[0].index] = 1;
        }
    }

    // Add selected objectives to the bot's memory
    for (int i = 0; i < 3; i++) {
        if (Mdata->chooseObjectives[i]) {
            MyBot->tab_obj[MyBot->nb_obj] = objs[i];
            printf("Objectif gardé : city1 = %d, city2 = %d, score = %d\n",
                   objs[i].city1, objs[i].city2, objs[i].score);
            MyBot->nb_obj++;
        }
    }

    sendMove(Mdata, Mresult);
}

// Simplified version of the bot's objective selection
// Only considers score, not feasibility or distance
void chooseObjectivesBot(MoveResult* Mresult, MoveData* Mdata, partie* MyBot) {
    Mdata->action = DRAW_OBJECTIVES;
    sendMove(Mdata, Mresult);
    Mdata->action = CHOOSE_OBJECTIVES;

    // Temporary structure for sorting by score
    struct objTmp {
        int index;
        int score;
    } objs[3];

    for (int i = 0; i < 3; i++) {
        objs[i].index = i;
        objs[i].score = Mresult->objectives[i].score;
    }

    // Simple bubble sort by score (ascending)
    for (int i = 0; i < 2; i++) {
        for (int j = i + 1; j < 3; j++) {
            if (objs[i].score > objs[j].score) {
                struct objTmp tmp = objs[i];
                objs[i] = objs[j];
                objs[j] = tmp;
            }
        }
    }

    // Reset objective choices
    for (int i = 0; i < 3; i++) {
        Mdata->chooseObjectives[i] = 0;
    }

    // Choose based on game phase
    if (MyBot->wagons == 45) {
        // Early game: take 2 highest scoring
        Mdata->chooseObjectives[objs[1].index] = 1;
        Mdata->chooseObjectives[objs[2].index] = 1;
    } else if (MyBot->wagons >= 14 && MyBot->wagons <= 45 && MyBot->wagons_opp > 20) {
        // Mid-game: take 2 lowest scoring (presumably easier)
        Mdata->chooseObjectives[objs[0].index] = 1;
        Mdata->chooseObjectives[objs[1].index] = 1;
    } else {
        // Late game: take only the lowest scoring one
        Mdata->chooseObjectives[objs[0].index] = 1;
    }

    // Save chosen objectives
    for (int i = 0; i < 3; i++) {
        if (Mdata->chooseObjectives[i]) {
            MyBot->tab_obj[MyBot->nb_obj].city1 = Mresult->objectives[i].from;
            MyBot->tab_obj[MyBot->nb_obj].city2 = Mresult->objectives[i].to;
            MyBot->tab_obj[MyBot->nb_obj].score = Mresult->objectives[i].score;
            printf("Objectif gardé : city1 = %d, city2 = %d, score = %d\n",
                   MyBot->tab_obj[MyBot->nb_obj].city1,
                   MyBot->tab_obj[MyBot->nb_obj].city2,
                   MyBot->tab_obj[MyBot->nb_obj].score);
            MyBot->nb_obj++;
        }
    }

    sendMove(Mdata, Mresult);
}


// Initializes the routes from the game's track data
void initRoutesFromTrackData(GameData Gdata, route routes_dispos[80]) {
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


// Updates the adjacency matrix for the graph representation of the map
void nbWagons(route route[80], GameData* Gdata, int G[36][36]){
    int Nt = Gdata->nbTracks;
    int Nc = Gdata->nbCities;
    for (int i=0; i<Nc; i++){
        for (int j=0; j<Nc; j++){
            G[i][j] = (i == j) ? 0 : 10000;
        }
    }
    for (int i=0; i<Nt; i++){
        if (route[i].owner == 1) continue;
        if (route[i].owner == 0){
            G[route[i].city1][route[i].city2] = 0;
            G[route[i].city2][route[i].city1] = 0;
            continue;
        }
        G[route[i].city1][route[i].city2] = route[i].length;
        G[route[i].city2][route[i].city1] = route[i].length;
    }
}


// Implements Dijkstra's algorithm to find the shortest path
void dijkstra(int src, route routes_dispos[80], GameData* Gdata, int D[36], int Prec[36]){
    int N = Gdata->nbCities;
    int visite[N];
    int G[N][N];
    nbWagons(routes_dispos, Gdata, G);
    for (int i=0; i<N; i++){
        D[i] = 10000;  // +infini
        visite[i] = 0;
        Prec[i] = -1;
    }
    D[src] = 0;
    int u;
    for (int i=0; i<N; i++){
        u = distanceMini(D, visite, N);
        visite[u] = 1;
        for (int v=0; v<N; v++){
            if ((visite[v] == 0) && (G[u][v] < 10000) && (D[u] + G[u][v] < D[v])){
                D[v] = D[u] + G[u][v];
                Prec[v] = u;
            }
        }
    }
}


// Finds the vertex with the minimum distance that hasn't been visited
int distanceMini(int D[36], int visite[36], int N){
    int min = 10000;
    int indice_min = -1;
    for (int i=0; i<N; i++){
        if ((visite[i]==0) && (D[i]<min)){
            min = D[i];
            indice_min = i;
        }
    }
    return indice_min;
}


// Displays the path from source to destination
void afficherChemin(int src, int dest, int Prec[36]) {
    int v = dest;
    printf("Chemin de %d à %d : ", src, dest);
    while (v != src && v != -1) {
        printf("%d <- ", v);
        v = Prec[v];
    }
    if (v == -1) {
        printf("pas de chemin trouvé.\n");
    } else {
        printf("%d\n", src);
    }
}

// Claims a route for the bot if possible
// Strategy: Prioritize claiming the longest available route that the bot can afford
// - Check all available routes and evaluate their length and required cards
// - Select the route with the maximum length that the bot can claim
// - If no route can be claimed, fallback to drawing cards

void claimer(MoveResult* Mresult, MoveData* Mdata, GameData* Gdata, partie* MyBot, route routes[80]) {
    MyBot->state = 0;  // Reset the bot state

    // Print current cards and wagons
    printf("Etat des cartes : ");
    for (int i = 1; i <= 9; i++) {
        printf("%d:%d ", i, MyBot->cardByColor[i]);
    }
    printf(" | Wagons restants : %d\n", MyBot->wagons);

    // Variables to track the best claimable route
    int len_max = 0;
    int i_max = -1;
    int from_max = -1, to_max = -1, color_max = -1;
    int nbLoco_max = 0;

    // Iterate over all available tracks
    for (int i = 0; i < Gdata->nbTracks; i++) {
        if (routes[i].owner != -1) continue; // Skip if already claimed

        int from = routes[i].city1;
        int to = routes[i].city2;
        int length = routes[i].length;
        int locomotives = MyBot->cardByColor[LOCOMOTIVE];

        // Try both color options (some routes are dual-colored)
        CardColor colors[2] = {routes[i].color1, routes[i].color2};
        int best_color = -1;
        int nb_best_color = 0;

        for (int ci = 0; ci < 2; ci++) {
            CardColor color = colors[ci];
            if (color == NONE) continue;

            if (color != LOCOMOTIVE) {
                int count = MyBot->cardByColor[color];
                if (count + locomotives >= length) {
                    if (count >= nb_best_color) {
                        best_color = color;
                        nb_best_color = count;
                    }
                }
            } else {
                // LOCOMOTIVE means any color can be used, try all options
                for (int k = 1; k <= 8; k++) {
                    int count = MyBot->cardByColor[k];
                    if (count + locomotives >= length) {
                        best_color = k;
                        nb_best_color = count;
                        break;  // Take first valid
                    }
                }
            }
        }

        // If no valid color found, skip this route
        if (best_color == -1) continue;

        int total = nb_best_color + locomotives;
        if (total < length || MyBot->wagons < length) continue;  // Not enough resources

        // Prioritize longest route that is feasible
        if (length > len_max) {
            len_max = length;
            i_max = i;
            from_max = from;
            to_max = to;
            color_max = best_color;
            nbLoco_max = (length > nb_best_color) ? (length - nb_best_color) : 0;
        }
    }

    // If a valid route was found, claim it
    if (i_max != -1) {
        Mdata->action = CLAIM_ROUTE;
        Mdata->claimRoute.from = from_max;
        Mdata->claimRoute.to = to_max;
        Mdata->claimRoute.color = color_max;
        Mdata->claimRoute.nbLocomotives = nbLoco_max;

        printf("Claim: %d -> %d | color: %d | length: %d\n", from_max, to_max, color_max, len_max);

        sendMove(Mdata, Mresult);

        // Update internal game state
        routes[i_max].owner = 0;
        MyBot->nbTracks_me++;
        MyBot->wagons -= len_max;
        MyBot->nbCards -= len_max;
        MyBot->cardByColor[color_max] -= (len_max - nbLoco_max);
        MyBot->cardByColor[LOCOMOTIVE] -= nbLoco_max;
        MyBot->score += calcul(len_max);

        return; // End turn after successful claim
    }

    // Fallback: No route claimable, draw cards instead
    printf("Pas de claim possible, on pioche.\n");
    BoardState board;
    getBoardState(&board);

    int picked = 0;
    for (int i = 0; i < 5 && picked < 2; i++) {
        if (board.card[i] == LOCOMOTIVE && picked == 0) {
            // Take visible locomotive (ends turn)
            Mdata->action = DRAW_CARD;
            Mdata->drawCard = LOCOMOTIVE;
            sendMove(Mdata, Mresult);
            MyBot->cardByColor[LOCOMOTIVE]++;
            picked += 2;
            MyBot->nbCards++;
        } else if (board.card[i] != LOCOMOTIVE) {
            // Take a visible non-locomotive card
            Mdata->action = DRAW_CARD;
            Mdata->drawCard = board.card[i];
            sendMove(Mdata, Mresult);
            MyBot->cardByColor[board.card[i]]++;
            picked++;
            MyBot->nbCards++;
        }
    }

    // Draw blind cards if needed
    while (picked < 2) {
        Mdata->action = DRAW_BLIND_CARD;
        sendMove(Mdata, Mresult);
        MyBot->cardByColor[Mresult->card]++;
        picked++;
        MyBot->nbCards++;
    }
}



// depth first search

int dfs(int src, int dest, int visite[], int nbCities, route routes[], int nbRoutes, int player) {  
    if (src == dest) return 1;
    visite[src] = 1;

    for (int i = 0; i < nbRoutes; i++) {
        if (routes[i].owner == player) { 
            int a = routes[i].city1;
            int b = routes[i].city2;
            int next = -1;

            if (a == src && !visite[b]) next = b;
            else if (b == src && !visite[a]) next = a;

            if (next != -1 && dfs(next, dest, visite, nbCities, routes, nbRoutes, player))
                return 1;
        }
    }

    return 0;
}


// Checks if an objective is completed
int objectifAtteint(obj objectif, route routes[], int nbRoutes, int nbCities, int player) {
    int visite[nbCities];
    for (int i = 0; i < nbCities; i++) visite[i] = 0;

    return dfs(objectif.city1, objectif.city2, visite, nbCities, routes, nbRoutes, player);
}


// Executes the bot's turn based on the current game state
// Strategy: Focus on completing objectives and claiming routes
// - Check if all objectives are completed; if so, pick new objectives or end the turn
// - Use Dijkstra's algorithm to find the shortest path for incomplete objectives
// - Prioritize claiming routes that contribute to completing objectives
// - If no claim is possible, draw cards to prepare for future claims

void playBotTurn(MoveResult* Mresult, MoveData* Mdata, GameData* Gdata, partie* MyBot, route routes[80]) {
    int D[36], Prec[36];
    int obj_atteints = 0;

    // Step 1: Check the completion status of all objectives
    for (int i = 0; i < MyBot->nb_obj; i++) {
        MyBot->tab_obj[i].done = objectifAtteint(MyBot->tab_obj[i], routes, Gdata->nbTracks, Gdata->nbCities, 0);
        printf("Objectif %d -> %d : %s\n",
            MyBot->tab_obj[i].city1, MyBot->tab_obj[i].city2,
            MyBot->tab_obj[i].done ? "ATTEINT" : "PAS ENCORE");
        obj_atteints += MyBot->tab_obj[i].done;
    }

    // Step 2: If all objectives are completed, decide next action
    if (obj_atteints == MyBot->nb_obj){
        if(MyBot->wagons_opp >= 14){
            // Choose new objectives if the opponent has enough wagons
            chooseObjectivesBot2(Mresult, Mdata, MyBot, Gdata, routes);
        }
        else{
            // End the turn if no new objectives can be chosen
            MyBot->state = 1;
            return;
        }
        return;
    }

    // Step 3: If the bot has too many cards, prioritize claiming routes
    if (MyBot->nbCards >= 26){
        MyBot->state = 1;
        return;
    }

    // Step 4: Use Dijkstra's algorithm to find paths for incomplete objectives
    int couleurs_utiles[10] = {0};
    int src;
    int dest;
    for (int i=0; i<MyBot->nb_obj; i++){
        if (MyBot->tab_obj[i].done == 0){
            src = MyBot->tab_obj[i].city1;
            dest = MyBot->tab_obj[i].city2;

            dijkstra(src, routes, Gdata, D, Prec);
            afficherChemin(src, dest, Prec);

            // Step 5: Analyze the path and identify useful colors for claiming routes
            int chemin[20];
            int len = 0;
            int v = dest;
            while (v != src && v != -1) {
                chemin[len++] = v;
                v = Prec[v];
            }
            if (v == -1) {
                printf("Pas de chemin dispo\n");
            } else {
                chemin[len++] = src; 

                for (int i = len - 1; i > 0; i--) {
                    unsigned int c1 = chemin[i];
                    unsigned int c2 = chemin[i - 1];

                    for (int j = 0; j < Gdata->nbTracks; j++) {
                        route r = routes[j];
                        if (r.owner != -1) continue;

                        if ((r.city1 == c1 && r.city2 == c2) || (r.city1 == c2 && r.city2 == c1)) {
                            int length = r.length;
                            int best_color = -1;
                            int nb_best_color = 0;
                            int locomotives = MyBot->cardByColor[LOCOMOTIVE];
                        
                            // Consider both colors of the route (if bicolored)
                            CardColor colors[2] = {r.color1, r.color2};

                            if (colors[0]!= LOCOMOTIVE){
                                couleurs_utiles[colors[0]] += length - MyBot->cardByColor[colors[0]];
                                printf("Color utile : %d\n", colors[0]);
                            }
                            if (colors[1] != NONE && colors[1] != LOCOMOTIVE){
                                couleurs_utiles[colors[1]] += length - MyBot->cardByColor[colors[1]];
                                printf("Color utile : %d\n", colors[1]);
                            }
                        
                            for (int ci = 0; ci < 2; ci++) {
                                CardColor color = colors[ci];
                                if (color == NONE) continue;
                                
                                if (color != LOCOMOTIVE){
                                    int count = MyBot->cardByColor[color];
                                    if (count + locomotives >= length) {
                                        if (count >= nb_best_color) {
                                            best_color = color;
                                            nb_best_color = count;
                                        }
                                    }
                                }
                                else{
                                    for (int k = 1; k <= 8; k++) {
                                        int count = MyBot->cardByColor[k];
                                        best_color = -1;
                                        if (count + locomotives >= length) {
                                            best_color = k;
                                            nb_best_color = count;
                                            break;
                                        }
                                    }
                                }
                            }
                            
                            // Step 6: If no valid color is found, skip this route
                            if (best_color == -1) continue; 
                        
                            // Step 7: If the bot has enough wagons, claim the route
                            if (MyBot->wagons < length) continue;
                        
                            int nbLoco = (length > nb_best_color) ? (length - nb_best_color) : 0;
                        
                            Mdata->action = CLAIM_ROUTE;
                            Mdata->claimRoute.from = c1;
                            Mdata->claimRoute.to = c2;
                            Mdata->claimRoute.color = best_color;
                            Mdata->claimRoute.nbLocomotives = nbLoco;
                        
                            printf("Claimed route %d-%d, color=%d, length=%d\n", c1, c2, best_color, length);

                            sendMove(Mdata, Mresult);
                        
                            routes[j].owner = 0;
                            MyBot->wagons -= length;
                            MyBot->cardByColor[best_color] -= (length - nbLoco);
                            MyBot->cardByColor[LOCOMOTIVE] -= nbLoco;
                            MyBot->nbCards -= length;
                            MyBot->score += calcul(length);
                        
                            return;
                        }
                    }
                }
            }
        }
    }

    // Step 8: If no claim is possible, draw cards
    printf("Pas de claim possible, on pioche.\n");
    BoardState board;
    getBoardState(&board);
    int picked = 0;
    for (int j=0; j<2; j++){
        for (int i = 0; i < 5 && picked < 2; i++) {
            if (couleurs_utiles[board.card[i]] > 0 && board.card[i] != LOCOMOTIVE) {
                Mdata->action = DRAW_CARD;
                Mdata->drawCard = board.card[i];
                printf("carte piochée : %d, numéro : %d\n", board.card[i], i);
                sendMove(Mdata, Mresult);
                MyBot->cardByColor[board.card[i]]++;
                couleurs_utiles[board.card[i]]--;
                picked++;
                MyBot->nbCards++;
                getBoardState(&board);           
            }
        }
    }
    // for (int i = 0; i < 5 && picked < 1; i++) {
    //     if (board.card[i] == LOCOMOTIVE && picked == 0) {
    //         Mdata->action = DRAW_CARD;
    //         Mdata->drawCard = LOCOMOTIVE;
    //         sendMove(Mdata, Mresult);
    //         MyBot->cardByColor[LOCOMOTIVE]++;
    //         picked += 2;
    //     }
    // }
    while (picked < 2) {
        Mdata->action = DRAW_BLIND_CARD;
        printf("carte piochée : %d\n", Mresult->card);
        sendMove(Mdata, Mresult);
        MyBot->cardByColor[Mresult->card]++;
        picked++;
        MyBot->nbCards++;
    }
}


