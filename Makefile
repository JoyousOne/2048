EXEC=2048_game

# OPTIONS = -Wall
# OPTIONS = -g -O0 -Wall
OPTIONS = -O2 -Wall

main: 2048_game.c
	gcc ${OPTIONS} -o ${EXEC} 2048_game.c

clean:
	rm -rf  ${EXEC} *.o
