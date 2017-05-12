/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// include POSIX 2004 standards
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <getopt.h> // getopt
#include <ctype.h>  // isprint
#include <math.h>   // floor
#include <string.h> // strtok
#include <time.h>   // nanosleep, time

#define FIELD_SIZE 9
#define FIELD_CENTER 4
#define LINE_BUF 50
char FILL_CHAR = '0';
char EMPTY_CHAR = '.';

/* STRUCTS */
typedef struct _board_ *Board;
struct _board_ {
  int **cells; // 0:empty, 1:filled| [r][c]
  size_t w;
  size_t h;
  long long int live;
};
typedef Board Update;
typedef struct _rules_ *Rules;
struct _rules_ {
  // both arrays defined by <# of neighbors>=index
  int *survive; // how many live neighbors to survive
  int *create;  // how many live neighbors to create
};

/* FUNCTIONS */
Board board_create(size_t, size_t);
void board_delete(Board *);
void board_print(Board);
int board_update(Board, Rules);
Board board_random(size_t, size_t, unsigned int);
Board file_to_board(FILE *);
void board_to_file(Board, FILE *);
Rules rules_create(int *, int *);
void rules_delete(Rules *);
int rules_check(int *, Rules);
Rules file_to_rules(FILE *);
Update update_generate(Board, Rules);

