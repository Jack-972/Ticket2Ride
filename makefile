CC = gcc
CFLAGS = -Wall -Wextra -g
PROGRAM = main

# Tous les fichiers objets nécessaires
OBJS = main.o jeu.o outils.o data.o clientAPI.o ticketToRide.o bot.o

# Règle principale
all: $(PROGRAM)

# Link final
$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compilation des .o à partir des .c
main.o: main.c jeu.h outils.h data.h
	$(CC) $(CFLAGS) -c main.c

jeu.o: jeu.c jeu.h outils.h data.h
	$(CC) $(CFLAGS) -c jeu.c

outils.o: outils.c outils.h
	$(CC) $(CFLAGS) -c outils.c

data.o: data.c data.h
	$(CC) $(CFLAGS) -c data.c

bot.o : bot.c bot.h
	$(CC) $(CFLAGS) -c bot.c


# API
clientAPI.o: ../tickettorideapi/clientAPI.c ../tickettorideapi/clientAPI.h
	$(CC) $(CFLAGS) -c ../tickettorideapi/clientAPI.c

ticketToRide.o: ../tickettorideapi/ticketToRide.c ../tickettorideapi/ticketToRide.h
	$(CC) $(CFLAGS) -c ../tickettorideapi/ticketToRide.c

# Nettoyage
clean:
	rm -f *.o $(PROGRAM)
