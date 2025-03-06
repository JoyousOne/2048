#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <curses.h>                   

#define HEIGTH 4
#define WIDTH 4
#define QUIT_KEY 'q'

// Color for the cells
// https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"
#define B_RED     "\x1b[91m"
#define B_GREEN   "\x1b[92m"
#define B_YELLOW  "\x1b[93m"
#define B_BLUE    "\x1b[94m"
#define B_MAGENTA "\x1b[95m"
#define B_CYAN    "\x1b[96m"
#define B_WHITE   "\x1b[97m"
#define RESET   "\x1b[0m"

// The game board
int board[HEIGTH][WIDTH];

struct termios orig_termios;

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1 ) {
    perror("error when disabling raw mode");
  }
 }

void enableRawMod() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
    perror("error in enableRawMode with: tcgetattr");
  }
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  raw.c_lflag &= ~(ECHO | ICANON);

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    perror("error in enableRawMode with: tcsetattr");
  }
}

void print_row_separator(int length) {
  for (int k = 0; k < length; k++)
    printf("+----");
  printf("+\n|");
}

void print_number(int num) {
  switch(num) {
    case 0:
      printf(WHITE "   0" RESET "");
      break;
    case 2:
      printf(B_WHITE "   2" RESET "");
      break;
    case 4:
      printf(B_RED "   4" RESET "");
      break;
    case 8:
      printf(RED "   8" RESET "");
      break;
    case 16:
      printf(B_YELLOW "  16" RESET "");
      break;
    case 32:
      printf(GREEN "  32" RESET "");
      break;
    case 64:
      printf(B_CYAN "  64" RESET "");
      break;
    case 128:
      printf(CYAN " 128" RESET "");
      break;
    case 256:
      printf(BLUE " 256" RESET "");
      break;
    case 512:
      printf(B_BLUE " 512" RESET "");
      break;
    case 1024:
      printf(MAGENTA "1024" RESET "");
      break;
    case 2048:
      printf(B_MAGENTA "2048" RESET "");
      break;
    default:
      exit(1);
  }
}

void print_board() {
  for (int i = 0; i < HEIGTH; i++) {
    print_row_separator(WIDTH);

    for (int j = 0; j < WIDTH; j++) {
      print_number(board[i][j]);
      printf("|");
    }

    printf("\n");
  }

  print_row_separator(WIDTH);
}

int main(void) {
  enableRawMod();

  char move;
  bool playing = true;

  board[0][0] = 0;
  board[0][1] = 2;
  board[0][2] = 4;
  board[0][3] = 8;

  board[1][0] = 16;
  board[1][1] = 32;
  board[1][2] = 64;
  board[1][3] = 128;

  board[2][0] = 256;
  board[2][1] = 512;
  board[2][2] = 1024;
  board[2][3] = 2048;
  
  while (read(STDIN_FILENO, &move, 1) &&
         move != QUIT_KEY &&
         playing
  ) {
    // printf("%c", move);
    if (move == 'p') {
      print_board();
    }
  }

  return 0;
}
