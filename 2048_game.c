#include <curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define HEIGHT 4
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
int board[HEIGHT][WIDTH];
// int free_cells = HEIGHT * WIDTH;

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
  for (int i = 0; i < HEIGHT; i++) {
    print_row_separator(WIDTH);

    for (int j = 0; j < WIDTH; j++) {
      print_number(board[i][j]);
      printf("|");
    }

    printf("\n");
  }

  print_row_separator(WIDTH);
}

typedef struct Position {
  int x;
  int y;
} position;

position get_new_cell(int row, int col) {
  // assert that either the row or the col is given and not both
  if (!(row == -1 || col == -1) && row != col) {
    perror("both the row and col where not specified");
    exit(1);
  }
  
  position p = {-1, -1};

  bool tried[4] = {false};
  int num_tried = 0;

  if (row == -1) {
    p.x = col;
    while (num_tried < WIDTH) {
      int new_row = rand() % WIDTH;

      if (tried[new_row]) continue;
      
      if (board[new_row][col] == 0) {
        // printf("      %d        %d\n", new_row, col);
        // print_board();
        // printf("board[new_row][col]: %d\n", board[new_row][col]);
        printf("board[%d][%d]: %d\n", new_row, col, board[new_row][col]);
        p.y = new_row;
        return p;
      } else {
        tried[new_row] = true;
        num_tried += 1;
      }
    }
  } else p.y = row;


  if (col == -1) {
    p.y = row;
    while (num_tried < HEIGHT) {
      int new_col = rand() % HEIGHT;

      if (tried[new_col]) continue;
      
      if (board[row][new_col] == 0) {
        p.x = new_col;
        return p;
      } else {
        tried[new_col] = true;
        num_tried += 1;
      }
    }
  } else p.x = col;

  return p;
}

typedef enum direction {UP, DOWN, LEFT, RIGHT} movement;
/*
  move the board in a given direction.
  Return -1 if no movement can be done.
*/
int move_board(movement move) {
  switch(move) {
    case UP:
      for (int x = 0; x < WIDTH; x++) {

        // Prepare the point to jump and skip the zeros in the way
        int jump = 0, jump_start = 0;
        while (board[jump_start][x] != 0) jump_start++;
        while (board[jump_start + jump][x] == 0 && jump + jump_start < HEIGHT - 1) jump++;
        
        // skip zeros
        printf("jump: %d\n", jump);
        printf("jump_start: %d\n", jump_start);
        // if (jump != HEIGHT) {
          for (int y = jump_start; y < HEIGHT - 1; y++) {
            if (y + jump >= HEIGHT) break;
            board[y][x] = board[y + jump][x];
            board[y + jump][x] = 0;
          }
        // }

        // Merge cells
        for (int y = 1; y < HEIGHT; y++) {
          if (board[y][x] == board[y-1][x] && board[y][x] != 0) {
            board[y-1][x] *= 2;
            board[y][x] = 0;
          } else if (board[y-1][x] == 0) {
            board[y-1][x] = board[y][x];
            board[y][x] = 0;
          }
        }
      }
      break;
    case DOWN:
      for (int x = WIDTH - 1; x > -1; x--) {

        // Prepare the point to jump and skip the zeros in the way
        int jump = WIDTH - 1, jump_start = WIDTH - 1;
        while (board[jump_start][x] != 0) jump_start--;
        while (board[jump - jump_start][x] == 0 && jump - jump_start > -1) jump--;
        
        // skip zeros
          for (int y = jump_start; y > -1; y--) {
            if (y - jump < 0) break;
            board[y][x] = board[y - jump][x];
            board[y - jump][x] = 0;
          }

        // Merge cells
        for (int y = HEIGHT - 2; y > -1; y--) {
          if (board[y][x] == board[y + 1][x] && board[y][x] != 0) {
            board[y + 1][x] *= 2;
            board[y][x] = 0;
          } else if (board[y + 1][x] == 0) {
            board[y + 1][x] = board[y][x];
            board[y][x] = 0;
          }
        }
      }
      break;
    case LEFT:
      for (int y = 0; y < HEIGHT; y++) {

        // Prepare the point to jump and skip the zeros in the way
        int jump = 0, jump_start = 0;
        while (board[y][jump_start] != 0) jump_start++;
        while (board[y][jump_start + jump] == 0 && jump + jump_start < WIDTH - 1) jump++;

        // skip zeros
        for (int x = jump_start; x < WIDTH - 1; x++) {
          if (x + jump >= WIDTH)
            break;
          board[y][x] = board[y][x + jump];
          board[y][x + jump] = 0;
        }

        // Merge cells
        for (int x = 1; x < WIDTH; x++) {
          if (board[y][x] == board[y][x - 1]) {
            board[y][x - 1] *= 2;
            board[y][x] = 0;
          } else if (board[y][x - 1] == 0) {
            board[y][x - 1] = board[y][x];
            board[y][x] = 0;
          }
        }
      }
      break;
    case RIGHT:
      for (int y = HEIGHT - 1; y > -1; y--) {

        // Prepare the point to jump and skip the zeros in the way
        int jump = HEIGHT - 1, jump_start = HEIGHT - 1;
        while (board[y][jump_start] != 0) jump_start--;
        while (board[y][jump - jump_start] == 0 && jump - jump_start > -1) jump--;

        // skip zeros
        for (int x = jump_start; x > -1; x--) {
          if (x - jump < 0) break;
          board[y][x] = board[y][x - jump];
          board[y][x - jump] = 0;
        }

        // Merge cells
        for (int x = WIDTH - 2; x > -1; x--) {
          if (board[y][x] == board[y][x + 1]) {
            board[y][x + 1] *= 2;
            board[y][x] = 0;
          } else if (board[y][x + 1] == 0) {
            board[y][x + 1] = board[y][x];
            board[y][x] = 0;
          }
        }
      }
      break;
  }
  return 0;  
}