int main(int argc, char **argv) {
  /* INITIALIZATIONS */
  int opt;
  long long int iterations = 1;
  double delay = 0;
  FILE *board_input_file = NULL, *rules_file = NULL, *board_output_file = NULL;
  int bif_hasval = 0, rf_hasval = 0, bof_hasval = 0;
  int isGenRand = 0;
  char genRandDim[50] = {0};
  int disp = 1; // 0:none, 1:some, 2:all
  int isInteractive = 0;

  /* OPTIONS PARSING */
  while ((opt = getopt(argc, argv, "b:r:o:n:d:h:g:vsci")) != -1) {
    switch (opt) {
    case 'i':
      isInteractive = 1;
      break;
    case 'g':
      isGenRand = 1;
      memcpy(genRandDim, optarg, strlen(optarg));
      break;
    case 'h':
      switch (optarg[0]) {
      case 'b':
        printf("(Board file help)\n"
               "A board file is a space-delimited plain text file with the "
               "format (without the underscores):\n"
               "________________________________\n"
               "<width of grid> <height of grid>\n<col> <row>\n<col> <row>\n"
               "...\n<col> <row>\n"
               "________________________________\n"
               "Each (<col> <row>) pair defines a live cell in the grid.\n\n");
        return 0;
      case 'r':
        printf("(Rule file help)\n"
               "A rule file is a space-delimited plain text file with the "
               "format (without the underscores):\n"
               "___________________________________\n"
               "<0> <1> <2> <3> <4> <5> <6> <7> <8>\n"
               "<0> <1> <2> <3> <4> <5> <6> <7> <8>\n"
               "___________________________________\n"
               "Each of the numbers can either be 0 or 1 in the file. Each "
               "line has exactly 9 numbers.\n"
               "The first line corresponds to which number of neighbors is "
               "needed for a live cell to live (1) or die (0).\n"
               "The second line corresponds to which number of neighbors is "
               "needed for a dead cell to become live (1) or remain dead "
               "(0).\n\n");
        return 0;
      default:
        printf("No help text exists for that\n\n");
        return 1;
      }
      return 0;
    case 'v':
      disp = 2;
      break;
    case 's':
      disp = 0;
      break;
    case 'c':
      disp = 1;
      break;
    case 'b':
      board_input_file = fopen(optarg, "r");
      bif_hasval = 1;
      break;
    case 'r':
      rules_file = fopen(optarg, "r");
      rf_hasval = 1;
      break;
    case 'o':
      board_output_file = fopen(optarg, "w");
      bof_hasval = 1;
      break;
    case 'n':
      iterations = strtoll(optarg, NULL, 10);
      break;
    case 'd':
      delay = strtod(optarg, NULL);
      break;
    case '?':
      if (optopt == 'h') {
        printf(
            "blifs -  \"bareLifeSimulator\": simulates and generates 2D "
            "cellular automata in a console\n\n"
            "Usage: blifs <options>\n\n"
            "(Required)\n"
            "\t-b board-file\t\tLoad a board file\n"
            "\t\t\t\t(required unless '-g' is passed)\n"
            "\t-r rules-file\t\tLoad a rules file\n"
            "\t\t\t\t(required unless '-n 0' is passed)\n"
            "\t-o file\t\t\tOutput final result as a board file\n"
            "\t\t\t\t(required when '-g' is passed, optional otherwise)\n\n"
            "(Optional)\n"
            "\t-c\t\t\tDisplay only initial & final iteration, or info for "
            "'-g' (default)\n"
            "\t-v\t\t\tDisplay verbose output\n"
            "\t-s\t\t\tDon't display any output\n\n"
            "\t-n integer\t\tNumber of times to run simulations (1 by "
            "default)\n"
            "\t\t\t\t(if 0 is passed, then the rules file isn't required)\n"
            "\t-d double\t\tDelay between displaying each simulation step\n"
            "\t-i\t\t\tInteractive mode, press any key to go to the next "
            "generation\n"
            "\t\t\t\t(0 by default)\n"
            "\t-g w:h:d\t\tGenerate a random board of size <w>x<h> with "
            "a 1/<d> chance of a live cell\n"
            "\t\t\t\t(w & h are required, d is optional and 2 by default)\n\n"
            "\t-h b\t\t\tShow info on board files\n"
            "\t-h r\t\t\tShow info on rules files\n"
            "\t-h\t\t\tShow these help options\n\n");
        return 0;
      } else if (optopt == 'b' || optopt == 'r' || optopt == 'o' ||
                 optopt == 'n' || optopt == 'd' || optopt == 'g')
        fprintf(stderr, "Option %c requires an argument\n", optopt);
      else if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'\n", optopt);
      return 1;
    default:
      abort();
    }
  }

  /* PROGRAM PROPER */
  if (isGenRand == 0) {

    if (bif_hasval == 0) {
      fprintf(stderr, "Input board file required\n");
      return 1;
    }
    if (rf_hasval == 0 && iterations != 0) {
      fprintf(stderr, "Rule file required\n");
      return 1;
    }

    Board b = file_to_board(board_input_file);
    if (b == NULL)
      return 1;

    long long int tot_size = b->w * b->h;

    Rules r = NULL;
    if (iterations != 0)
      r = file_to_rules(rules_file);

    if (disp > 0) {
      board_print(b);
      printf("[READ (%zu*%zu) BOARD, ALIVE: %lld/%lld]\n", b->w, b->h, b->live,
             tot_size);
    }

    long long int nth = 0;

	struct timespec *req = NULL;
	if (delay != 0) {
 	   req = malloc(sizeof(struct timespec));
   	   req->tv_sec = floor(delay);
       req->tv_nsec = (long)((double)(delay - floor(delay)) * (double)1000000000);
	}

    while (nth != iterations) {
      if (isInteractive == 1)
        getchar();
      int goprint = ((nth == iterations - 1 && disp == 1) || disp == 2);
      if (board_update(b, r)) {
        if (goprint)
          board_print(b);
        nth++;
      } else {
        if (goprint) {
          board_print(b);
          printf("[BOARD IS STATIC]\n");
        }
        break;
      }
      if (goprint)
        printf("[GEN %lld/%lld, ALIVE: %lld/%lld (%zu*%zu)]\n", nth, iterations,
               b->live, tot_size, b->w, b->h);
      if (nth != iterations && delay != 0 && req != NULL)
        nanosleep(req, NULL);
    }

    free(req);

    fclose(board_input_file);
    if (iterations != 0) {
      rules_delete(&r);
      fclose(rules_file);
    }

    if (bof_hasval == 1) {
      board_to_file(b, board_output_file);
      board_delete(&b);
      fclose(board_output_file);
    }
  } else {
    if (bof_hasval == 0) {
      fprintf(stderr,
              "Output board file required for generating a random board\n");
      return 1;
    }
    char *w_c = strtok(genRandDim, ":");
    char *h_c = strtok(NULL, ":");
    char *s_c = strtok(NULL, ":");
    if (w_c == NULL || h_c == NULL)
      fprintf(stderr, "Invalid arguments for '-g'\n");
    size_t width = (size_t)strtol(w_c, NULL, 10);
    size_t height = (size_t)strtol(h_c, NULL, 10);
    unsigned int div = 1;

    if (s_c != NULL)
      div = (unsigned int)strtol(s_c, NULL, 10);

    Board new_b = board_random(width, height, div);

    board_to_file(new_b, board_output_file);
    if (disp > 0) {
      double fill =
          ((double)(new_b->live) / ((double)(new_b->w) * (double)(new_b->h))) *
          100.0;
      double div_perc = ((double)1 / (double)div) * 100;
      if (disp == 2)
        board_print(new_b);
      printf("[ACTUAL LIVE: %.2lf%%, LIFE PROBABILITY: %.2lf (%zu*%zu)]\n",
             fill, div_perc, new_b->w, new_b->h);
    }
    board_delete(&new_b);
    fclose(board_output_file);
  }

  return 0;
}

