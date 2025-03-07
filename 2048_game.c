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
      printf("Invalid number: %d", num);
      exit(1);
  }
}

void clear_terminal() { printf("\e[1;1H\e[2J"); }

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

bool move_is_possible() {

  // Check if there is mergeable cells
  for (int row = 0; row < HEIGHT - 1; row++) {
    for (int col = 0; col < WIDTH - 1; col++) {
      if (board[row][col] == board[row][col + 1] ||
          board[row][col] == board[row + 1][col])
        return true;
    }
  }

  // Check if there is a zero present
  for (int row = 0; row < HEIGHT; row++) {
    for (int col = 0; col < WIDTH; col++) {
      if (board[row][col] == 0)
        return true;
    }
  }

  // Check last row
  for (int i = 0; i < WIDTH - 1; i++)
    if (board[HEIGHT-1][i] == board[HEIGHT-1][i+1]) return true;

  // Check last col
  for (int i = 0; i < WIDTH - 1; i++)
    if (board[i][WIDTH-1] == board[i+1][WIDTH-1]) return true;
  
  // print_board();
  return false;
}

typedef enum direction {UP, DOWN, LEFT, RIGHT} movement;
typedef enum result {VICTORY, DEFEAT, PLAYING, ERROR} result;
/*
  move the board in a given direction.
  Return -1 if no movement can be done.
*/
int move_board(movement move) {
  int result = PLAYING;
  // if (!move_is_possible() && result != VICTORY) result = DEFEAT;
  // if (!move_is_possible()) return DEFEAT;
  switch(move) {
    case UP:
      for (int x = 0; x < WIDTH; x++) {

        // Prepare the point to jump and skip the zeros in the way
        int jump = 0, jump_start = 0;
        while (board[jump_start][x] != 0) jump_start++;
        while (board[jump_start + jump][x] == 0 && jump + jump_start < HEIGHT - 1) jump++;
        
        // skip zeros
          for (int y = jump_start; y < HEIGHT - 1; y++) {
            if (y + jump >= HEIGHT) break;
            board[y][x] = board[y + jump][x];
            board[y + jump][x] = 0;
          }

        // Merge cells
        for (int y = 1; y < HEIGHT; y++) {
          if (board[y][x] == board[y-1][x] && board[y][x] != 0) {
            board[y-1][x] *= 2;

            if (board[y-1][x] == 2048) result = VICTORY;
            
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

            if (board[y + 1][x] == 2048) result = VICTORY;
            
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

            if (board[y][x-1] == 2048) result = VICTORY;
            
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

            if (board[y][x + 1] == 2048) result = VICTORY;
            
            board[y][x] = 0;
          } else if (board[y][x + 1] == 0) {
            board[y][x + 1] = board[y][x];
            board[y][x] = 0;
          }
        }
      }
      break;
  }
  // if (!move_is_possible() && result != VICTORY) result = DEFEAT;
  return result;
}

void treat_result(result res) {
  switch (res) {
    case VICTORY:
      printf(GREEN "YOU HAVE WON !!!" RESET "\n");
      exit(0);
      break;
    case DEFEAT:
      printf(RED "YOU LOST!" RESET "\n");
      exit(0);
      break;
    // We don't need to do anything in this case
    case PLAYING:
    default:
      break;
  }
}

/*
  Return true if the new position is valid and added, false if the game is done
*/
bool insert_cell(position *new_pos) {
  int x = new_pos->x, y = new_pos->y;

  bool valid_position = true;
  
  if (x == -1 || y == -1) valid_position = false;
  else board[y][x] = 2;
  
  print_board();
  return valid_position;
}

int main(void) {
  enableRawMod();

  srand(time(NULL));
  
  clear_terminal();

  char move;
  bool playing = true;


  // for (int i = 0; i < WIDTH; i++){
  //   for (int j = 0; j < HEIGHT; j++) {
  //     board[i][j] = i + j;
  //   }
  // }
  // board[0][0] = 1024;
  // board[0][1] = 1024;
  
  // board[0][0] = 0;
  // board[0][1] = 2;
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
    position new_pos;

    // up
    if (move == 'w') {
      result res = move_board(UP);
      new_pos = get_new_cell(HEIGHT - 1, -1);
      
      insert_cell(&new_pos);
      // if (!insert_cell(&new_pos)) res = DEFEAT;
      // 
      treat_result(res);
    }
    // left
    if (move == 'a') {
      result res = move_board(LEFT);
      new_pos = get_new_cell(-1, WIDTH - 1);
      
      insert_cell(&new_pos);
      // if (!insert_cell(&new_pos)) res = DEFEAT;
      treat_result(res);
    }
    // down
    if (move == 's') {
      result res = move_board(DOWN);
      new_pos = get_new_cell(0, -1);
      
      insert_cell(&new_pos);
      // if (!insert_cell(&new_pos)) res = DEFEAT;
      treat_result(res);
    }
    // right
    if (move == 'd') {
      result res = move_board(RIGHT);
      new_pos = get_new_cell(-1, 0);
      
      insert_cell(&new_pos);
      // if (!insert_cell(&new_pos)) res = DEFEAT;
      treat_result(res);
    }

    if (!move_is_possible()) treat_result(DEFEAT);
    clear_terminal();
  }

  return 0;
}