int main(void) {
  enableRawMod();

  srand(time(NULL));
  
  char move;
  bool playing = true;

  // board[0][0] = 0;
  // board[0][1] = 2;
  // board[0][2] = 4;
  // board[0][3] = 8;

  // board[1][0] = 16;
  // board[1][1] = 32;
  // board[1][2] = 64;
  // board[1][3] = 128;

  // board[2][0] = 256;
  // board[2][1] = 512;
  // board[2][2] = 1024;
  // board[2][3] = 2048;
  
  while (read(STDIN_FILENO, &move, 1) &&
         move != QUIT_KEY &&
         playing
  ) {
    // printf("%c", move);
    if (move == 'p') {
      print_board();
    }

    // up
    if (move == 'w') {
      move_board(UP);
      position new_pos = get_new_cell(HEIGHT - 1, -1);
      int x = new_pos.x, y = new_pos.y; 
      
      printf("x: %d, y: %d\n", x, y);
      if (x == -1 || y == -1) break;
      board[new_pos.y][new_pos.x] = 2;
      print_board();
      
    }
    // left
    if (move == 'a') {
      move_board(LEFT);
      position new_pos = get_new_cell(-1, WIDTH - 1);
      int x = new_pos.x, y = new_pos.y; 
      
      printf("x: %d, y: %d\n", x, y);
      if (x == -1 || y == -1) break;
      board[new_pos.y][new_pos.x] = 2;
      print_board();
    }
    // down
    if (move == 's') {
      move_board(DOWN);
      position new_pos = get_new_cell(0, -1);
      int x = new_pos.x, y = new_pos.y; 
      
      printf("x: %d, y: %d\n", x, y);
      if (x == -1 || y == -1) break;
      board[new_pos.y][new_pos.x] = 2;
      print_board();
    }
    // right
    if (move == 'd') {
      move_board(RIGHT);
      position new_pos = get_new_cell(-1, 0);
      int x = new_pos.x, y = new_pos.y; 
      
      printf("x: %d, y: %d\n", x, y);
      if (x == -1 || y == -1) break;
      board[new_pos.y][new_pos.x] = 2;
      print_board();
    }
  }

  return 0;
}
