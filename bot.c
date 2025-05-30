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

    // On garde au moins le 1er, et le plus long des 2 autres
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
        Mdata->chooseObjectives[1] = (minIndex == 2);
        Mdata->chooseObjectives[2] = (minIndex == 1);
        MyBot->tab_obj[MyBot->nb_obj].city1 = Mresult->objectives[2].from;
        MyBot->tab_obj[MyBot->nb_obj].city2 = Mresult->objectives[2].to;
        MyBot->tab_obj[MyBot->nb_obj].score = Mresult->objectives[2].score;
        printf("city1 : %d, city2 : %d, score : %d\n", MyBot->tab_obj[MyBot->nb_obj].city1, MyBot->tab_obj[MyBot->nb_obj].city2,MyBot->tab_obj[MyBot->nb_obj].score);
        MyBot->nb_obj += 1;
    }
    else{
        Mdata->chooseObjectives[1] = (minIndex == 2);
        Mdata->chooseObjectives[2] = (minIndex == 1);
        MyBot->tab_obj[MyBot->nb_obj].city1 = Mresult->objectives[1].from;
        MyBot->tab_obj[MyBot->nb_obj].city2 = Mresult->objectives[1].to;
        MyBot->tab_obj[MyBot->nb_obj].score = Mresult->objectives[1].score;
        printf("city1 : %d, city2 : %d, score : %d\n", MyBot->tab_obj[MyBot->nb_obj].city1, MyBot->tab_obj[MyBot->nb_obj].city2,MyBot->tab_obj[MyBot->nb_obj].score);
        MyBot->nb_obj += 1;
    }

    sendMove(Mdata, Mresult);
}


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


/*
void playBotTurn(MoveResult* Mresult, MoveData* Mdata, GameData* Gdata, partie* MyBot, route routes[80]) {
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
*/

int dfs(int src, int dest, int visite[], int nbCities, route routes[], int nbRoutes) {
    if (src == dest) return 1;
    visite[src] = 1;

    for (int i = 0; i < nbRoutes; i++) {
        if (routes[i].owner == 0) {  // Seulement les routes possédées par le bot
            int a = routes[i].city1;
            int b = routes[i].city2;
            int next = -1;

            if (a == src && !visite[b]) next = b;
            else if (b == src && !visite[a]) next = a;

            if (next != -1 && dfs(next, dest, visite, nbCities, routes, nbRoutes))
                return 1;
        }
    }

    return 0;
}

int objectifAtteint(obj objectif, route routes[], int nbRoutes, int nbCities) {
    int visite[nbCities];
    for (int i = 0; i < nbCities; i++) visite[i] = 0;

    return dfs(objectif.city1, objectif.city2, visite, nbCities, routes, nbRoutes);
}


void playBotTurn(MoveResult* Mresult, MoveData* Mdata, GameData* Gdata, partie* MyBot, route routes[80]) {
    int D[36], Prec[36];
    int obj_atteints = 0;

    for (int i = 0; i < MyBot->nb_obj; i++) {
        MyBot->tab_obj[i].done = objectifAtteint(MyBot->tab_obj[i], routes, Gdata->nbTracks, Gdata->nbCities);
        printf("Objectif %d -> %d : %s\n",
            MyBot->tab_obj[i].city1, MyBot->tab_obj[i].city2,
            MyBot->tab_obj[i].done ? "ATTEINT" : "PAS ENCORE");
        obj_atteints += MyBot->tab_obj[i].done;
    }

    if (obj_atteints == MyBot->nb_obj){
        chooseObjectivesBot(Mresult, Mdata, MyBot);
        return;
    }

    int src;
    int dest;
    for (int i=0; i<MyBot->nb_obj; i++){
        if (MyBot->tab_obj[i].done == 0){
            src = MyBot->tab_obj[i].city1;
            dest = MyBot->tab_obj[i].city2;

            dijkstra(src, routes, Gdata, D, Prec);
            afficherChemin(src, dest, Prec);

            // Parcours du chemin
            int chemin[36];
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
                            int needed = r.length;
                            int color = r.color1;
                            int locomotives = MyBot->cardByColor[LOCOMOTIVE];
                            int color_cards = MyBot->cardByColor[color];

                            if (color == LOCOMOTIVE) {
                                for (int k = 1; k <= 8; k++) {
                                    if (MyBot->cardByColor[k] + locomotives >= needed) {
                                        color = k;
                                        color_cards = MyBot->cardByColor[k];
                                        break;
                                    }
                                }
                            }

                            if (MyBot->wagons >= needed && (color_cards + locomotives >= needed)) {
                                int nbLoco = (needed > color_cards) ? (needed - color_cards) : 0;
                                Mdata->action = CLAIM_ROUTE;
                                Mdata->claimRoute.from = c1;
                                Mdata->claimRoute.to = c2;
                                Mdata->claimRoute.color = color;
                                Mdata->claimRoute.nbLocomotives = nbLoco;

                                sendMove(Mdata, Mresult);

                                routes[j].owner = 0;
                                MyBot->wagons -= needed;
                                MyBot->cardByColor[color] -= (needed - nbLoco);
                                MyBot->cardByColor[LOCOMOTIVE] -= nbLoco;

                                printf("Claimed route %d-%d, color=%d, length=%d\n", c1, c2, color, needed);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    

    // Si aucun claim possible, piocher
    printf("Pas de claim possible, on pioche.\n");
    BoardState board;
    getBoardState(&board);
    int picked = 0;
    for (int i = 0; i < 5 && picked < 2; i++) {
        if (board.card[i] == LOCOMOTIVE && picked == 0) {
            Mdata->action = DRAW_CARD;
            Mdata->drawCard = LOCOMOTIVE;
            sendMove(Mdata, Mresult);
            MyBot->cardByColor[LOCOMOTIVE]++;
            picked += 2;
        } else if (board.card[i] != LOCOMOTIVE) {
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


