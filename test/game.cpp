/*** 
  game.cpp -- the game engine for Othello 

  */

#include "game.hpp"

#define INPUT_BUFFER_SIZE 1024

extern char player1, player2;
extern char *HELP_MESSAGE;
extern bool showLegalMoves;
extern char avail_symbol[];
extern char selfPlayLimit;

static bool temp[64];
static char tempbuf[INPUT_BUFFER_SIZE];

double evaluationTime;

/* Function prototypes */
static bool gameOver(Board *b);
static char parseHumanMove(char *buf);
//static char getRandomMove(Board *board, bool *legalMoves);

/* the main loop of the game. player1 is black, player2 is white */
void playGame(Board *board) {
  /* declare some variables */
  char currentPlayer;
  char countBlack, countWhite;  // # of black and white pieces
  bool legalMoves[64];
  bool hasLegalMove;
  char buf[INPUT_BUFFER_SIZE], bufCopied[INPUT_BUFFER_SIZE];
  char nextMove;
  char x, y;
  const char *color;
  int nUR;  // how many moves to undo/redo
  int countUR;  // how many moves can be actually undo/redo
  char *token, *token2;
  bool hasSpecialCommand;
  bool firstInput;
  bool skip = false;
  evaluationTime = 0;
  /* Timing -- none essential variables */
  clock_t tm1, tm2, steptm1, steptm2;
  double steptime;
  tm1 = clock();
  // seed the random generator
  srand(time(NULL));
  /* main loop */
  while(!gameOver(board)) {
    hasLegalMove = findLegalMoves(board, legalMoves);
    currentPlayer = board->wt == BLACK? player1 : player2;
    color = board->wt == BLACK? "Black" : "White";
    
    // Hand game control over to human after the specified # of self-played moves.
    if(player1 == COMPUTER && player2 == COMPUTER && board->m == selfPlayLimit) {
      currentPlayer = player1 = player2 = HUMAN;
      selfPlayLimit = 120;
    }
    
    if(currentPlayer == HUMAN && !skip) {
      nextMove = ILLEGAL; // initialize for the while loop
      hasSpecialCommand = false;
      firstInput = true;
      while(!hasSpecialCommand &&
            (nextMove == ILLEGAL || (hasLegalMove && nextMove == PASS) || 
            (nextMove != PASS && !legalMoves[(int)nextMove]))) {
        if(!firstInput)
          printf("*** ILLEGAL MOVE! *** Please re-enter.\n");
        printBoard(board, legalMoves);
        if(!hasLegalMove)
          printf("YOU HAVE NO MOVE TO MAKE. PRESS [ENTER] TO PASS. (%s, # %d) ", 
                color, board->m + 1);
        else
          printf("Your move (%s, # %d): ", color, board->m + 1);
        fflush(stdout);
        fgets(buf, INPUT_BUFFER_SIZE, stdin);  // read input for stdin
        buf[strlen(buf)-1] = 0;  // get rid of the new_line character
        strcpy(bufCopied, buf);
        token = strtok(bufCopied, " ");
        token2 = strtok(NULL, " ");
        /* first make should the the input is not empty string */
        if(!strlen(buf)) {
          if(hasLegalMove) {
            firstInput = true;
            continue;
          }
          else { // interpret it as a pass
            nextMove = PASS;
            break;
          }
        }
        /* check for special command */
        if(strcmp(buf, "?") == 0 || strcmp(buf, "help") == 0) {
          printf("\n");
          printf(HELP_MESSAGE, avail_symbol);
          printf("Press [ENTER] to continue...");
          fgets(tempbuf, 10, stdin);
       	  hasSpecialCommand = true;
        }
        if(strcmp(buf, "quit") == 0 || strcmp(buf, "exit") == 0) {
          gameUnfinished = true;
          return;
        }
        if(strcmp(token, "pm") == 0 || strcmp(token, "possiblemoves") == 0) {
          if(token2 == NULL) {
            showLegalMoves = !showLegalMoves;
            printf("Show possible moves: %s\n", showLegalMoves? "ON" : "OFF");
          }
          else {
            showLegalMoves = true;
            avail_symbol[0] = token2[0];
            printf("Show possible moves using '%s'\n", avail_symbol);
          }
          hasSpecialCommand = true;
        }
        else if(strcmp(buf, "swapsides") == 0) {
          char temp = player1;
          player1 = player2;
          player2 = temp;
          const char *sb, *sw;
          sb = player1 == COMPUTER? "COMPUTER" : "HUMAN";
          sw = player2 == COMPUTER? "COMPUTER" : "HUMAN";
          printf("### Sides swaped -- Black: %s, White: %s ###\n", sb, sw);
          hasSpecialCommand = true;
        }
        else if(strcmp(buf, "skip") == 0 || strcmp(buf, "sk") == 0) {
          skip = true;
          hasSpecialCommand = true;
        }
        else if(strcmp(buf, "handover") == 0) {
          if(board->wt == BLACK) {
            player1 = COMPUTER;
            printf("### Black is now played by COMPUTER. ###");
          }
          else {
            player2 = COMPUTER;
            printf("### White is now played by COMPUTER. ###");
          }
          tm1 = clock();
          hasSpecialCommand = true;
        }
        else if(strcmp(token, "undo") == 0 && (token2 == NULL)) {
          if(undoMove(board)) {  
            if(player1 == COMPUTER || player2 == COMPUTER)
              undoMove(board);
          }
          else
            printf("No move to undo!\n");
          hasSpecialCommand = true;
        }
        else if(strcmp(token, "redo") == 0 && token2 == NULL) {
          if(redoMove(board)) {
            if(player1 == COMPUTER || player2 == COMPUTER)
              redoMove(board);
          }
          else
            printf("No move to redo!\n");
          hasSpecialCommand = true;
        }
        else if(strcmp(token, "save") == 0 && token2 == NULL) {
          char name[1024];
          printf("File name (.sav): ");
          fgets(name, INPUT_BUFFER_SIZE, stdin);
          buf[strlen(buf)-1] = 0;  // get rid of the new_line character
          saveGame(board, name);
          hasSpecialCommand = true;
        }
        else if(strcmp(token, "load") == 0 && token2 == NULL) {
          char name[1024];
          printf("File name (.sav): ");
          fgets(name, INPUT_BUFFER_SIZE, stdin);
          buf[strlen(buf)-1] = 0;  // get rid of the new_line character
          Board *newb = loadGame(name);
          if(newb != NULL) {
            free(board);
            board = newb;
          }
          hasSpecialCommand = true;
        }
        else if(strcmp(token, "undo") == 0 || strcmp(token, "redo") == 0) {
          // printf("token2: %s", token2);  // degug
          nUR = token2 == NULL? 0 : atoi(token2);
          countUR = 0;
          if(strcmp(token, "undo") == 0) {
            if(strcmp(token2, "all") == 0) {
              while(undoMove(board))
                countUR++;
            }
            else {
              for(int i=0; i<nUR; i++)
                if(undoMove(board))
                  countUR++;
            }
            printf("%d moves undone.\n", countUR);
            hasSpecialCommand = true;
          }
          else if(strcmp(token, "redo") == 0) {
            if(strcmp(token2, "all") == 0) {
              while(redoMove(board))
                countUR++;
            }
            else {
              for(int i=0; i<nUR; i++)
                if(redoMove(board))
                  countUR++;
            }
            printf("%d moves redone.\n", countUR);
            hasSpecialCommand = true;
          }
        }
        else if(strcmp(token, "save") == 0) {
          saveGame(board, token2);
          hasSpecialCommand = true;
        }
        else if(strcmp(token, "load") == 0) {
          Board *newb = loadGame(token2);
          if(newb != NULL) {
            free(board);
            board = newb;
          }
          hasSpecialCommand = true;
        }
        else if(strcmp(buf, "diff") == 0) {
          printf("Current A.I. level; -- search depth: %d, end game exact moves: %d\n",
                originalSearchDepth, bruteForceDepth);
          hasSpecialCommand = true;
        }
        /* Mainly for internal testing */
        else if(strcmp(buf, "stat") == 0) {
          showStatistics = !showStatistics;
          hasSpecialCommand = true;
        }
        else if(strcmp(buf, "time") == 0) {
          showTime = !showTime;
          hasSpecialCommand = true;
        }
        else if(strcmp(buf, "dots") == 0) {
          showDots = !showDots;
          hasSpecialCommand = true;
        }
        else if(strcmp(token, "sd") == 0) {
          if(token2 == NULL) {
            printf("Current search depth: %d\n", originalSearchDepth);
          }
          else {
            char newDepth = atoi(token2);
            if(newDepth > 0) {
              printf("Search depth (was %d) set to %d.\n", originalSearchDepth, newDepth);
              originalSearchDepth = newDepth;
            }
            else
              printf("INVALID VALUE (input must be a positive integer)!\n");
          }
          hasSpecialCommand = true;
        }
        else if(strcmp(token, "se") == 0) {
          if(token2 == NULL) {
            printf("Current end game exact moves: %d\n", bruteForceDepth);
          }
          else {
            char newDepth = atoi(token2);
            if(newDepth > 0) {
              printf("Number of exact moves (was %d) set to %d.\n", 
                    bruteForceDepth, newDepth);
              bruteForceDepth = newDepth;
            }
            else
              printf("INVALID VALUE (input must be a positive integer)!\n");
          }
          hasSpecialCommand = true;
        }
        else if(strcmp(buf, "mopt") == 0) {
          printf("stat    --  Show statistics\n");
          printf("time    --  Show time\n");
          printf("dots    --  Show progress\n");
          printf("sd <n>  --  Set search depth to <n>\n");
          printf("se <n>  --  Set number of exact moves to <n>\n");
          hasSpecialCommand = true;
        }
        else { /* parse the next move */
          nextMove = parseHumanMove(buf);
        }
        firstInput = false;
      }
      if(!hasSpecialCommand) {
        if(nextMove == PASS)
          makePass(board);
        else
          makeMove(board, nextMove % 8, nextMove / 8);
      }
    }
    else {  // the computer player.
      if(!skip) // avoid printing out board twice
        printBoard(board, legalMoves);
      skip = false;
      /* If computer plays black, make a pause */
      if(board->n == 0 && player1 == COMPUTER) {
        printf("Press [ENTER] to start the game!");
        fgets(tempbuf, 10, stdin);
        tm1 = clock();
      }
      printf("Computer is thinking");
      if(!showDots)
        printf("...\n");
      fflush(stdout);
      if(hasLegalMove) {
        steptm1 = clock();
        nextMove = getMinimaxMove(board, legalMoves);
        steptm2 = clock();
        steptime = (double)(steptm2 - steptm1) / CLOCKS_PER_SEC;
        evaluationTime = steptime;
        y = nextMove / 8;
        x = nextMove - 8*y;
        if(legalMove(board, x, y)) {
          makeMove(board, x, y);
          printf("Computer (%s, # %d) played at %c%c.", color, 
                board->m, x+'a', y+'1');
          if(showTime)
            printf("\t(%.3f seconds)", steptime);
          printf("\n");
        }
        else {
          printf("Computer (%s, # %d) returned ILLEGAL MOVE: %c%c !! Please debug!\n",
                  color, board->m, x+'a', y+'1');
          exit(1);
        }
      }
      else {
        makePass(board);
        if(showDots)
          printf("\n");
        printf("Computer passed. (%s, # %d)\n", color, board->m);
      }
    }
  }
  /* game is over */
  findLegalMoves(board, legalMoves);
  printBoard(board, legalMoves);
  countPieces(board, &countBlack, &countWhite);
  if(countBlack > countWhite) { // use the winner-gets-empties convention.
    printf("Black wins by %d to %d.\n", 64 - countWhite, countWhite);
  }
  else if(countBlack < countWhite) {
    printf("White wins by %d to %d.\n", 64 - countBlack, countBlack);
  }
  else {
    printf("Game is drawn at %d to %d.\n", countBlack, countWhite);
  }
  tm2 = clock();
  totalTimeUsed = (double)(tm2 - tm1) / CLOCKS_PER_SEC;
}