Board board_create(size_t w, size_t h) {
  Board b = malloc(sizeof(struct _board_));
  b->cells = malloc(sizeof(int *) * w);
  for (int i = 0; i < w; i++) {              // per column(x)
    (b->cells)[i] = malloc(sizeof(int) * h); // per row(y) in column i(x)
    for (int j = 0; j < h; j++) {
      (b->cells)[i][j] = 0;
    }
  }
  b->w = w;
  b->h = h;
  b->live = 0;
  return b;
}
Rules rules_create(int *s, int *c) {
  Rules ret = malloc(sizeof(struct _rules_));
  ret->survive = malloc(sizeof(int) * FIELD_SIZE);
  ret->create = malloc(sizeof(int) * FIELD_SIZE);
  for (int i = 0; i < FIELD_SIZE; i++) {
    (ret->survive)[i] = s[i];
    (ret->create)[i] = c[i];
  }
  return ret;
}
void board_delete(Board *b) {
  for (int i = 0; i < (*b)->w; i++) {
    free(((*b)->cells)[i]);
  }
  free((*b)->cells);
  free(*b);
  *b = NULL;
}
void rules_delete(Rules *r) {
  free((*r)->survive);
  free((*r)->create);
  free(*r);
  *r = NULL;
}
int rules_check(int *field, Rules r) {
  int living = 0;
  for (int i = 0; i < FIELD_SIZE; i++) {
    if (i != FIELD_CENTER)
      living += field[i];
  }
  int curr_state = field[FIELD_CENTER];
  if (curr_state == 0) {
    if ((r->create)[living] == 1)
      return 1;
    else
      return 2;
  } else {
    if ((r->survive)[living] == 1)
      return 0;
    else
      return 2;
  }
  return -1;
}
Update update_generate(Board b, Rules r) {
  Update u = board_create(b->w, b->h);
  for (int i = 0; i < b->w; i++) {
    int field[FIELD_SIZE]; // per column(x)

    int x_l = i - 1, x_r = i + 1;

    // for wrapping around the board horizontally
    if (i == 0)
      x_l = b->w - 1;
    else if (i == b->w - 1)
      x_r = 0;

    for (int j = 0; j < b->h; j++) { // per row(y) in column i(x)
      int y_t = j - 1, y_b = j + 1;

      // for wrapping around the board vertically
      if (j == 0)
        y_t = b->h - 1;
      else if (j == b->h - 1)
        y_b = 0;

      field[0] = (b->cells)[x_l][y_t];
      field[1] = (b->cells)[i][y_t];
      field[2] = (b->cells)[x_r][y_t];
      field[3] = (b->cells)[x_l][j];
      field[4] = (b->cells)[i][j];
      field[5] = (b->cells)[x_r][j];
      field[6] = (b->cells)[x_l][y_b];
      field[7] = (b->cells)[i][y_b];
      field[8] = (b->cells)[x_r][y_b];

      int res = rules_check(field, r);
      (u->cells)[i][j] = res;

      if (res == 2 && (b->cells)[i][j] != 0) {
        u->live -= 1;
      } else if (res == 1) {
        u->live += 1;
      }
    }
  }
  return u;
}
void board_print(Board b) {
  for (int i = 0; i < b->h; i++) {
    for (int j = 0; j < b->w; j++) {
      if ((b->cells)[j][i] == 1)
        putchar(FILL_CHAR);
      else if ((b->cells)[j][i] == 0)
        putchar(EMPTY_CHAR);
      else
        putchar('0' + (b->cells)[j][i]);
    }
    putchar('\n');
  }
  fflush(stdout);
}
int board_update(Board b, Rules r) {
  Update u = update_generate(b, r);
  b->live += u->live;
  int isEmpty = 1;
  for (int i = 0; i < b->w; i++) {
    for (int j = 0; j < b->h; j++) {
      int upd = (u->cells)[i][j];
      if (upd == 0)
        continue;
      else if (upd == 1) {
        (b->cells)[i][j] = 1;
        isEmpty = 0;
      } else if (upd == 2) {
        (b->cells)[i][j] = 0;
      }
    }
  }
  return !isEmpty;
  board_delete(&u);
}
Board file_to_board(FILE *fi) {
  int row_tot, col_tot;
  fscanf(fi, "%d", &col_tot);
  fscanf(fi, "%d", &row_tot);
  Board b = board_create(col_tot, row_tot);
  int **bc = (b->cells);
  int ctr = 0;
  int buf = 0;
  int row = 0, col = 0;
  while (fscanf(fi, "%d", &buf) != EOF) {
    if (ctr == 0) {
      col = buf;
      if (col >= col_tot) {
        fprintf(stderr, "Board file has invalid cell definitions");
        return NULL;
      }
      ctr = 1;
    } else {
      row = buf;
      if (row >= row_tot) {
        fprintf(stderr, "Board file has invalid cell definitions");
        return NULL;
      }
      bc[col][row] = 1;
      b->live += 1;
      ctr = 0;
    }
  }
  return b;
}
Rules file_to_rules(FILE *fi) {
  int s[9] = {0};
  int c[9] = {0};
  for (int i = 0; i < 18; i++) {
    if (i < 9) {
      fscanf(fi, "%d", &s[i]);
    } else {
      fscanf(fi, "%d", &c[i - 9]);
    }
  }
  return (rules_create(s, c));
}
void board_to_file(Board b, FILE *fo) {
  fprintf(fo, "%zu %zu\n", b->w, b->h);
  fflush(fo);
  for (int i = 0; i < b->w; i++) {
    for (int j = 0; j < b->h; j++) {
      if ((b->cells)[i][j] == 1) {
        fprintf(fo, "%d %d\n", i, j);
        fflush(fo);
      }
    }
  }
}
Board board_random(size_t w, size_t h, unsigned int div) {
  Board b = board_create(w, h);
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  srandom((time_t)ts.tv_nsec);
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      int val = div > 2 ? (random() % div) : (random() % 2);
      if (val == 1) {
        b->live += 1;
        (b->cells)[i][j] = val;
      }
    }
  }
  return b;
}