/* Test if the game is over */
static bool gameOver(Board *b) {
  if(findLegalMoves(b, temp)) {
    return false;
  }
  makePass(b);
  if(findLegalMoves(b, temp)) {
    undoMove(b);
    return false;
  }
  undoMove(b);
  return true;
}

/* Parse the move from a string */
static char parseHumanMove(char *buf) {
  char p;
  char x, y;
  if(strcmp(buf, "pass") == 0)
    return PASS;
  if(strlen(buf) < 2)
    return ILLEGAL;
  if(buf[0] >= 'a' && buf[0] <= 'h')
    x = buf[0] - 'a';
  else if(buf[0] >= 'A' && buf[0] <= 'H')
    x = buf[0] - 'A';
  else
    return ILLEGAL;
  p = 1;
  while(buf[p] == ' ' || buf[p] == '-')
    p++;
  if(buf[p] >= '1' && buf[p] <= '8')
    y = buf[p] - '1';
  else
    return ILLEGAL;
  p++;
  while(buf[p] == ' ')
    p++;
  if(buf[p] != 0)
    return ILLEGAL;
  return CONV_21(x, y);
}

/* save game in text format */
void saveGame(Board *b, const char *name0, bool prompt) {
  int length = strlen(name0);
  char *name = (char*)malloc((length+1)*sizeof(char));
  strcpy(name, name0);
  if(length) {
    while(name[length-1] == '\n' || name[length-1] == '\r' || name[length-1] == ' ')
      name[--length] = 0; // remove any trailing returns/newlines/spaces
    bool hasDot = false;
    bool dotFormat = false;
    for(int i=0; i<length; i++) {
      if(hasDot) {
        if(name[i] == '/' || name[i] == '\\')
          hasDot = false;
        else
          dotFormat = true;
      }
      if(name[i] == '.') {
        hasDot = true;
        dotFormat = false;
      }
    }
    if(!dotFormat) {
      name[length] = '.';
      name[length+1] = 's';
      name[length+2] = 'a';
      name[length+3] = 'v';
      name[length+4] = 0;
    }
    FILE *savefile = fopen(name, "r+");
    if(savefile == NULL || !prompt) {
      if(savefile != NULL) {
        fclose(savefile);
        remove(name);
      }
      savefile = fopen(name, "w+");
      if(savefile == NULL && prompt) {
        printf("ERROR - Can't save to \"%s\"\n", name);
        free(name);
        return;
      }
      int success;
      success = fprintf(savefile, "%d ", b->mirrored);
      int m = b->m;
      int top = b->top;
      success = fprintf(savefile, "%d ", m);
      success = fprintf(savefile, "%d ", top);
      for(int i=1; i<=top; i++) {
        char mv[5]; // string: a1, e2, etc.
        const char *str = mv;
        char place = b->moves[i];
        if(place == PASS) {
          str = "pass";
        }
        else {  
          mv[0] = 'a' + (place & 7);
          mv[1] = '1' + (place >> 3);
          mv[2] = 0;
        }
        success = fprintf(savefile, "%s ", str);
      }        
      if(success >= 0 && prompt) {
        printf("Game saved.\n");
      }
      else {
        if(prompt)
          printf("ERROR - Saving game failed!\n");
      }
    }
    else {
      char ans[INPUT_BUFFER_SIZE];
      bool hasAnswer = false;
      while(!hasAnswer) {
        printf("File already exists. Overwrite it? (yes/no): ");
        fgets(ans, INPUT_BUFFER_SIZE, stdin);
        ans[strlen(ans)-1] = 0;  // get rid of the new_line character
        if(strcmp(ans, "yes") == 0 || strcmp(ans, "y") == 0) {
          hasAnswer = true;
          fclose(savefile);
          int remError = remove(name);
          if(remError) {
            printf("ERROR - Can't overwrite specified file!\n");
            free(name);
            return;
          }
          savefile = fopen(name, "w+");
          int success;
          success = fprintf(savefile, "%d ", b->mirrored);
          int m = b->m;
          int top = b->top;
          success = fprintf(savefile, "%d ", m);
          success = fprintf(savefile, "%d ", top);
          for(int i=1; i<=top; i++) {
            char mv[5]; // string: a1, e2, etc.
            const char *str = mv;
            char place = b->moves[i];
            if(place == PASS) {
              str = "pass";
            }
            else {  
              mv[0] = 'a' + (place & 7);
              mv[1] = '1' + (place >> 3);
              mv[2] = 0;
            }
            success = fprintf(savefile, "%s ", str);
          }
          for(int i=0; i<64; i++)
            fprintf(savefile, "%s", "");
          if(success >= 0) {
            printf("Game saved.\n");
          }
          else {
            printf("ERROR - Saving game failed!\n");
          }
        }
        else if(strcmp(ans, "no") == 0 || strcmp(ans, "n") == 0) {
          hasAnswer = true;
          printf("Saving game CANCELLED.\n");
        }
      }
    }
    fclose(savefile);
  }
  else {
    if(prompt)
      printf("Saving game CANCELLED.\n");
  }
  free(name);
}

void saveGame(Board *b, const char *name) {
  saveGame(b, name, 1);
}

void saveGameNoPrompt(Board *b, const char *name) {
  saveGame(b, name, 0);
}

/* Load a saved game */
Board* loadGame(const char *name0) {
  int length = strlen(name0);
  char *name = (char*)malloc((length+1)*sizeof(char));
  strcpy(name, name0);
  Board *bor = NULL;
  if(length) {
    while(name[length-1] == '\n' || name[length-1] == '\r' || name[length-1] == ' ')
      name[--length] = 0; // remove any trailing returns/newlines/spaces
    bool hasDot = false;
    bool dotFormat = false;
    for(int i=0; i<length; i++) {
      if(hasDot) {
        if(name[i] == '/' || name[i] == '\\')
          hasDot = false;
        else
          dotFormat = true;
      }
      if(name[i] == '.') {
        hasDot = true;
        dotFormat = false;
      }
    }
    if(!dotFormat) {
      name[length] = '.';
      name[length+1] = 's';
      name[length+2] = 'a';
      name[length+3] = 'v';
      name[length+4] = 0;
    }
    FILE *loadfile = fopen(name, "r");
    if(loadfile == NULL) {
      printf("ERROR - File '%s' not found!\n", name);
    }
    else {
      int flipped;
      int m, top;
      fscanf(loadfile, "%d ", &flipped);
      fscanf(loadfile, "%d ", &m);
      fscanf(loadfile, "%d ", &top);
      Board *newb = makeBoard(flipped);
      bool error = false;
      for(int i=1; i<=top; i++) {
        char mv[5];
        char x, y;
        fscanf(loadfile, "%s ", mv);
        if(strcmp(mv, "pass") == 0) {
          bool lm[64];
          if(!findLegalMoves(newb, lm)) {
            makePass(newb);
          }
          else {
            error = true;
            break;
          }
        }
        else {  
          x = (mv[0] - 'a');
          y = (mv[1] - '1');
          if(legalMove(newb, x, y)) {
            makeMove(newb, x, y);
          }
          else {
            error = true;
            break;
          }
        }
      }
      for(int i=0; i<top-m && !error; i++) {
        if(!undoMove(newb)) {
          error = true;
          break;
        }
      }
      if(error) {
        newb = NULL;
        printf("ERROR - File is invalid or damaged!\n");
      }
      else {
        printf("Saved game loaded.\n");
      }
      fclose(loadfile);
      bor = newb;
    }
  }
  else {
    printf("Loading game CANCELLED.\n");
  }
  free(name);
  return bor;
}

/* Generate a random move */
//static char getRandomMove(Board *board, bool *legalMoves) {
//  char start = (char)(rand() % 64);
//  for(char i=start; i<64; i++)
//    if(legalMoves[i])
//      return i;
//  for(char i=0; i<start; i++)
//    if(legalMoves[i])
//      return i;
//  return PASS;
//}

// temporary
/*
char searchDepth;
char bruteForceDepth;
bool winLarge;
*/

/******  Main: test/debug ***************/
/*
int main(int argc, char **argv) {
  searchDepth = 8;
  char oldSD = searchDepth;
  bruteForceDepth = 16;
  winLarge = 1;
  char isFlipped = 0;
  char player1 = HUMAN;
  char player2 = COMPUTER;
  if(argc >= 3) {
    player1 = strcmp(argv[1], "c")? HUMAN : COMPUTER;
    player2 = strcmp(argv[2], "c")? HUMAN : COMPUTER;
  }  
  if(argc >= 4) 
    searchDepth = atoi(argv[3]);
  if(argc >= 5)
    bruteForceDepth = atoi(argv[4]);
  if(argc >= 6)
    winLarge = atoi(argv[5]);
  if(argc >= 7)
    isFlipped = atoi(argv[6]);

  time_t t1, t2;
  time(&t1);
  
  Board *gb = makeBoard(isFlipped);
  playGame(gb, player1, player2);
  
  time(&t2);
  double timePassed = difftime(t2, t1); 
  printf("Total time: %f seconds. (depth: %d, brute-force: %d, winLarge: %d)\n",
         timePassed, oldSD, bruteForceDepth, winLarge);
  
  //makeMove(gb, 5, 4);
//  bool legalMoves[64];
//  findLegalMoves(gb, legalMoves);
//  printBoard(gb, legalMoves);
//  char compMove = getMinimaxMove(gb, legalMoves);
//  printf("compMove: %d\n", compMove);
//  makeMove(gb, compMove % 8, compMove / 8);
//  findLegalMoves(gb, legalMoves);
//  printBoard(gb, legalMoves);
  
  //// playGame(gb, HUMAN, COMPUTER);
//  printBoard(gb, legalMoves);
//  unsigned int mask0, mask1;
//  int nLegalMoves = findLegalMoves(gb->a[0], gb->wt, &mask0, &mask1);
//  printf("nLegalMoves: %d\n", nLegalMoves);
//  char index = 0;
//  while(index < 64 && !legalMoves[index])
//    index++;
//  copyBoardArray(gb->a[1], gb->a[0]);
//  bool *temp = (bool*)calloc(64, sizeof(bool));
//  tryMove(gb->a[0], BLACK, index % 8, index / 8);
//  printBoard(gb, temp);
//  tryMove(gb->a[0], WHITE, 2, 4);
//  printBoard(gb, temp);
//  tryMove(gb->a[0], BLACK, 3, 5);
//  printBoard(gb, temp);
//  tryMove(gb->a[0], WHITE, 4, 2);
//  printBoard(gb, temp);
  return 0;
}
// */

  /*
  bool correct = true;
  for(int i=0; i<32; i++) {
    if((legalMoves[i] && !(mask0 & (1 << i))) ||
       (!legalMoves[i] && (mask0 & (1 << i)))) {
      correct = false;
      printf("Incorrect at bit %d\n", i);
    }
  }
  for(int i=32; i<64; i++) {
    if((legalMoves[i] && !(mask1 & (1 << (i-32)))) || 
       (!legalMoves[i] && (mask1 & (1 << (i-32))))) {
      correct = false;
      printf("Incorrect at bit %d\n", i);
    }
  }
  printf("Test result: %d\n", correct);
  bool wrongMoves[64];
  for(int i=0; i<32; i++)
    wrongMoves[i] = mask0 & (1 << i);
  for(int i=32; i<64; i++)
    wrongMoves[i] = mask1 & (1 << (i-32));
  printf("Wrong moves: \n");
  printBoard(gb, wrongMoves);
  // */


