/*******
  minimax.cpp
  This is the AI minimax search and evaluation function for Othello.
 */

#include <math.h>
#include <string.h>
#include "minimax.hpp"
#include "liberty.hpp"
#include "mobility.hpp"
#include "adj_table.hpp"

//#ifndef NDEBUG
//#define NDEBUG  // uncomment to disable 'assert()' debug -- not recommended
//#endif

static int base;

unsigned int countPruning;
unsigned int countSearching, countEval;
double evaluationValue;
int evaluationValueExact;
int WDLresponse;

static bool quiescenceSearch;
static bool inEndGame;
static bool useEndGameSolver;

static double extra;  // to avoid zero denominator
static double cornerValue;
static double discWeight;
static double near2cornerDiagCoeff;
static double discWeightArr[64]; // indexed by empty squares

// Row/column/diagonal configuration indices -- 4-based
// 0-7: row 0-7, 8-15: column 0-7, 
// 16-30: 15 major-direction diagonals (order: left->right)
// 31-45: 11 non-major-dir. diagonals (order: left->right)
// 46: dummy index, always == 0
#define DUMMY_CONFIG_ENTRY  46
static unsigned short configIndices[47];

// Maps a square to the entries of its row, column, and diagonals in configIndices
// 6 bits unit from low to high: row, column, maj-diag, non-maj-diag (entryies)
// 0 == non applicable (doesn't make sence as index to entries, as described above
// #define CEM(x, y, z, w) ((x) | ((y) << 6) | ((z) << 12) | ((w) << 18))
const unsigned int configEntryMap[91] =  {
    0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0x7d7200, 0x818240, 0x859280, 0x89a2c0, 0x8db300, 0x91c340, 0x95d380, 0x99e3c0, 
    0,  0x816201, 0x857241, 0x898281, 0x8d92c1, 0x91a301, 0x95b341, 0x99c381, 0x9dd3c1, 
    0,  0x855202, 0x896242, 0x8d7282, 0x9182c2, 0x959302, 0x99a342, 0x9db382, 0xa1c3c2, 
    0,  0x894203, 0x8d5243, 0x916283, 0x9572c3, 0x998303, 0x9d9343, 0xa1a383, 0xa5b3c3, 
    0,  0x8d3204, 0x914244, 0x955284, 0x9962c4, 0x9d7304, 0xa18344, 0xa59384, 0xa9a3c4, 
    0,  0x912205, 0x953245, 0x994285, 0x9d52c5, 0xa16305, 0xa57345, 0xa98385, 0xad93c5, 
    0,  0x951206, 0x992246, 0x9d3286, 0xa142c6, 0xa55306, 0xa96346, 0xad7386, 0xb183c6, 
    0,  0x990207, 0x9d1247, 0xa12287, 0xa532c7, 0xa94307, 0xad5347, 0xb16387, 0xb573c7, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// Config index entry - extract (configEntryMap entry e, i: {0, 1, 2, 3})
#define CIE(e, i) (((e) >> ((i) * 6)) & 63)   // CI entry

// Position on the config index (0-7), row, diags index by col number, col. indexed by row no.
#define CIP(e, i) ((i)==1? ((e) & 63) : ((((e) >> 6) & 63) - 8))

// Map from edge configuration to count of stable disc of an edge w/o corner (e.g. OXXXXXXO)
// For black, flip config (XOR 0xffff) if counting for white.
static unsigned char stableEdgeWoCornerDiscCount[0xaaa6 - 0x9556 + 1];

const int directions[] = {1, -1, 8, -8, 9, -9, 10, -10};
// Indicate towards what directions a disc played into a certain place can make flipped
const uchar dirmask[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  81, 81, 87, 87, 87, 87, 22, 22,
  0,  81, 81, 87, 87, 87, 87, 22, 22,
  0,  121,121,255,255,255,255,182,182,
  0,  121,121,255,255,255,255,182,182,
  0,  121,121,255,255,255,255,182,182,
  0,  121,121,255,255,255,255,182,182,
  0,  41, 41, 171,171,171,171,162,162,
  0,  41, 41, 171,171,171,171,162,162,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// Need to test place i only if libmap[i] < emptyLibmap[i]
const char emptyLibmap[91] = { 
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 5, 5, 5, 5, 5, 5, 3,
    0, 5, 8, 8, 8, 8, 8, 8, 5,
    0, 5, 8, 8, 8, 8, 8, 8, 5,
    0, 5, 8, 8, 8, 8, 8, 8, 5,
    0, 5, 8, 8, 8, 8, 8, 8, 5,
    0, 5, 8, 8, 8, 8, 8, 8, 5,
    0, 5, 8, 8, 8, 8, 8, 8, 5,
    0, 3, 5, 5, 5, 5, 5, 5, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char inCorner[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 0, 0, 0, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 3, 0, 0, 0, 0, 0, 0, 4,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const char inAdjCorner[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 2, 0, 0, 0, 0, 4, 0,
  0, 3, 0, 0, 0, 0, 0, 0, 5,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 6, 0, 0, 0, 0, 0, 0, 8,
  0, 0, 7, 0, 0, 0, 0, 9, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int cornerSquares[5] = {-1, 10, 17, 73, 80};
static const int adjXsquares[5] = {-1, 20, 25, 65, 70};
static const int adjCornerDirs[5][2] = {
    {0, 0}, 
    {1, 9}, {-1, 9}, {-9, 1}, {-9, -1}
};
static const int adjCorner[5][2] = {
    {-1, -1},  // dummy
    {11, 19}, {16, 26}, {64, 74}, {71, 79}
};
static const int adjCornerCorners[5][2] = {
    {-1, -1},
    {17, 73}, {10, 80}, {10, 80}, {17, 73}
};

static const char worst2best[64] =
{
/*B2*/      20 , 25 , 65 , 70 ,
/*B1*/      11 , 16 , 19 , 26 , 64 , 71 , 74 , 79 ,
/*C2*/      21 , 24 , 29 , 34 , 56 , 61 , 66 , 69 ,
/*D2*/      22 , 23 , 38 , 43 , 47 , 52 , 67 , 68 ,
/*D3*/      31 , 32 , 39 , 42 , 48 , 51 , 58 , 59 ,
/*D1*/      13 , 14 , 37 , 44 , 46 , 53 , 76 , 77 ,
/*C3*/      30 , 33 , 57 , 60 ,
/*C1*/      12 , 15 , 28 , 35 , 55 , 62 , 75 , 78 ,
/*A1*/      10 , 17 , 73 , 80 , 
/*D4*/      40 , 41 , 49 , 50
};

///* Fixed ordering of squares */
//static char worst2best64[64] =
//{
///*B2*/       9 ,     14 ,    49 ,    54 ,
///*B1*/       1 ,     6 ,     8 ,     15 ,    48 ,    55 ,    57 ,    62 ,
///*C2*/       10 ,    13 ,    17 ,    22 ,    41 ,    46 ,    50 ,    53 ,
///*D2*/       11 ,    12 ,    25 ,    30 ,    33 ,    38 ,    51 ,    52 ,
///*D3*/       19 ,    20 ,    26 ,    29 ,    34 ,    37 ,    43 ,    44 ,
///*D1*/       3 ,     4 ,     24 ,    31 ,    32 ,    39 ,    59 ,    60 ,
///*C3*/       18 ,    21 ,    42 ,    45 ,
///*C1*/       2 ,     5 ,     16 ,    23 ,    40 ,    47 ,    58 ,    61 ,
///*A1*/       0 ,     7 ,     56 ,    63 ,    
///*D4*/       27 ,    28 ,    35 ,    36 
//};

/* The empty list */
static EmptyList *emHead; // first (and dummy) node in the list
static int emSize;

static EmptyList *cornerNodes[4];
static int nEmptyCorners;
static EmptyList *adjCornerNodes[5][2];
static EmptyList *allNodes[91];

static const double edgeValue[7] = {
  0.0,  // dummy
  UNBALANCED_EDGE_1_COEFF * EDGE_VALUE_NOMALIZATION,
  UNBALANCED_EDGE_2_COEFF * EDGE_VALUE_NOMALIZATION,
  UNBALANCED_EDGE_3_COEFF * EDGE_VALUE_NOMALIZATION,
  UNBALANCED_EDGE_4_COEFF * EDGE_VALUE_NOMALIZATION,
  UNBALANCED_EDGE_5_COEFF * EDGE_VALUE_NOMALIZATION,
  BALANCED_EDGE_6_COEFF * EDGE_VALUE_NOMALIZATION
};


/*** Prototypes for forward reference ***/

/* Now only getMax is used
double 
getMin(char *a, char lastMove, char secondLast, char color, char depth, char passes, 
       char prevmaxDOF, char prevminDOF, //Mobility in previous getMax and getMin;
       char selfPieces, char oppPieces, double alpha, double beta); 
*/
double 
getMax(char *a, int lastMove, int secondLast, int color, int depth, int passes, 
       int prevmaxDOF, int prevminDOF, 
       int selfPieces, int oppPieces, double alpha, double beta);

void copyBoardArray(char *to, char *from);
void copyBoardArrayTo91from64(char *to, char *from);
inline void freeEmptyList(EmptyList *head);
inline bool legalMove(char *a, int color, int place);
inline int findLegalMoves(char *a, int color, EmptyList **legalMoves);
inline int findLegalMoves(char *a, int color);
//char findLegalMovesC(char *a, char color, EmptyList **legalMoves, bool *cornerAvail);
int tryMove(char *a, int color, int place, char *flipped, EmptyList *currNode);
inline void undo(char *a, int color, int place, char *flipped, int nFlipped, EmptyList *currNode);

int evaluateEndGame(int selfPieces, int oppPieces);
int evaluateEndGameWDL(int selfPieces, int oppPieces);
static double 
evaluateBoard(char *a, int forWhom, int whoseTurn, int prevmaxDOF, 
            int prevminDOF, int selfPieces, int oppPieces);

void printBoardArray(char *a);
int strongEndGameSolve(char *a, char color, char selfPieces, char oppPieces, 
                      char prevNotPass, double alpha, double beta);



#if USE_NEW_END_GAME_SOLVER
// For faster exact solve end of game
inline char flipEnd(char *placePointer, char dir, char color, char oppcolor, 
                char *flipped, char cumulativeFlips);
char tryMoveEnd(char *a, char color, char place, char *flipped, EmptyList *currNode);
inline void undo_end(char *a, char color, char place, char *flipped, char nFlipped,
                    EmptyList *currNode);
char getExactMove(char *a, char color, char selfPieces, char oppPieces, EmptyList **legalMoves,
                  char nLegalMoves);
double getMin_exact(char *a, char lastMove, char color, char selfPieces, char oppPieces, 
                  double alpha, double beta);
double getMax_exact(char *a, char lastMove, char color, char selfPieces, char oppPieces, 
                  double alpha, double beta);
inline int evaluateEndGame_exact(char selfPieces, char oppPieces);
#endif

inline static void createEmptyList(char *a);
static void initDiscWeight();
inline static double 
  getStableDiscScore(char *a, char color, int corner, int dir1, int dir2, double weight);
inline static double getEdgeValue(char *a, char color, int corner, int dir);
static int getSecondMove(char *a, int firstMove);
inline static int endSolve(char *a, int lastMove, char color, int empties, int discDiff, 
                          double alpha, double beta);
static bool hasCornerMoves(char *a, char color);
void initStableEdgeWoCornerDiscCount();

void initConfigIndices(char *a);
inline void updateConfigIndicesFlip(int place);
inline void updateConfigIndicesPlace(int place, int color);
inline void updateConfigIndicesRemove(int place);
void updateAllIndicesMove(char *a, int color, int place, char *flipped, int flipCount);
void updateAllIndicesUndo(char *a, int color, int place, char *flipped, int nFlipped);



/* Return a move using minimax search. -- Called only once per move made */
char getMinimaxMove(Board *board, bool *lm64) {
  /* Initialization */
  countPruning = countSearching = countEval = 0;
  base = board->n;
  char lastMove = board->moves[(int)board->m];
  const char depth = 0; // constant within this function
  const char passes = 0;
  const char color = board->wt;
  double currValue;
  double bestValue = 2* SMALL_FLOAT; // avoid return a PASS while there is still legal moves
  char bestMove = PASS;
  EmptyList *bestNode = emHead;
  double alpha = SMALL_FLOAT;
  double beta = LARGE_FLOAT;
  useEndGameSolver = false;
  // stats -- init
  evaluationValue = 999;
  evaluationValueExact = 999;
  WDLresponse = 999;
  // init stable edge (w/o corner) disc count table
  static bool stableEdgeWoCornerDiscCountInitialized = false;
  if(!stableEdgeWoCornerDiscCountInitialized) {
    initStableEdgeWoCornerDiscCount();
    stableEdgeWoCornerDiscCountInitialized = true;
  }
  
  /* If end of game is within the brute force depth, search all the way to the end. */
  if(base + 4 + bruteForceDepth >= 64) {
    searchDepth = 64;
    if(USE_NEW_END_GAME_SOLVER && 64 - base >= MIN_EMPTIES_FOR_END_GAME_SOLVER)
      useEndGameSolver = true;
    inEndGame = true;
  }
  else {
    searchDepth = originalSearchDepth;
    if(extraPlies && base >= EXTRA_PLIES_START && base + searchDepth < 64 - 8)
      searchDepth += 2;  // some experiment
    inEndGame = false;
    if(APPLY_SEARCH_LIMIT_TO_MID_GAME) {  
      /* The following condition is added, because the current mid-game evaluation function
        works pretty awfully when there are very few empty squares left. 
        It should be removed once the evaluation function is able to evalute this kind of
        situation reasonably well */
      while(searchDepth > 4 && base + 4 + searchDepth > 64 - MIN_EMPTIES_FOR_MID_GAME_EVAL)
        searchDepth -= 2;
    }
  }
  if(base + searchDepth >= QUIESCENCE_START) {
    quiescenceSearch = DO_USE_QUIESCENCE_SEARCH;
  }
  else {
    quiescenceSearch = false;
  }
  initDiscWeight();
  /* extra compensation if search is very shallow */
  extra = DENOMINATOR_EXTRA_DOF;
  near2cornerDiagCoeff = 1 * NEAR_TO_CORNER_NEG_DIAG_COEFF;
  if(searchDepth <= 4)
    near2cornerDiagCoeff = 4 * NEAR_TO_CORNER_NEG_DIAG_COEFF;
  else if(searchDepth <= 6)
    near2cornerDiagCoeff = 2 * NEAR_TO_CORNER_NEG_DIAG_COEFF;
  else if(searchDepth <= 8)
    near2cornerDiagCoeff = 1 * NEAR_TO_CORNER_NEG_DIAG_COEFF;
  else if(searchDepth <= 10)
    near2cornerDiagCoeff = 1 * NEAR_TO_CORNER_NEG_DIAG_COEFF;
  
  // corner weight
  //cornerValue = ESTIMATED_CORNER_WORTH;
  // === Very ad hoc, random experiment ===
  /* The rationale is that deeper search should have detected the strategic 
    value of corners. Therefore the extra corner value should be reduced to 
    avoid over-counting mistake. (Very experimental, not known if good or not.) 
    */
  const double cCoeff = 1 / 1.5;
  switch(searchDepth) { 
    case 1: case 2: case 3: case 4:
      cornerValue = ESTIMATED_CORNER_WORTH * 1/cCoeff;
      break;
    case 5: case 6:
      cornerValue = ESTIMATED_CORNER_WORTH;
      break;
    case 7: case 8:
      cornerValue = ESTIMATED_CORNER_WORTH * cCoeff;
      break;
    case 9: case 10:
      cornerValue = ESTIMATED_CORNER_WORTH * cCoeff * cCoeff;
      break;
    default: // >= 11
      cornerValue = ESTIMATED_CORNER_WORTH * cCoeff * cCoeff * cCoeff;
  }
  
  // initialize the arrays to the board configuration.
  char *a = board->a[base];
  char b[91];
  copyBoardArrayTo91from64(b, a);
  // Create the empty list and related information
  createEmptyList(b);
  // Initialize the liberty map
  initLibmap(b);
  initConfigIndices(b);
  // get disc counts
  char selfPieces, oppPieces;
  if(color == BLACK)
    countPieces(a, &selfPieces, &oppPieces, base+depth+4);
  else
    countPieces(a, &oppPieces, &selfPieces, base+depth+4);
  // Find all legal moves
  EmptyList **legalMoves, *legalMoves1[64];
  char flipped[20], nFlips;
  char nLegalMoves = findLegalMoves(b, color, legalMoves1);
  int orignLegalMoves = nLegalMoves;  // in case needed after MPC
  legalMoves = legalMoves1;
  if(LEGAL_MOVES_DEBUG) {
    printf("legalMoves for color %s:\n", color==BLACK? "black" : "white");
    for(int i=0; i<nLegalMoves; i++) {
      char sq64 = CONV_91_64(legalMoves[i]->square);
      char mv[3];
      mv[0] = 'a' + (sq64 & 7);
      mv[1] = '1' + (sq64 >> 3);
      mv[2] = 0;
      printf("%s, ", mv);
    }
    printf("\n");
  }
  if(base == 0) {  // don't want to search for the first move.
    char result = HARD_WIRED_FIRST_MOVE;
    if(!randomnessLevel) {  
      if(boardFlipped)
        result = HARD_WIRED_FIRST_MOVE_BOARD_FLIPPED;
      result = HARD_WIRED_FIRST_MOVE;
    }
    else {
      char sq = legalMoves[rand()%nLegalMoves]->square;
      result = CONV_91_64(sq);
    }
    freeEmptyList(emHead);
    return result;
  }
  else if(base == 1) { 
    // kind of choosing between diagonal opening and perpendicular
    char result = getSecondMove(a, board->moves[1]);
    freeEmptyList(emHead);
    return result;
  }
  /* Shallow search for move ordering and maybe MPC cutoff */
  char remainingDepth = searchDepth - depth;
  // may need to experiment different searchDepth - depth threshhold values.
  if(USE_MOVE_ORDERING && remainingDepth >= MIN_DEPTH_FOR_ORDERING && 
      base + depth <= 64 - MIN_EMPTIES_FOR_ORDERING) {
    // back-up variables to be changed in the shallow search.
    double oldAlpha = alpha, oldBeta = beta;
    char oldSearchDepth = searchDepth;
    bool oldUseEndGameSolver = useEndGameSolver;
    // change some variables
    useEndGameSolver = false;
    char orderingSearchDepth = remainingDepth - ORDERING_SEARCH_DEPTH_OFFSET;
    orderingSearchDepth = orderingSearchDepth < MAX_ORDERING_SEARCH_DEPTH? 
              orderingSearchDepth : MAX_ORDERING_SEARCH_DEPTH;  // shallow search, with MAX
    char upperBound = emSize - MIN_EMPTIES_FOR_EVALUATION;
    if(orderingSearchDepth > upperBound)
      orderingSearchDepth = upperBound;
    searchDepth = depth + orderingSearchDepth;
    EmptyList *orderedMoves[64];
    double orderedMoveValues[64];
    int index;
    for(int i=0; i<nLegalMoves; i++) {
      EmptyList *currNode = legalMoves[i];
      char place = currNode->square;
      nFlips = tryMove(b, color, place, flipped, currNode);
      updateAllIndicesMove(b, color, place, flipped, nFlips);
      currValue = - getMax(b, place, lastMove, OTHER(color), depth+1, passes, nLegalMoves, 
                        nLegalMoves, oppPieces-nFlips, selfPieces+nFlips+1, -beta, -alpha);
      if(DEBUG_MINIMAX) {
        printf("getMin returned: %e, (depth: %d, in ordering search)\n", currValue, depth+1);
      }
      assert(nFlips > 0);
      updateAllIndicesUndo(b, color, place, flipped, nFlips);
      undo(b, color, place, flipped, nFlips, currNode);
      if(currValue > bestValue) {
        bestValue = currValue;
      }
      if(bestValue > alpha) {
        //alpha = bestValue; // shouldn't use a-b cut here, need accurate value for ordering
      }
      // keep the order, best to worst
      index = 0;
      while(index < i && currValue <= orderedMoveValues[index]) 
        index++;
      // insert this move, shift everything up
      for(int j=i; j>index; j--) {
        orderedMoves[j] = orderedMoves[j-1];
        orderedMoveValues[j] = orderedMoveValues[j-1];
      }
      orderedMoves[index] = currNode;
      orderedMoveValues[index] = currValue;
    }
    /* Possibly use MPC */
    if(USE_MPC && !inEndGame && orderingSearchDepth >= MIN_DEPTH_FOR_MPC && 
        nLegalMoves >= MIN_LEGAL_MOVES_FOR_MPC && emSize >= MIN_EMPTIES_FOR_MPC) {
      double topValue = orderedMoveValues[0];
      if(topValue >= TOP_VALUE_LOWER_BOUND && topValue <= TOP_VALUE_UPPER_BOUND) {  
        double threshold = ((double)MPC_THRESHOLD) / orderingSearchDepth;
        int j = 1;
        while(j < nLegalMoves && topValue - orderedMoveValues[j] <= threshold)
          j++;
        j = MAX(j, MIN(nLegalMoves, MPC_BASE_MIN_MOVES 
                                - (orderingSearchDepth >> 1))); 
        if(MPC_FEEDBACK) {
          printf("Ordered moves:\n");
          for(int k=0; k<nLegalMoves; k++) {
            char mv[3];
            mv[0] = 'a' + (CONV_91_64(orderedMoves[k]->square) & 7);
            mv[1] = '1' + (CONV_91_64(orderedMoves[k]->square) >> 3);
            mv[2] = 0;
            printf("%s, %f\n", mv, orderedMoveValues[k]);
          }
          printf("Cut off: ");
          for(int k=j; k<nLegalMoves; k++) {
            char mv[3];
            mv[0] = 'a' + (CONV_91_64(orderedMoves[k]->square) & 7);
            mv[1] = '1' + (CONV_91_64(orderedMoves[k]->square) >> 3);
            mv[2] = 0;
            if(k == nLegalMoves - 1)
              printf("%s", mv);
            else
              printf("%s, ", mv);
          }
          printf("\n");
        }
        nLegalMoves = j;  // i.e. low-valued moves are cut off.
      }
    }
    // restore values changed in the ordering search.
    alpha = oldAlpha;
    beta = oldBeta;
    useEndGameSolver = oldUseEndGameSolver;
    searchDepth = oldSearchDepth; 
    bestValue = 2 * SMALL_FLOAT; // avoid return a PASS while there is still legal moves
    // update the legalMoves array (now point to the ordered moves).
    legalMoves = orderedMoves;
  }
  /* try the ordered legal moves */
  int finalMoves[64];
  double finalEvals[64];
  if(useEndGameSolver && emSize <= MAX_EMPTIES_FOR_END_GAME_SOLVER) {
    alpha = -64;
    beta = 64;
  }
  for(int i=0; i<nLegalMoves; i++) {
    EmptyList *currNode = legalMoves[i];
    char place = currNode->square;
    nFlips = tryMove(b, color, place, flipped, currNode);
    updateAllIndicesMove(b, color, place, flipped, nFlips);
    if(showDots) { // show progress
      printf(".");
      fflush(stdout);
    }
    //printf("trying move %c%c\n", place%9-1+'a', place/9-1+'1');
    if(base + depth + 4 == 63) {  // just filled the board.
      currValue = evaluateEndGame(selfPieces+nFlips+1, oppPieces-nFlips);
    }
    else if(base + 4 + bruteForceDepth >= 64) {
      if(ANDERSSON_ENABLED && useAndersson) {
        if(winLarge) {
          currValue = strongEndGameSolve(b, OTHER(color), selfPieces+nFlips+1, 
                                        oppPieces-nFlips, 1, alpha, beta);
        }
        else if(loseSmall) {
          currValue = strongEndGameSolve(b, OTHER(color), selfPieces+nFlips+1, 
                                        oppPieces-nFlips, 1, alpha, 1);
        }
        else {
          double lowerBound = alpha > -1? alpha : -1;
          currValue = strongEndGameSolve(b, OTHER(color), selfPieces+nFlips+1, 
                                        oppPieces-nFlips, 1, lowerBound, 1);
        }
      }
      else if(useEndGameSolver && emSize <= MAX_EMPTIES_FOR_END_GAME_SOLVER) {
        if(winLarge) {
          currValue = - endSolve(b, place, OTHER(color), emSize, 
                              oppPieces-selfPieces-2*nFlips-1, -beta, -alpha);
        }
        else if(loseSmall) {
          currValue = - endSolve(b, place, OTHER(color), emSize, 
                              oppPieces-selfPieces-2*nFlips-1, -1, -alpha);
        }
        else {
          double lowerBound = alpha > -1? alpha : -1;
          currValue = - endSolve(b, place, OTHER(color), emSize, 
                              oppPieces-selfPieces-2*nFlips-1, -1, -lowerBound);
        }
      }
      else { // own end of game search
        if(winLarge) {
          currValue = - getMax(b, place, lastMove, OTHER(color), depth+1, 0, nLegalMoves, 
                          nLegalMoves, oppPieces-nFlips, selfPieces+nFlips+1, -beta, -alpha);
        }
        else if(loseSmall) {
          currValue = - getMax(b, place, lastMove, OTHER(color), depth+1, 0, nLegalMoves, 
                          nLegalMoves, oppPieces-nFlips, selfPieces+nFlips+1, -1, -alpha);
        }
        else {
          double lowerBound = alpha > -1? alpha : -1;
          currValue = - getMax(b, place, lastMove, OTHER(color), depth+1, 0, nLegalMoves, 
                          nLegalMoves, selfPieces+nFlips+1, oppPieces-nFlips, 
                          -1, -lowerBound);
        }
      }
    }
    else {
      if(searchDepth == 1 && !(hasCornerMoves(b, OTHER(color))))
        currValue = - evaluateBoard(b, OTHER(color), OTHER(color), 
                          nLegalMoves, nLegalMoves, 
                          oppPieces-nFlips, selfPieces+nFlips+1);
      else
        currValue = - getMax(b, place, lastMove, OTHER(color), depth+1, 0, nLegalMoves, 
                        nLegalMoves, oppPieces-nFlips, selfPieces+nFlips+1, -beta, -alpha);
    }
    if(DEBUG_MINIMAX || CURRENT_DEBUG2) {
      printf("getMin returned: %f, for move: %d (%c%c) (depth: %d), alpha: %f, beta: %f\n", currValue,
            place, 'a'+(place-10)%9, '1'+place/9-1, depth+1, alpha, beta);
    }
    // keep the order, best to worst, for opening randomness
    if(base <= 2) {  
      int index = 0;
      bool put2end = false;
      int nlm = findLegalMoves(b, OTHER(color));
      if((nlm == 3 && oppPieces-nFlips == 2) || // c6 in after f5-d6
          nlm == 6 ||   // 2nd-line move
          place == 21 || place == 24 || place == 29 || place == 34 ||
          place == 56 || place == 61 || place == 66 || place == 69)
            // don't want the c6 & c7 move in an f5-d6 perpendicular opening
            // or any second-line move for the third move
        put2end = true;
      while(index < i && (currValue <= finalEvals[index] || put2end)) 
        index++;
      // insert this move, shift everything up
      for(int j=i; j>index; j--) {
        finalMoves[j] = finalMoves[j-1];
        finalEvals[j] = finalEvals[j-1];
      }
      finalMoves[index] = place;
      finalEvals[index] = put2end? -1000 : currValue;
    }
    assert(nFlips > 0);
    updateAllIndicesUndo(b, color, place, flipped, nFlips);
    undo(b, color, place, flipped, nFlips, currNode);
    if(currValue > bestValue) {
      bestNode = currNode;
      bestMove = place;
      bestValue = currValue;  
      if(bestValue > alpha) {
        alpha = bestValue;
        if(useEndGameSolver && emSize <= MAX_EMPTIES_FOR_END_GAME_SOLVER) {
          if(!winLarge && alpha >= 1)
            break;
        }
        if(alpha >= beta)
          break;
      }
    }
  }
  
  // randomness in opening (second black move -- first white move is chosen even before)
  if(randomnessLevel && base == 2) {
    int randChosen = rand() % 3;
        // findLegalMoves(b, OTHER(color)) == 4 means perpendicular opening
    if(randomnessLevel >= 8) { // large randomness
      bestMove = finalMoves[randChosen];
      bestValue = finalEvals[randChosen];
    }
    else { // can only do so for perpendicular opening
      if(findLegalMoves(b, OTHER(color)) == 4) {
        bestMove = finalMoves[randChosen];
        bestValue = finalEvals[randChosen];
      }
    }
  }
  
  if(inEndGame && !(ANDERSSON_ENABLED && useAndersson) && !useEndGameSolver) {
    bestValue /= (1 << 10);
  }
  /* Use WDL (win-draw-loss) search to see if the estimated best move is a winning move */
  bool exactValue = false;
  bool knowsWDL = false;
  const char *winDrawLoss = "UNKNOWN";
  double bestMoveWDLvalue = 1;
  if(USE_WDL_SEARCH && base+4+bruteForceDepth < 64 && base+4+bruteForceDepth >= 62 &&
      bestValue >= BEST_VALUE_WDL_THRESHOLD) { 
      // if bestValue is too small, it is most likely a loss anyway.
    knowsWDL = true;
    useEndGameSolver = USE_NEW_END_GAME_SOLVER;
    nFlips = tryMove(b, color, bestMove, flipped, bestNode);
    updateAllIndicesMove(b, color, bestMove, flipped, nFlips);
    if(showDots) { // show progress
      printf(".");
      fflush(stdout);
    }
    if(ANDERSSON_ENABLED && useAndersson) {
      bestMoveWDLvalue = strongEndGameSolve(b, OTHER(color), selfPieces+nFlips+1, 
                                      oppPieces-nFlips, 1, -1, 1);
    }
    else if(useEndGameSolver && emSize <= MAX_EMPTIES_FOR_END_GAME_SOLVER) {
      bestMoveWDLvalue = - endSolve(b, bestMove, OTHER(color), emSize, 
                            oppPieces-selfPieces-2*nFlips-1, -1, 1);
    }
    else {
      searchDepth = 64;
      bestMoveWDLvalue = - getMax(b, bestMove, lastMove, OTHER(color), depth+1, 0, 
                nLegalMoves, nLegalMoves, oppPieces-nFlips, selfPieces+nFlips+1, -1, 1);
    }
    assert(nFlips > 0);
    updateAllIndicesUndo(b, color, bestMove, flipped, nFlips);
    undo(b, color, bestMove, flipped, nFlips, bestNode);
    if(bestMoveWDLvalue > 0) {
      winDrawLoss = "Win";
      if(bestValue <= 0) {
        bestValue = 2.0;
        WDLresponse = 2;
      }
      else WDLresponse = 1;
    }
    //printf("WDL - move: %c%c, value: %f\n", bestMove%9-1+'a', bestMove/9-1+'1', bestMoveWDLvalue); //debug
  }
  /* if estimated best move is not a winning one, do WDL search at 2 steps 
    ahead of End-of-game search */
  if(USE_WDL_SEARCH && base+4+bruteForceDepth < 64 && base+4+bruteForceDepth >= 62 &&
     bestMoveWDLvalue <= 0) {
    /* At this point, the estimated best move is not a winning move */
    /* initiate WDL search to see if a win or draw can be secured */
    searchDepth = 64; // search to the end
    double WDL_value = SMALL_FLOAT;
    char bestWDLmove = -2; // init value
    alpha = -1;
    beta = 1;
    for(int i=0; i<orignLegalMoves; i++) { // want to try every move
      EmptyList *currNode = legalMoves[i];
      char place = currNode->square;
      if(place == bestMove) { // this move already tested
        currValue = bestMoveWDLvalue;
      }
      else {  
        nFlips = tryMove(b, color, place, flipped, currNode);
        updateAllIndicesMove(b, color, place, flipped, nFlips);
        if(showDots) { // show progress
          printf(".");
          fflush(stdout);
        }
        if(ANDERSSON_ENABLED && useAndersson) {
          currValue = strongEndGameSolve(b, OTHER(color), selfPieces+nFlips+1, 
                                          oppPieces-nFlips, 1, alpha, beta);
        }
        else if(useEndGameSolver && emSize <= MAX_EMPTIES_FOR_END_GAME_SOLVER) {
          currValue = - endSolve(b, place, OTHER(color), emSize, 
                                oppPieces-selfPieces-2*nFlips-1, -beta, -alpha);
        }
        else {
          currValue = - getMax(b, place, lastMove, OTHER(color), depth+1, 0, nLegalMoves, 
                        nLegalMoves, oppPieces-nFlips, selfPieces+nFlips+1, -beta, -alpha);
        }
        assert(nFlips > 0);
        updateAllIndicesUndo(b, color, place, flipped, nFlips);
        undo(b, color, place, flipped, nFlips, currNode);
      }
      if(currValue > WDL_value) {
        bestWDLmove = place;
        WDL_value = currValue;  
        if(WDL_value > alpha) {
          alpha = WDL_value;
          if(alpha >= beta)
            break;
        }
      }
      //printf("WDL - currMove: %c%c, value: %f\n", place%9-1+'a', place/9-1+'1', currValue); //debug
    }
    if(!useEndGameSolver && !(ANDERSSON_ENABLED && useAndersson)) {
      WDL_value /= (1 << 10);
    }
    if(WDL_value >= 0) {
      bestMove = bestWDLmove;
      if(WDL_value > 0) {
        bestValue = 2.0;
        winDrawLoss = "Win";
        WDLresponse = 2;
      }
      else {
        bestValue = 0.0;
        exactValue = true;
        winDrawLoss = "Draw";
        WDLresponse = 0;
      }
    }
    else { // losing situation anyway
      winDrawLoss = "Loss";
      if(bestValue >= 0) {  // just for statistical purpose
        bestValue = -2.0;
        WDLresponse = -2;
      }
      else
        WDLresponse = -1;
    }
  }
  assert(searchDepth > 0); // bug detection
  // show some statistics
  if(inEndGame)
    exactValue = true;
  if(showDots && !guiMode)
    printf("\n");
  if(COUNT_PRUNING && showStatistics) {
    if(bestValue >= (1 << 10)) {  // game can be won in mid-game
      bestValue /= (1 << 10);
      evaluationValueExact = (int)bestValue << 10;
      if(bestValue == 64)
        exactValue = true;
    }
    else if(bestValue <= 0 - (1 << 10)) {
      bestValue /= (1 << 10);  // game lost in mid-game
      exactValue = true;
    }
    if(exactValue) {
      if(!guiMode)
        printf("Searched: %d, Evaluated: %d; Best Value: %d", 
                countSearching, countEval, (int)bestValue);
      evaluationValueExact = (int)bestValue;
    }
    else {
      if(!guiMode) {
        if(evaluationValueExact > 999) {
          printf("Searched: %d, Evaluated: %d; Best Value: >= %d", 
                  countSearching, countEval, evaluationValueExact >> 10);
        }
        else {
          printf("Searched: %d, Evaluated: %d; Best Value: %f", 
                  countSearching, countEval, bestValue);
          if(knowsWDL)
            printf(" (%s)", winDrawLoss);
        }
      }
    }
    if(!guiMode)
      printf("\n");
    evaluationValue = bestValue;
  }
  freeEmptyList(emHead);
  return CONV_91_64(bestMove);
}


/***************************************************************
                    The minimax core      
 *****************************************************************
 */
 
// Utility funtion: for checking time cutoff
static inline void checkTime() {
  // Check every TIME_CHECK_INTERVAL moves to see if timer expired
  static int lastCountSearching = 0;
  if(timeLimit != 0 && countSearching - lastCountSearching >= TIME_CHECK_INTERVAL) {
    lastCountSearching = countSearching;
    if((uint)clock() >= timeLimit) { 
      // time expired 
      printf("%d\n", 64);  // 64 - code for incomplete search
      exit(0);
    }
  }
}


/**
  Function: getMax(...)
  ---- The MAX par of Minimax ---- 
  getMax always plays for "self"
*/
double getMax(char *a, int lastMove, int secondLast, int color, int depth, int passes, 
             int prevmaxDOF, int prevminDOF, 
             int selfPieces, int oppPieces,
             double alpha, double beta) {
  if(DEBUG_MINIMAX || AB_DEBUG || CURRENT_DEBUG) {
    printf("In getMax, at depth %d -- alpha: %e, beta: %e\n", depth, alpha, beta);
    // printf("DEBUG_MINIMAX: %d", DEBUG_MINIMAX);
  }
  if(true || (COUNT_PRUNING && showStatistics))
    countSearching++;
  checkTime();
  // Initialization started.
  uchar uPieces = base + depth + 4;
  //printf("uPieces + emSize: %d, uPieces: %d, row 4: 0x%04x\n", uPieces+emSize, uPieces, configIndices[4]);
  assert(uPieces + emSize == 64);  // bug detection
  assert(selfPieces >= 0 && oppPieces >= 0);
  char place;
  char flipped[20], nFlips;
  char otherColor = OTHER(color);
  /* If there is only one move left, simply player it (if possible) and count scores */
  if(uPieces == 63) {
    double score;
    EmptyList *currNode = emHead->next;
    place = currNode->square;
    nFlips = tryMove(a, color, place, flipped, currNode);
    if(nFlips) { // i.e. move can be played (by min) (min == opponent)
      updateAllIndicesMove(a, color, place, flipped, nFlips);
      score = evaluateEndGame(selfPieces+nFlips+1, oppPieces-nFlips);
      updateAllIndicesUndo(a, color, place, flipped, nFlips);
      undo(a, color, place, flipped, nFlips, currNode);
    }
    else { // see if the other player (max) can play that move
      nFlips = tryMove(a, otherColor, place, flipped, currNode);
      if(nFlips) { // i.e. move can be played by the other player
        updateAllIndicesMove(a, otherColor, place, flipped, nFlips);
        score = evaluateEndGame(selfPieces-nFlips, oppPieces+nFlips+1);
        updateAllIndicesUndo(a, otherColor, place, flipped, nFlips);
        undo(a, otherColor, place, flipped, nFlips, currNode);
      }
      else {  // no one can play this move
        score = evaluateEndGame(selfPieces, oppPieces);
      }
    }
    return score;
  }
  /* Prepare for deeper search */
  EmptyList **legalMoves, *legalMoves1[64];
  char nLegalMoves;
  nLegalMoves = findLegalMoves(a, color, legalMoves1);
  legalMoves = legalMoves1;
  if(CURRENT_DEBUG) {
    printf("legalMoves for color %d:\n", color);
    for(int i=0; i<nLegalMoves; i++)
      printf("%d, ", legalMoves[i]->square);
    printf("\n");
  }
  /* test if there is any legal move posssible */
  if(!nLegalMoves) {
    double score;
    if(lastMove == PASS) // last step is also pass, this branch ends here.
      score = evaluateEndGame(selfPieces, oppPieces);
    else // make a pass (set lastx to -1), depth NOT incremented.
      score = - getMax(a, PASS, lastMove, otherColor, depth, 1-passes, prevminDOF, 0, 
                    oppPieces, selfPieces, -beta, -alpha);
    return score;
  }
  /* Now there are legal moves to make */
  double maxValue = SMALL_FLOAT;
  double currValue = SMALL_FLOAT;
  /* Shallow search for move ordering and maybe MPC cutoff */
  char remainingDepth = searchDepth - depth;
  // may need to experiment different searchDepth - depth threshhold values.
  if(USE_MOVE_ORDERING && remainingDepth >= MIN_DEPTH_FOR_ORDERING && 
      base + depth <= 64 - MIN_EMPTIES_FOR_ORDERING) { 
              // at least 10 moves left
    // back-up variables to be changed in the shallow search.
    double oldAlpha = alpha, oldBeta = beta;
    char oldSearchDepth = searchDepth;
    char oldUseEndGameSolver = useEndGameSolver;
    // change some variables
    useEndGameSolver = false;  // don't want that in move ordering part
    char orderingSearchDepth = remainingDepth - ORDERING_SEARCH_DEPTH_OFFSET;
    if(orderingSearchDepth > MAX_ORDERING_SEARCH_DEPTH)
      orderingSearchDepth = MAX_ORDERING_SEARCH_DEPTH;  // shallow search, with MAX
    char upperBound = emSize - MIN_EMPTIES_FOR_EVALUATION;
    if(orderingSearchDepth > upperBound)
      orderingSearchDepth = upperBound;
    searchDepth = depth + orderingSearchDepth;
    EmptyList *orderedMoves[64];
    double orderedMoveValues[64];
    int index;
    for(int i=0; i<nLegalMoves; i++) {
      EmptyList *currNode = legalMoves[i];
      char place = currNode->square;
      nFlips = tryMove(a, color, place, flipped, currNode);
      updateAllIndicesMove(a, color, place, flipped, nFlips);
      // can never reach cut-off depth here.
      if(remainingDepth <= MOB_ORDERING_DEPTH)
        currValue = - findLegalMoves(a, otherColor);
      else if(remainingDepth <= ONE_PLY_ORDERING_DEPTH)
        currValue = - evaluateBoard(a, otherColor, otherColor, prevminDOF, nLegalMoves, 
                                  oppPieces-nFlips, selfPieces+nFlips+1);
      else
        currValue = - getMax(a, place, lastMove, otherColor, depth+1, passes, 
                           prevminDOF, nLegalMoves, oppPieces-nFlips, selfPieces+nFlips+1, 
                           SMALL_FLOAT, LARGE_FLOAT);
      assert(nFlips > 0);
      updateAllIndicesUndo(a, color, place, flipped, nFlips);
      undo(a, color, place, flipped, nFlips, currNode);
      if(currValue > maxValue) {
        maxValue = currValue;  
        if(maxValue > alpha) {
          //alpha = maxValue;
        }
      }
      // keep the order, best to worst (small to large, since in getmin)
      index = 0;
      while(index < i && currValue <= orderedMoveValues[index]) 
        index++;
      // insert this move, shift verything up
      for(int j=i; j>index; j--) {
        orderedMoves[j] = orderedMoves[j-1];
        orderedMoveValues[j] = orderedMoveValues[j-1];
      }
      orderedMoves[index] = currNode;
      orderedMoveValues[index] = currValue;
    }
    /* Possibly use MPC */
    if(USE_MPC && !inEndGame && orderingSearchDepth >= MIN_DEPTH_FOR_MPC &&
        nLegalMoves >= MIN_LEGAL_MOVES_FOR_MPC && emSize >= MIN_EMPTIES_FOR_MPC) {
      double topValue = orderedMoveValues[0];
      if(topValue >= TOP_VALUE_LOWER_BOUND && topValue <= TOP_VALUE_UPPER_BOUND) {    
        double threshold = ((double)MPC_THRESHOLD) / orderingSearchDepth;
        int j = 1;
        while(j < nLegalMoves && topValue - orderedMoveValues[j] <= threshold)
          j++;
        // i.e. low-valued moves are cut off.
        nLegalMoves = MAX(j, MIN(nLegalMoves, MPC_BASE_MIN_MOVES 
                                - (orderingSearchDepth >> 1))); 
      }
    }
    // restore values changed in the ordering search.
    alpha = oldAlpha;
    beta = oldBeta;
    useEndGameSolver = oldUseEndGameSolver;
    searchDepth = oldSearchDepth; 
    maxValue = SMALL_FLOAT;
    // update the legalMoves array (now point to the ordered moves).
    legalMoves = orderedMoves;
  }
  if(AB_DEBUG) {
    printf("After ordering, at depth %d -- alpha: %e, beta: %e\n", depth, alpha, beta);
  }
  
  // Find out if it were otherColor's turn, it could play any of the corners.
  // Needed for quiescence search
  bool cornerAvail[5] = {0, 0, 0, 0, 0};
  if(DO_USE_QUIESCENCE_SEARCH && quiescenceSearch) {
    for(int i=1; i<=4; i++)
      cornerAvail[i] = legalMove(a, otherColor, cornerSquares[i]);      
  }
  
  /* try the ordered legal moves (or unordered moves if ordering didn't happen) */
  for(int i=0; i<nLegalMoves; i++) {
    EmptyList *currNode = legalMoves[i];
    int place = currNode->square;
    nFlips = tryMove(a, color, place, flipped, currNode);
    updateAllIndicesMove(a, color, place, flipped, nFlips);
    /* test if cut-off depth will be reached. If so, avoid an extra recursive call. */
    if(depth + 1 >= searchDepth + passes) { // always evaluate when it is one side's turn
      if(DO_USE_QUIESCENCE_SEARCH && quiescenceSearch) {
        // Quiescence search for corner trade
        int secondLastIndex = inCorner[secondLast];
        int lastMoveIndex = inCorner[lastMove];
        int placeIndex = inCorner[place];
        bool currValueUpdated = false;
        // search for immediate corner moves
        currValue = SMALL_FLOAT;
        if(secondLastIndex) {
          double getMinValue = LARGE_FLOAT;
          bool getMinValueUpdated = false;
          searchDepth += 2;
          char fpd[20];
          for(int j=0; j<nEmptyCorners; j++) {
            EmptyList *currCornerNode = cornerNodes[j];
            if(currCornerNode != NULL) {  
              char currCorner = currCornerNode->square;
              char nfp = tryMove(a, otherColor, currCorner, fpd, currCornerNode);
              if(nfp) {
                updateAllIndicesMove(a, otherColor, currCorner, fpd, nfp);
                double newValue = getMax(a, currCorner, place, color, depth+2, passes, 
                                        nLegalMoves, prevminDOF, selfPieces+nFlips-nfp+1,
                                        oppPieces-nFlips+1+nfp, alpha, beta);
                updateAllIndicesUndo(a, otherColor, currCorner, fpd, nfp);
                undo(a, otherColor, currCorner, fpd, nfp, currCornerNode);
                if(newValue < getMinValue) {
                  getMinValue = newValue;
                  getMinValueUpdated = true;
                }
              }
            }
          }
          searchDepth -= 2;
          if(getMinValueUpdated && getMinValue > currValue) {
            currValue = getMinValue;
            currValueUpdated = true;
          }
        }
        // non-immediate corner moves
        if(placeIndex) {
          double getMinValue = LARGE_FLOAT;
          bool getMinValueUpdated = false;
          char fpd[20];
          double newValue;
          for(int j=0; j<2; j++) {
            int adjCornerPlace = adjCorner[placeIndex][j];
            int dir = adjCornerPlace - place;
            while(a[adjCornerPlace] == otherColor ||
                  a[adjCornerPlace] == color) // for config like .OO.XXX.
              adjCornerPlace += dir;
            if(a[adjCornerPlace] == EMPTY && a[adjCornerPlace+dir] == color) {  
              EmptyList *currAdjCornerNode = allNodes[adjCornerPlace];
              assert(currAdjCornerNode != NULL); // bug detection
              int nfpd = tryMove(a, otherColor, adjCornerPlace, fpd, currAdjCornerNode);
              if(nfpd) {
                updateAllIndicesMove(a, otherColor, adjCornerPlace, fpd, nfpd);
                if(legalMove(a, otherColor, adjCornerCorners[placeIndex][j])) {  
                  searchDepth += 2;
                  //printf("Here, adjCornerPlace: %d\n", adjCornerPlace); // temp debug
                  newValue = getMax(a, adjCornerPlace, place, color, depth+2, passes, 
                                    nLegalMoves, prevminDOF, selfPieces+nFlips-nfpd+1, 
                                    oppPieces-nFlips+nfpd+1, alpha, beta);
                  if(newValue < getMinValue) {
                    getMinValue = newValue;
                    getMinValueUpdated = true;
                  }
                  searchDepth -= 2;
                }
                updateAllIndicesUndo(a, otherColor, adjCornerPlace, fpd, nfpd);
                undo(a, otherColor, adjCornerPlace, fpd, nfpd, currAdjCornerNode);
              }
            }
          }
          if(getMinValueUpdated && getMinValue > currValue) {
            currValue = getMinValue;
            currValueUpdated = true;
          }
        }
        if(lastMoveIndex) {
          // search 2 extra plies to discover a corner trade.
          if(legalMove(a, color, adjCornerCorners[lastMoveIndex][0]) || 
              legalMove(a, color, adjCornerCorners[lastMoveIndex][1])) {
            searchDepth += 2;
            double getMinValue = - getMax(a, place, lastMove, otherColor, depth+1, passes, 
                prevminDOF, nLegalMoves, oppPieces-nFlips, selfPieces+nFlips+1, 
                -beta, -alpha);
            if(getMinValue > currValue) {
              currValue = getMinValue;
              currValueUpdated = true;
            }
            searchDepth -= 2;
          }
        }
        // search for move that leads immediately to new corner moves for opponent
        if(false && !placeIndex) { // TOO TIME CONSUMING
            double getMinValue = LARGE_FLOAT;
            bool getMinValueUpdated = false;
            searchDepth += 2;
            char fpd[20];
            /*
            printf("place: %c%c, lastMove: %c%c, secondLast: %c%c, emSize: %d\n", 'a'+place%9-1, '1'+place/9-1, 
                'a'+lastMove%9-1, '1'+lastMove/9-1, 'a'+secondLast%9-1, '1'+secondLast/9-1, emSize);
            //printf("placeIndex: %d, lastMoveIndex: %d, secondLastIndex: %d\n",
            //        placeIndex, lastMoveIndex, secondLastIndex); //debug
            printf("cornerAvail(prev): [%d %d %d %d]\n", cornerAvail[1], cornerAvail[2], 
                cornerAvail[3], cornerAvail[4]);
            printf("cornerAvail(curr): [%d %d %d %d]\n", legalMove(a, otherColor, cornerSquares[1]), 
                legalMove(a, otherColor, cornerSquares[2]), legalMove(a, otherColor, cornerSquares[3]),
                legalMove(a, otherColor, cornerSquares[4]));
            */
            for(int j=1; j<=4; j++) {
                int currCorner = cornerSquares[j];
                EmptyList *currCornerNode = allNodes[currCorner];
                //printf("  cornerAvail[%d]: %d, legalMove: %d\n", j, cornerAvail[j], 
                //      legalMove(a, otherColor, currCorner));
                if(!cornerAvail[j] &&  // BEFORE 'place' is played
                        legalMove(a, otherColor, currCorner)) {  // AFTER ...
                    //printf("* take j = %d\n", j);
                    int nfp = tryMove(a, otherColor, currCorner, fpd, currCornerNode);
                    updateAllIndicesMove(a, otherColor, currCorner, fpd, nfp);
                    double newValue = getMax(a, currCorner, place, color, depth+2, passes, 
                                        nLegalMoves, prevminDOF, selfPieces+nFlips-nfp+1,
                                        oppPieces-nFlips+1+nfp, alpha, beta);
                    updateAllIndicesUndo(a, otherColor, currCorner, fpd, nfp);
                    undo(a, otherColor, currCorner, fpd, nfp, currCornerNode);
                    if(newValue < getMinValue) {
                      getMinValue = newValue;
                      getMinValueUpdated = true;
                    }
                }
            }
            searchDepth -= 2;
            if(getMinValueUpdated && getMinValue > currValue) {
                currValue = getMinValue;
                currValueUpdated = true;
            }
        }
        // make sure the position gets evaluated somehow.
        if(!currValueUpdated) { // used normal eval. if quiescence search doesn't happen
          currValue = - evaluateBoard(a, otherColor, otherColor, prevminDOF, nLegalMoves, 
                                      oppPieces-nFlips, selfPieces+nFlips+1);
        }
      }
      else {
        currValue = - evaluateBoard(a, otherColor, otherColor, prevminDOF, nLegalMoves, 
                                  oppPieces-nFlips, selfPieces+nFlips+1);
      }
    }
    else {
      if(false && useEndGameSolver && emSize <= MAX_EMPTIES_FOR_END_GAME_SOLVER)
        // only want to call endsolve at top level
        currValue = - endSolve(a, place, otherColor, emSize, oppPieces-selfPieces-2*nFlips-1,
                              -beta, -alpha);
      else
        currValue = - getMax(a, place, lastMove, otherColor, depth+1, passes, 
                        prevminDOF, nLegalMoves, oppPieces-nFlips, selfPieces+nFlips+1, 
                        -beta, -alpha);
    }
    if(DEBUG_MINIMAX || CURRENT_DEBUG) {
      printf("getMin returned: %e, for move: %d. (depth: %d)\n", currValue,
            legalMoves[i]->square, depth+1);
    }
    assert(nFlips > 0);
    updateAllIndicesUndo(a, color, place, flipped, nFlips);
    undo(a, color, place, flipped, nFlips, currNode);
    if(currValue > maxValue) {
      maxValue = currValue;  
      if(maxValue > alpha) {
        alpha = maxValue;  
        if(alpha >= beta) {
          if(COUNT_PRUNING && showStatistics)
            countPruning++;
          return alpha;
        }
      }
    }
  }
  return maxValue;
}

/************************ End of Minimax core ***********************/


/* copy a board array */
void copyBoardArray(char *to, char *from) {
  for(int i=0; i<64; i++) {
    to[i] = from[i];
  }
}

/* copy a 64-square board array to a 91 square one */
void copyBoardArrayTo91from64(char *to, char *from) {
  for(int i=0; i<91; i++)
    to[i] = DUMMY;
  for(int i=0; i<64; i++)
    to[CONV_64_91(i)] = from[i];
}

/* Create empty list and initialize cornerNodes[] and adjCornerNodes[][] */
inline static void createEmptyList(char *a) {
  // Initialize
  emHead = (EmptyList*)malloc(sizeof(EmptyList));
  emHead->prev = NULL;
  emHead->next = NULL;
  emSize = 0;
  for(int i=0; i<4; i++) {
    cornerNodes[i] = NULL;
  }
  for(int i=0; i<5; i++) {
    adjCornerNodes[i][0] = NULL;
    adjCornerNodes[i][1] = NULL;
  }
  for(int i=0; i<91; i++) {
    allNodes[i] = NULL;
  }
  // Build up the empty square linked list
  EmptyList *currNode = emHead;
  nEmptyCorners = 0;
  for(int i=63; i>=0; i--) {
    int place = worst2best[i];
    if(a[place] == EMPTY) {
      EmptyList *newNode = (EmptyList*)malloc(sizeof(EmptyList));
      newNode->square = place;
      newNode->next = NULL;
      newNode->prev = currNode;
      currNode->next = newNode;
      currNode = newNode;
      emSize++;
      allNodes[place] = newNode;
      int adjIndex = inAdjCorner[place];
      if(inCorner[place]) {
        cornerNodes[nEmptyCorners] = newNode;
        nEmptyCorners++;
      }
      else if(adjIndex) {
        adjCornerNodes[adjIndex >> 1][adjIndex & 1] = newNode;
      }
    }
  }
}

/* free the memory taken by the EmptyList */
inline void freeEmptyList(EmptyList *head) {
  EmptyList *currNode = head;
  while(currNode != NULL) {
    EmptyList *nextNode = currNode->next;
    free(currNode);
    currNode = nextNode;
  }
}

/* test if flip in one direction is possible */
/*
inline bool canFlip(char *a, char *placePointer, char dir, char color, char oppcolor) {
  bool result = false;
  char *p = placePointer + dir;
  if(*p == oppcolor) {
    p += dir;
    if(*p == oppcolor) {
      p += dir;
      if(*p == oppcolor) {
        p += dir;
        if(*p == oppcolor) {
          p += dir;
          if(*p == oppcolor) {
            p += dir;
            if(*p == oppcolor) {
              p += dir;
            }
          }
        }
      }
    }
    if(*p == color)
      result = true;
  }
  return result;
}*/

/* test if a move is legal one on a board array for the given color */
inline bool legalMove(char *a, int color, int place) {
    int mobmapOffset = (color - 1) << 3;
    const ushort *mt = mobTableBin;
    const ushort *ci = configIndices;
    uint e = configEntryMap[place];
    //assert(ci[CIE(e, 1)] >= 0 && ci[CIE(e, 1)] < (1 << 16));
    //printf("here\n");
    //printf("mt[1000]: %d\n", mt[0]);
    if(((mt[ci[CIE(e, 1)]] >> mobmapOffset) & (1 << (e & 63))) ||
       ((mt[ci[CIE(e, 0)]] >> mobmapOffset) & (1 << (((e >> 6) & 63) - 8))) ||
       ((mt[ci[CIE(e, 2)]] >> mobmapOffset) & (1 << (((e >> 6) & 63) - 8))) ||
       ((mt[ci[CIE(e, 3)]] >> mobmapOffset) & (1 << (((e >> 6) & 63) - 8))))
        return true;
    return false;
}
/*
bool legalMove(char *a, char color, char place) {
  char *p = a + place; // place pointer
  if(*p != EMPTY)
    return false;
  bool result = false;
  char oppcolor = OTHER(color);
  uchar dirs = dirmask[place];
  if(dirs & 1) {
    result = canFlip(a, p, directions[0], color, oppcolor);
    if(result)
      return result;
  }
  if(dirs & 2) {
    result = canFlip(a, p, directions[1], color, oppcolor);
    if(result)
      return result;
  }
  if(dirs & 4) {
    result = canFlip(a, p, directions[2], color, oppcolor);
    if(result)
      return result;
  }
  if(dirs & 8) {
    result = canFlip(a, p, directions[3], color, oppcolor);
    if(result)
      return result;
  }
  if(dirs & 16) {
    result = canFlip(a, p, directions[4], color, oppcolor);
    if(result)
      return result;
  }
  if(dirs & 32) {
    result = canFlip(a, p, directions[5], color, oppcolor);
    if(result)
      return result;
  }
  if(dirs & 64) {
    result = canFlip(a, p, directions[6], color, oppcolor);
    if(result)
      return result;
  }
  if(dirs & 128) {
    result = canFlip(a, p, directions[7], color, oppcolor);
    if(result)
      return result;
  }
  return result;
}
*/

/* flip all the discs in a certain direction, played at 'place'. return the 
  count of discs flipped. 'flipCount' is used as the entry index for the 
  'flipped' array. */
inline int flip(char *a, char *placePointer, char dir, char color, char oppcolor, 
                char *flipped, char cumulativeFlips) {
  int count = cumulativeFlips;
  char *p = placePointer + dir;
  if(*p == oppcolor) {
    p += dir;
    if(*p == oppcolor) {
      p += dir;
      if(*p == oppcolor) {
        p += dir;
        if(*p == oppcolor) {
          p += dir;
          if(*p == oppcolor) {
            p += dir;
            if(*p == oppcolor) {
              p += dir;
            }
          }
        }
      }
    }
    if(*p == color) {
      p -= dir;
      while(p != placePointer) {
        *p = color;
        flipped[count] = p - a;
        count++;
        p -= dir;
      }
    }
  }
  return count;
}

/* find all the legal moves. Returns the number of legalMoves found */
inline int findLegalMoves(char *a, int color, EmptyList **legalMoves) {
  int count = 0;
  EmptyList *currNode = emHead->next;
  while(currNode != NULL) {
    int place = currNode->square;
    if(libmap[place] < emptyLibmap[place] &&
        legalMove(a, color, place)) {
      legalMoves[count] = currNode;
      count++;
    }
    currNode = currNode->next;
  }
  return count;
}

/* find all the legal moves. Returns the number of legalMoves found.
  Also store in the 5-element map ([0] is dummy) whether each corner is a legal move */
/*char findLegalMovesC(char *a, char color, EmptyList **legalMoves, bool *cornerAvail) {
  char count = 0;
  EmptyList *currNode = emHead->next;
  while(currNode != NULL) {
    char place = currNode->square;
    if(libmap[place] < emptyLibmap[place] &&
        legalMove(a, color, place)) {
      legalMoves[count] = currNode;
      count++;
      cornerAvail[inCorner[place]] = true;
    }
    currNode = currNode->next;
  }
  return count;
}*/

/* find the number of legalmoves without storing them, only want the count */
inline int findLegalMoves(char *a, int color) {
  int count = 0;
  EmptyList *currNode = emHead->next;
  while(currNode != NULL) {
    int place = currNode->square;
    if(libmap[place] < emptyLibmap[place] && 
        legalMove(a, color, place)) {
      count++;
    }
    currNode = currNode->next;
  }
  return count;
}

/* find the number of legalmoves without storing them, only want the count and 
  the corner moves (also find the number of corner moves). */ /*
char findLegalMoves(char *a, char color, EmptyList **cornerMoves, char *nCornerMoves) {
  char count = 0;
  char countCM = 0;
  EmptyList *currNode = emHead->next;
  while(currNode != NULL) {
    char place = currNode->square;
    if(libmap[place] < emptyLibmap[place] && 
        legalMove(a, color, place)) {
      count++;
      if(inCorner[place])
        cornerMoves[countCM++] = currNode;
    }
    currNode = currNode->next;
  }
  *nCornerMoves = countCM;
  return count;
}*/

/* try a move on the minimax's board. Return the number of pieces flipped. 
  in this whole project, tryMove is only called after we know this move to 
  be made is legal. */
int tryMove(char *a, int color, int place, char *flipped, EmptyList *currNode) {
  if(DEBUG_MINIMAX) {
    printf("tryMoveEnd - place: %d, color %d\n", place, color);
  }
  char *p = a + place; // place pointer
  if(*p != EMPTY)
    return 0;
  int flipCount = 0;
  int oppcolor = OTHER(color);
  uchar dirs = dirmask[place];
  if(dirs & 1) {
    flipCount = flip(a, p, directions[0], color, oppcolor, flipped, flipCount);
  }
  if(dirs & 2) {
    flipCount = flip(a, p, directions[1], color, oppcolor, flipped, flipCount);
  }
  if(dirs & 4) {
    flipCount = flip(a, p, directions[2], color, oppcolor, flipped, flipCount);
  }
  if(dirs & 8) {
    flipCount = flip(a, p, directions[3], color, oppcolor, flipped, flipCount);
  }
  if(dirs & 16) {
    flipCount = flip(a, p, directions[4], color, oppcolor, flipped, flipCount);
  }
  if(dirs & 32) {
    flipCount = flip(a, p, directions[5], color, oppcolor, flipped, flipCount);
  }
  if(dirs & 64) {
    flipCount = flip(a, p, directions[6], color, oppcolor, flipped, flipCount);
  }
  if(dirs & 128) {
    flipCount = flip(a, p, directions[7], color, oppcolor, flipped, flipCount);
  }
  if(flipCount) {
    *p = color;
    // remove the node from the list
    EmptyList *prev = currNode->prev;
    EmptyList *next = currNode->next;
    prev->next = next;
    if(next != NULL)
      next->prev = prev;
    emSize--;
    /*if(emSize >= MIN_EMPTIES_FOR_MID_GAME_EVAL) { // moved out of tryMove
      updateLibmap(place);
      updateConfigIndicesPlace(place, color);
      for(int i=0; i<flipCount; i++)
        updateConfigIndicesFlip(flipped[i]);
    }*/
  }
  if(DEBUG_MINIMAX) {
    printBoardArray(a);
    printf("flipCount: %d\n", flipCount);
  }
  return flipCount;
}

/* Count the number of discs that can be flipped in a certain direction by playing into
  a certain square but without actually flipping them. */
inline int ctFlip(char *a, char *placePointer, int dir, int color, int oppcolor) {
  char *p = placePointer + dir;
  if(*p == oppcolor) {
    p += dir;
    int count = 1;
    if(*p == oppcolor) {
      p += dir;
      count++;
      if(*p == oppcolor) {
        p += dir;
        count++;
        if(*p == oppcolor) {
          p += dir;
          count++;
          if(*p == oppcolor) {
            p += dir;
            count++;
            if(*p == oppcolor) {
              p += dir;
              count++;
            }
          }
        }
      }
    }
    if(*p == color)
      return count;
  }
  return 0;
}

/* Count the number of discs that will be flipped by playing into a square w/o actually
  flipping them */
int countFlips(char *a, int color, int place) {
  char *p = a + place; // place pointer
  if(*p != EMPTY)
    return 0;
  char oppcolor = OTHER(color);
  uchar dirs = dirmask[place];
  int count = 0;
  if(dirs & 1) {
    count += ctFlip(a, p, directions[0], color, oppcolor);
  }
  if(dirs & 2) {
    count += ctFlip(a, p, directions[1], color, oppcolor);
  }
  if(dirs & 4) {
    count += ctFlip(a, p, directions[2], color, oppcolor);
  }
  if(dirs & 8) {
    count += ctFlip(a, p, directions[3], color, oppcolor);
  }
  if(dirs & 16) {
    count += ctFlip(a, p, directions[4], color, oppcolor);
  }
  if(dirs & 32) {
    count += ctFlip(a, p, directions[5], color, oppcolor);
  }
  if(dirs & 64) {
    count += ctFlip(a, p, directions[6], color, oppcolor);
  }
  if(dirs & 128) {
    count += ctFlip(a, p, directions[7], color, oppcolor);
  }
  return count;
}

/* undo a move previously made by 'color' */
inline void 
undo(char *a, int color, int place, char *flipped, int nFlipped, EmptyList *currNode) {
  a[place] = EMPTY;
  int otherColor = OTHER(color);
  
  /*for(int i=0; i<19; i++) {
    a[flipped[i]] = otherColor;
    if(i >= nFlipped-1)
      break;
  }*/
  // Equivalent to the above loop 
  switch(nFlipped-1) {
    case 18: a[flipped[18]] = otherColor;
    case 17: a[flipped[17]] = otherColor;
    case 16: a[flipped[16]] = otherColor;
    case 15: a[flipped[15]] = otherColor;
    case 14: a[flipped[14]] = otherColor;
    case 13: a[flipped[13]] = otherColor;
    case 12: a[flipped[12]] = otherColor;
    case 11: a[flipped[11]] = otherColor;
    case 10: a[flipped[10]] = otherColor;
    case 9: a[flipped[9]] = otherColor;
    case 8: a[flipped[8]] = otherColor;
    case 7: a[flipped[7]] = otherColor;
    case 6: a[flipped[6]] = otherColor;
    case 5: a[flipped[5]] = otherColor;
    case 4: a[flipped[4]] = otherColor;
    case 3: a[flipped[3]] = otherColor;
    case 2: a[flipped[2]] = otherColor;
    case 1: a[flipped[1]] = otherColor;
    case 0: a[flipped[0]] = otherColor;
  }
  
  /*if(emSize >= MIN_EMPTIES_FOR_MID_GAME_EVAL) { // moved outside of undo
    updateLibmapUndo(place); // reset liberty map
    updateConfigIndicesRemove(place); // reset config. indices
    for(int i=0; i<nFlipped; i++)
      updateConfigIndicesFlip(flipped[i]);
  }*/
  
  // insert this node back to the list
  currNode->prev->next = currNode;
  EmptyList *next = currNode->next;
  if(next != NULL)
    next->prev = currNode;
  assert(nFlipped);
  emSize++;
  if(DEBUG_MINIMAX) {
    printf("undo move: %d\n", place);
    printBoardArray(a);
  }
}


/* Premitive evaluation functions -- not good at all, just to make game going */

/* Evaluate an end-of-game situation (just count) */
inline int evaluateEndGame(int selfPieces, int oppPieces) {
  if(true || (COUNT_PRUNING && showStatistics)) {
    countEval++;
    countSearching++;
  }
  //checkTime();
  int score;
  // make sure a secured win out-weighs any estimated advantage.
  if(selfPieces > oppPieces)
    score = (64 - oppPieces - oppPieces) << 10;
  else if(selfPieces < oppPieces)
    score = (selfPieces + selfPieces - 64) << 10;
  else
    score = 0;
  return score;
}

/* Only ask if it is Win, Draw or Loss, > 0 means win. */
inline int evaluateEndGameWDL(int selfPieces, int oppPieces) {
  if(true || (COUNT_PRUNING && showStatistics)) {
    countSearching++;
    countEval++;
  }
  //checkTime();
  return selfPieces - oppPieces;
}

/* Evaluate an board situation (game still in progress) */
static double 
evaluateBoard(char *a, int forWhom, int whoseTurn, int prevmaxDOF, 
            int prevminDOF, int selfPieces, int oppPieces) {
  if(DEBUG_MINIMAX) {
    printBoardArray(a);
    printf("Depth: %d, disc diff. value: %e\n", selfPieces+oppPieces-base-4, 
            (double)(selfPieces-oppPieces));
  }
  if(true || (COUNT_PRUNING && showStatistics)) {
    countEval++;
    countSearching++;
  }
  //checkTime();
  // One side wins large if the other is anihilated
  if(selfPieces == 0)
    return -64 * (1 << 10);
  else if(oppPieces == 0)
    return 64 * (1 << 10);
  
  char nEmp = 64 - selfPieces - oppPieces;
  
  char selfMobility, oppMobility;
  if(true || USE_CURRENT_MOBILITY_FOR_BOTH_SIDES) {
    selfMobility = findLegalMoves(a, forWhom); // slow steps!
    oppMobility = findLegalMoves(a, OTHER(forWhom));
  }
  else { // not the standard approach - and not compatible with evalEndSolve()
    // in mamimax, always max for self, min for opponent.
    if(forWhom == whoseTurn) {
      selfMobility = findLegalMoves(a, whoseTurn);
      oppMobility = prevminDOF;
    }
    else {
      selfMobility = prevmaxDOF;
      oppMobility = findLegalMoves(a, whoseTurn);
    }
  }
  
//  if(forWhom == whoseTurn)
//    prevmaxDOF = nLegalMoves;
//  else
//    prevminDOF = nLegalMoves;
//  //*/
  
  double result = 0;
  /* First evaluate for mobility */
  if(USE_MOBILITY_RATIO) {  // *** DEFAULT ***
    double mobilityRatio = ((MAX(selfMobility, MIN_MOBILITY_CONSIDERED) + extra) 
                          / (MAX(oppMobility, MIN_MOBILITY_CONSIDERED) + extra));
    result += mobilityRatio - 1 / mobilityRatio;
  }
  else { // use mobility difference
    result += selfMobility - oppMobility;
  }
  
  /* Evaluate for disc difference */
  double discWeightA = discWeightArr[64-selfPieces-oppPieces];
  if(USE_DISC_COUNT) {
    if(USE_DISC_RATIO) { // experimental (result: doesn't look good)
    result += discWeight * ((double)selfPieces/oppPieces - (double)oppPieces/selfPieces);
    }
    else { // *** DEFAULT ***
    int deficit = oppPieces - selfPieces;
    if(deficit > DISC_DIFF_CEILING) {
      deficit = DISC_DIFF_CEILING;
    }
    if(USE_DISC_WEIGHT_ARR)
      result += discWeightA * (- deficit);
      //result += discWeightArr[60-base-originalSearchDepth] * (- deficit);
    else
      result += discWeight * (- deficit); // discWeigh is negative most of the times
    }
  }
  
  /* Evaluate potential mobility */ 
  if(USE_POTENTIAL_MOBILITY) {
//    static FILE *fp = fopen("tempout-pMob.txt", "w");
//    static int count1 = 0, count2 = 0;
//    assert(fp != NULL);
//    if(fp != NULL) {
//        count1 = (3-(forWhom<<1))*(oppPieces-selfPieces);
//        count2 = pMobDiff;
//        fprintf(stdout, "%d\t%d\n", count1, count2);
//    }

    // Attempts -- semi-random
    //double factor = (3-(forWhom<<1)) * pMobDiff * 0.20 >= 10? 10 : (3-(forWhom<<1)) * pMobDiff * 0.20;
    //result += discWeightArr[64-selfPieces-oppPieces] * factor;
  }
  
  /* A little simple account for corners */
  double corner = cornerValue;
  double near2cornerDiag = near2cornerDiagCoeff * nEmp;
  char self = forWhom, opp = OTHER(forWhom);
  double stableWeight = STABLE_DISC_WEIGHT - discWeightA;
  //double stableWeight = STABLE_DISC_WEIGHT_COEFF * nEmp - discWeight;
  
  // Account for corners and near-to-corner squares.
  /* --- Succint loop code, but slightly slower... --- */
/*  for(int i=1; i<=4; i++) { // Interate through four corners
    char cc = cornerSquares[i];
    char ccColor = a[cc];
    const char *adjccDirs = adjCornerDirs[i];
    if(ccColor == self) {
        result += corner;
        if(USE_STABLE_COUNT)
            result += getStableDiscScore(a, self, cc, adjccDirs[0], adjccDirs[1], stableWeight);
    }
    else if(ccColor == opp) {
        result -= corner;
        if(USE_STABLE_COUNT)
            result -= getStableDiscScore(a, opp, cc, adjccDirs[0], adjccDirs[1], stableWeight);
    }
    else {  // this corner is still empty
        char near0 = a[adjXsquares[i]];
        if(near0 == self)
            result -= near2cornerDiag;
        else if(near0 == opp)
            result += near2cornerDiag;
        if(USE_EDGE_VALUE) {
            for(int j=0; j<2; j++) {
                if(a[adjCornerCorners[i][j]] == EMPTY) {
                    char near1 = a[adjCorner[i][j]];
                    if(near1 == self)
                        result += getEdgeValue(a, self, cc, adjccDirs[j]);
                    else if(near1 == opp)
                        result -= getEdgeValue(a, opp, cc, adjccDirs[j]);
                }
            }
        }
    }
  } */
  /* ----------------------- */
  
  // --- lengthy loop-unrolled code eq. to the previous commented-out block
  //  -- slightly faster ---
  char c1 = a[CORNER_1], c2 = a[CORNER_2], c3 = a[CORNER_3], c4 = a[CORNER_4];
  // upper-left 
  if(c1 == self) {
    result += corner;
    if(USE_STABLE_COUNT) {
      result += getStableDiscScore(a, self, CORNER_1, 1, 9, stableWeight);
    }
  }
  else if(c1 == opp) {
    result -= corner;
    if(USE_STABLE_COUNT) {
      result -= getStableDiscScore(a, opp, CORNER_1, 1, 9, stableWeight);
    }
  }
  else { // this corner is still empty
    char near0 = a[20];
    if(near0 == self)
      result -= near2cornerDiag;
    else if(near0 == opp)
      result += near2cornerDiag;
    if(USE_EDGE_VALUE) {  
      if(c2 == EMPTY) {
        char near1 = a[11];
        if(near1 == self)
          result += getEdgeValue(a, self, CORNER_1, 1);
        else if(near1 == opp)
          result -= getEdgeValue(a, opp, CORNER_1, 1);
      }
      if(c3 == EMPTY) {
        char near2 = a[19];
        if(near2 == self)
          result += getEdgeValue(a, self, CORNER_1, 9);
        else if(near2 == opp)
          result -= getEdgeValue(a, opp, CORNER_1, 9);
      }
    }
  }
  // upper-right 
  if(c2 == self) {
    result += corner;
    if(USE_STABLE_COUNT) {
      result += getStableDiscScore(a, self, CORNER_2, -1, 9, stableWeight);
    }
  }
  else if(c2 == opp) {
    result -= corner;
    if(USE_STABLE_COUNT) {
      result -= getStableDiscScore(a, opp, CORNER_2, -1, 9, stableWeight);
    }
  }
  else { // this corner is still empty
    char near0 = a[25];
    if(near0 == self)
      result -= near2cornerDiag;
    else if(near0 == opp)
      result += near2cornerDiag;
    if(USE_EDGE_VALUE) {
      if(c1 == EMPTY) {
        char near1 = a[16];
        if(near1 == self)
          result += getEdgeValue(a, self, CORNER_2, -1);
        else if(near1 == opp)
          result -= getEdgeValue(a, opp, CORNER_2, -1);
      }
      if(c4 == EMPTY) {
        char near2 = a[26];
        if(near2 == self)
          result += getEdgeValue(a, self, CORNER_2, 9);
        else if(near2 == opp)
          result -= getEdgeValue(a, opp, CORNER_2, 9);
      }
    }
  }
  // lower-left 
  if(c3 == self) {
    result += corner;
    if(USE_STABLE_COUNT) {
      result += getStableDiscScore(a, self, CORNER_3, 1, -9, stableWeight);
    }
  }
  else if(c3 == opp) {
    result -= corner;
    if(USE_STABLE_COUNT) {
      result -= getStableDiscScore(a, opp, CORNER_3, 1, -9, stableWeight);
    }
  }
  else { // this corner is still empty
    char near0 = a[65];
    if(near0 == self)
      result -= near2cornerDiag;
    else if(near0 == opp)
      result += near2cornerDiag;
    if(USE_EDGE_VALUE) {  
      if(c1 == EMPTY) {
        char near1 = a[64];
        if(near1 == self)
          result += getEdgeValue(a, self, CORNER_3, -9);
        else if(near1 == opp)
          result -= getEdgeValue(a, opp, CORNER_3, -9);
      }
      if(c4 == EMPTY) {
        char near2 = a[74];
        if(near2 == self)
          result += getEdgeValue(a, self, CORNER_3, 1);
        else if(near2 == opp)
          result -= getEdgeValue(a, opp, CORNER_3, 1);
      }
    }
  }
  // lower-right 
  if(c4 == self) {
    result += corner;
    if(USE_STABLE_COUNT) {
      result += getStableDiscScore(a, self, CORNER_4, -1, -9, stableWeight);
    }
  }
  else if(c4 == opp) {
    result -= corner;
    if(USE_STABLE_COUNT) {
      result -= getStableDiscScore(a, opp, CORNER_4, -1, -9, stableWeight);
    }
  }
  else { // this corner is still empty
    char near0 = a[70];
    if(near0 == self)
      result -= near2cornerDiag;
    else if(near0 == opp)
      result += near2cornerDiag;
    if(USE_EDGE_VALUE) {  
      if(c2 == EMPTY) {
        char near1 = a[71];
        if(near1 == self)
          result += getEdgeValue(a, self, CORNER_4, -9);
        else if(near1 == opp)
          result -= getEdgeValue(a, opp, CORNER_4, -9);
      }
      if(c3 == EMPTY) {
        char near2 = a[79];
        if(near2 == self)
          result += getEdgeValue(a, self, CORNER_4, -1);
        else if(near2 == opp)
          result -= getEdgeValue(a, opp, CORNER_4, -1);
      }
    }
  }
  
  // Account for stable edges w/o corners
  int stableEdgeGain = 0;
  int edgeConfig;
  unsigned char *setab = stableEdgeWoCornerDiscCount;
  // top edge
  if(c1 == WHITE && c2 == WHITE) {
    edgeConfig = configIndices[0];
    edgeConfig -= 0x9556;
    if(edgeConfig >= 0 && edgeConfig <= 0xaaa6 - 0x9556)
      stableEdgeGain += self == BLACK? setab[edgeConfig] : - setab[edgeConfig];
  }
  else if(c1 == BLACK && c2 == BLACK) {
    edgeConfig = (unsigned short)(configIndices[0] ^ 0xffff);
    edgeConfig -= 0x9556;
    if(edgeConfig >= 0 && edgeConfig <= 0xaaa6 - 0x9556)
      stableEdgeGain += self == BLACK? - setab[edgeConfig] : setab[edgeConfig];  
  }
  // bottom edge
  if(c3 == WHITE && c4 == WHITE) {
    edgeConfig = configIndices[7];
    edgeConfig -= 0x9556;
    if(edgeConfig >= 0 && edgeConfig <= 0xaaa6 - 0x9556)
      stableEdgeGain += self == BLACK? setab[edgeConfig] : - setab[edgeConfig];
  }
  else if(c3 == BLACK && c4 == BLACK) {
    edgeConfig = (unsigned short)(configIndices[7] ^ 0xffff);
    edgeConfig -= 0x9556;
    if(edgeConfig >= 0 && edgeConfig <= 0xaaa6 - 0x9556)
      stableEdgeGain += self == BLACK? - setab[edgeConfig] : setab[edgeConfig];  
  }
  // left edge
  if(c1 == WHITE && c3 == WHITE) {
    edgeConfig = configIndices[8];
    edgeConfig -= 0x9556;
    if(edgeConfig >= 0 && edgeConfig <= 0xaaa6 - 0x9556)
      stableEdgeGain += self == BLACK? setab[edgeConfig] : - setab[edgeConfig];
  }
  else if(c1 == BLACK && c3 == BLACK) {
    edgeConfig = (unsigned short)(configIndices[8] ^ 0xffff);
    edgeConfig -= 0x9556;
    if(edgeConfig >= 0 && edgeConfig <= 0xaaa6 - 0x9556)
      stableEdgeGain += self == BLACK? - setab[edgeConfig] : setab[edgeConfig];  
  }
  // right edge
  if(c2 == WHITE && c4 == WHITE) {
    edgeConfig = configIndices[15];
    edgeConfig -= 0x9556;
    if(edgeConfig >= 0 && edgeConfig <= 0xaaa6 - 0x9556)
      stableEdgeGain += self == BLACK? setab[edgeConfig] : - setab[edgeConfig];
  }
  else if(c2 == BLACK && c4 == BLACK) {
    edgeConfig = (unsigned short)(configIndices[15] ^ 0xffff);
    edgeConfig -= 0x9556;
    if(edgeConfig >= 0 && edgeConfig <= 0xaaa6 - 0x9556)
      stableEdgeGain += self == BLACK? - setab[edgeConfig] : setab[edgeConfig];  
  }
  // update result with weight
  result += stableEdgeGain * stableWeight;
  

  /* A bit account for whose turn it is */
  if(forWhom == whoseTurn)
    result -= MOVE_SIDE_SURPLUS;
  else
    result += MOVE_SIDE_SURPLUS;
  return result;
}

/* Print out the board array (64-square) -- for debugging purpose */
void printBoardArray(char *a) {
  int place;
  printf("\n   0 1 2 3 4 5 6 7\n");
  for(int y=0; y<8; y++) {
    printf("%d  ", y);
    for(int x=0; x<8; x++) {
      place = CONV_21(x, y);
      if(a[place] == BLACK)
        printf("X ");
      else if(a[place] == WHITE)
        printf("O ");
      else
        printf(". ");
    }
    printf("\n");
  }
  printf("\n");
}

/* Return the second move -- one way to add a bit diversity to opening w/o book */
// for 64-square board
static int getSecondMove(char *a, int firstMove) {
  char choice[3];
  switch(firstMove) {
    case 37:
      choice[0] = 43;
      choice[1] = 45;
      choice[2] = 29;
      break;
    case 44:
      choice[0] = 29;
      choice[1] = 45;
      choice[2] = 43;
      break;
    case 43:
      choice[0] = 26;
      choice[1] = 42;
      choice[2] = 44;
      break;
    case 34:
      choice[0] = 44;
      choice[1] = 42;
      choice[2] = 26;
      break;
    case 26:
      choice[0] = 20;
      choice[1] = 18;
      choice[2] = 34;
      break;
    case 19:
      choice[0] = 34;
      choice[1] = 18;
      choice[2] = 20;
      break;
    case 20:
      choice[0] = 37;
      choice[1] = 21;
      choice[2] = 19;
      break;
    case 29:
      choice[0] = 19;
      choice[1] = 21;
      choice[2] = 37;
      break;
  }
  int result;
  int index = 0;  // perpendicular opening preferred
  if(randomnessLevel) {
    evaluationValue = 0.0;
    if(randomnessLevel < 8) {
      index = rand() & 1;
    }
    else {
      index = rand() % 3;
      if(index == 2)
        evaluationValue = -0.5;
    }
  }
  result = choice[index];
  if(COUNT_PRUNING && showStatistics && !guiMode) {
    if(index)
      printf("Choosing Diagonal Opening.\n");
    else
      printf("Choosing Perpendicular Opening.\n");
  }
  return result;
}

/* Uses Andersson's complicated end of game solver 
  Here 'color' is the opponent color as viewed by Minimax, but is 'self' 
  as viewed by EndSolve. */
  /* NOTE: In core minimax, alpha is for max and beta for min, but here 
   since EndSolve is actually evaluating for Min but return a positve
   value (and we invert that value as return value), both alpha and beta must
   be swapped and then inverted (0 - ..) before passing to EndSolve. */
int strongEndGameSolve(char *a, char color, char selfPieces, char oppPieces, 
                       char prevNotPass, double alpha, double beta) {
  int result = 0;
#if ANDERSSON_ENABLED
  int nEmpt = 64 - selfPieces - oppPieces;
  int difference = oppPieces - selfPieces;
  int ecolor = color == BLACK? END_BLACK : END_WHITE;
  uchar eboard[91]; // board specified by the end of game solver
  for(int i=0; i<91; i++) {
    if(a[i] == BLACK)
      eboard[i] = END_BLACK;
    else if(a[i] == WHITE)
      eboard[i] = END_WHITE;
    else if(a[i] == EMPTY)
      eboard[i] = END_EMPTY;
    else
      eboard[i] = DUMMY;
  }
  double end_alpha = 0 - beta;
  double end_beta = 0 - alpha;
  PrepareToSolve(eboard);
  result = EndSolve(eboard, end_alpha, end_beta, ecolor, nEmpt, difference, prevNotPass);
#endif
  return - result; // since result is as viewed by opponent.
}

/* Initialize the disc weight (relative to mobility -- staged) */
static void initDiscWeight() {
  int currStage = base + searchDepth;
  if(currStage < STAGE_1_END) {
    discWeight = DISC_WEIGHT_STAGE_1;
  }
  else if(currStage < STAGE_1_2_START) {
    discWeight = DISC_WEIGHT_STAGE_1 + ((DISC_WEIGHT_STAGE_1_2 - DISC_WEIGHT_STAGE_1) *
                  (currStage - STAGE_1_END)) / (STAGE_1_2_START - STAGE_1_END);
  }
  else if(currStage < STAGE_1_2_END) {
    discWeight = DISC_WEIGHT_STAGE_1_2;
  }
  else if(currStage < STAGE_2_START) {
    discWeight = DISC_WEIGHT_STAGE_1 + ((DISC_WEIGHT_STAGE_2 - DISC_WEIGHT_STAGE_1) *
                  (currStage - STAGE_1_END)) / (STAGE_2_START - STAGE_1_END);
  }
  else if(currStage < STAGE_2_END) {
    discWeight = DISC_WEIGHT_STAGE_2;
  }
  else if(currStage < STAGE_3_START) {
    discWeight = DISC_WEIGHT_STAGE_2 + ((DISC_WEIGHT_STAGE_3 - DISC_WEIGHT_STAGE_2) *
                  (currStage - STAGE_2_END)) / (STAGE_3_START - STAGE_2_END);
  }
  else if(currStage < STAGE_3_END) {
    discWeight = DISC_WEIGHT_STAGE_3;
  }
  else if(currStage < STAGE_4_START) {
    discWeight = DISC_WEIGHT_STAGE_3 + ((DISC_WEIGHT_STAGE_4 - DISC_WEIGHT_STAGE_3) *
                  (currStage - STAGE_3_END)) / (STAGE_4_START - STAGE_3_END);
  }
  else {
    discWeight = DISC_WEIGHT_STAGE_4;
  }
  // add some randomness
  if(randomnessLevel >= 2) {
    double randomFactor = ((double)(rand() % 4096) / 4096) - 0.5; // +- 0.5
    double randomCoeff = 1 + randomnessLevel * randomFactor * RANDOM_FACTOR_CONSTANT;
    discWeight *= randomCoeff;
  }
  //discWeight = DISC_WEIGHT_TEST;  // debug / test
  
  //******************
  // The array: another approach, should be more consistent
  for(int i=0; i<STAGE_1_END; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_1;
  for(int i=STAGE_1_END; i<STAGE_1_2_START; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_1 + ((DISC_WEIGHT_STAGE_1_2 - DISC_WEIGHT_STAGE_1) *
                          (i - STAGE_1_END)) / (STAGE_1_2_START - STAGE_1_END);
  for(int i=STAGE_1_2_START; i<=STAGE_1_2_END; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_1_2;
  for(int i=STAGE_1_2_END; i<STAGE_2_START; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_1_2 + ((DISC_WEIGHT_STAGE_2 - DISC_WEIGHT_STAGE_1_2) *
                          (i - STAGE_1_2_END)) / (STAGE_2_START - STAGE_1_2_END);
  for(int i=STAGE_2_START; i<STAGE_2_END; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_2;
  for(int i=STAGE_2_END; i<STAGE_3_START; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_2 + ((DISC_WEIGHT_STAGE_3 - DISC_WEIGHT_STAGE_2) *
                          (i - STAGE_2_END)) / (STAGE_3_START - STAGE_2_END);
  for(int i=STAGE_3_START; i<STAGE_3_END; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_3;
  for(int i=STAGE_3_END; i<STAGE_4_START; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_3 + ((DISC_WEIGHT_STAGE_4 - DISC_WEIGHT_STAGE_3) *
                          (i - STAGE_3_END)) / (STAGE_4_START - STAGE_3_END);
  for(int i=STAGE_4_START; i<STAGE_4_END; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_4;
  for(int i=STAGE_4_END; i<STAGE_5_START; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_4 + ((DISC_WEIGHT_STAGE_5 - DISC_WEIGHT_STAGE_4) *
                          (i - STAGE_4_END)) / (STAGE_5_START - STAGE_4_END);
  for(int i=STAGE_5_START; i<60; i++)
    discWeightArr[60-i] = DISC_WEIGHT_STAGE_5;
    
  // randomness
  if(randomnessLevel >= 2) {
    double randomFactor = ((double)(rand() % 4096) / 4096) - 0.5; // +- 0.5
    double randomCoeff = 1 + randomnessLevel * randomFactor * RANDOM_FACTOR_CONSTANT;
    for(int i=1; i<=60; i++)
      discWeightArr[i] *= randomCoeff;
  }
}

/* Some count (but not accurate) for stable (irreversible) discs.
  corner must be of "color". */
inline static double 
getStableDiscScore(char *a, char color, int corner, int dir1, int dir2, double weight) {
  double result = 0;
  int bound1 = corner + dir1, bound2 = corner + dir2;
  int dir1count = 1, dir2count = 1;
  double edgeStableDiscWeight = weight * EDGE_STABLE_DISCS_WEIGHT_FACTOR;
  double line3StableDiscWeight = weight * THIRD_LINE_STABLE_DISCS_WEIGHT_FACTOR;
  /* 1. Edge */
  // one edge ...
  if(a[bound1] == color) {
    result += edgeStableDiscWeight * 2;
    bound1 += dir1;
    dir1count++;  // becomes 2
    if(a[bound1] == color) {
      result += edgeStableDiscWeight;
      bound1 += dir1;
      dir1count++;  // becomes 3
      if(a[bound1] == color) {
        result += edgeStableDiscWeight;
        bound1 += dir1;
        if(a[bound1] == color) {
          result += edgeStableDiscWeight;
          bound1 += dir1;
          if(a[bound1] == color) {
            result += edgeStableDiscWeight;
            bound1 += dir1;
            if(a[bound1] == color) {
              //result += edgeStableDiscWeight;
              bound1 += dir1;
              if(a[bound1] == color) {
                //result += edgeStableDiscWeight;
                bound1 += dir1;
              }
            }
          }
        }
      }
    }
  }
  // ... the other edge
  if(a[bound2] == color) {
    result += edgeStableDiscWeight * 2;
    bound2 += dir2;
    dir2count++;  // 2
    if(a[bound2] == color) {
      result += edgeStableDiscWeight;
      bound2 += dir2;
      dir2count++;  // 3
      if(a[bound2] == color) {
        result += edgeStableDiscWeight;
        bound2 += dir2;
        if(a[bound2] == color) {
          result += edgeStableDiscWeight;
          bound2 += dir2;
          if(a[bound2] == color) {
            result += edgeStableDiscWeight;
            bound2 += dir2;
            if(a[bound2] == color) {
              //result += edgeStableDiscWeight;
              bound2 += dir2;
              if(a[bound2] == color) {
                //result += edgeStableDiscWeight;
                bound2 += dir2;
              }
            }
          }
        }
      }
    }
  }
  /* 2. the lines above the edges */
  bound1 += dir2 - dir1;
  bound2 += dir1 - dir2;
  int cornerDiag = corner + dir1 + dir2;
  //printf("dir1count: %d, dir2count: %d\n", dir1count, dir2count);
  if(dir1count >= 3 && dir2count >= 3 && a[cornerDiag] == color) {
    result += weight; // cornerDiag itself 
    // line-2
    int i = cornerDiag + dir1;
    int j = cornerDiag + dir2;
    int line2count1 = 2, line2count2 = 2;
    while(i != bound1 && a[i] == color) {
      result += weight;
      i += dir1;
      line2count1++;
    }
    while(j != bound2 && a[j] == color) {
      result += weight;
      j += dir2;
      line2count2++;
    }
    //printf("line2count1: %d, line2count2: %d\n", line2count1, line2count2);
    /* line-3 */
    int diag3 = cornerDiag + dir1 + dir2;
    if(line2count1 >= 4 && line2count2 >= 4 && a[diag3] == color) {
      result += line3StableDiscWeight;
      // line-3
      i += dir2 - dir1;
      j += dir1 - dir2;
      int k1 = diag3 + dir1;
      int k2 = diag3 + dir2;
      while(k1 != i && a[k1] == color) {
        result += line3StableDiscWeight;
        k1 += dir1;
      }
      while(k2 != j && a[k2] == color) {
        result += line3StableDiscWeight;
        k2 += dir2;
      }
    }
  }
  //printf("result: %f, color: %d, corner: %d: emSize: %d\n", result, color, corner, emSize);
  return result;
}

/* Count for balanced/unbalanced edge configurations */
/* Must already know that this corner is empty and the square adjacent to 
  corner in this direction is 'color', and the corner on the other side is
  empty */
inline static double 
getEdgeValue(char *a, char color, int corner, int dir) {
  int count = 1;
  int multiplier = emSize - EMPTIES_OFFSET;
  if(multiplier < 0)
    multiplier = 0;
  char *p = a + corner + dir + dir;
  if(*p == color) {
    count++;
    p += dir;
    if(*p == color) {
      p += dir;
      count++;
      if(*p == color) {
        p += dir;
        count++;
        if(*p == color) {  
          p += dir;
          count++;
          if(*p == color) {
            count++;
          }
        }
      }
    }
  }
  else if (*p == EMPTY) {
    p += dir;
    if(*p == EMPTY) {  
      p += dir;
      if(*p != color) {  
        return SINGLE_DISC_NEAR_CORNER_COEFF * EDGE_VALUE_NOMALIZATION * multiplier;
      }
    }
    else if(*p != color) { // i.e. other color
      p += dir;
      if(*p == EMPTY) {
        p += dir;
        if(*p != color) { // dangerous .?..X.O. or .?X.X.O.
          return edgeValue[1] * multiplier * 1.5;
        }
      }
    }
  }
  return edgeValue[count] * multiplier;
}

// Initialize count for stable edge discs w/o corners
void initStableEdgeWoCornerDiscCount() {
  memset(stableEdgeWoCornerDiscCount, 0, 0xaaa6 + 1 - 0x9556);
  stableEdgeWoCornerDiscCount[0x9556 - 0x9556] = 6;  // OXXXXXXO
  stableEdgeWoCornerDiscCount[0xa556 - 0x9556] = 5;  // OOXXXXXO
  stableEdgeWoCornerDiscCount[0x955a - 0x9556] = 5;  // OXXXXXOO
  stableEdgeWoCornerDiscCount[0xa956 - 0x9556] = 4;  // OOOXXXXO
  stableEdgeWoCornerDiscCount[0xa55a - 0x9556] = 4;  // OOXXXXOO
  stableEdgeWoCornerDiscCount[0x956a - 0x9556] = 4;  // OXXXXOOO
  stableEdgeWoCornerDiscCount[0xaa56 - 0x9556] = 3;  // OOOOXXXO
  stableEdgeWoCornerDiscCount[0xa95a - 0x9556] = 3;  // OOOXXXOO
  stableEdgeWoCornerDiscCount[0xa56a - 0x9556] = 3;  // OOXXXOOO
  stableEdgeWoCornerDiscCount[0x95aa - 0x9556] = 3;  // OXXXOOOO
  stableEdgeWoCornerDiscCount[0xaa96 - 0x9556] = 2;  // OOOOOXXO
  stableEdgeWoCornerDiscCount[0xaa5a - 0x9556] = 2;  // OOOOXXOO
  stableEdgeWoCornerDiscCount[0xa96a - 0x9556] = 2;  // OOOXXOOO
  stableEdgeWoCornerDiscCount[0xa5aa - 0x9556] = 2;  // OOXXOOOO
  stableEdgeWoCornerDiscCount[0x96aa - 0x9556] = 2;  // OXXOOOOO
  stableEdgeWoCornerDiscCount[0xaaa6 - 0x9556] = 1;  // OOOOOOXO
  stableEdgeWoCornerDiscCount[0xaa9a - 0x9556] = 1;  // OOOOOXOO
  stableEdgeWoCornerDiscCount[0xaa6a - 0x9556] = 1;  // OOOOXOOO
  stableEdgeWoCornerDiscCount[0xa9aa - 0x9556] = 1;  // OOOXOOOO
  stableEdgeWoCornerDiscCount[0xa6aa - 0x9556] = 1;  // OOXOOOOO
  stableEdgeWoCornerDiscCount[0x9aaa - 0x9556] = 1;  // OXOOOOOO
}

// Acount for stable edge without corners (NOT YET INTEGRATED - should pre-store results in a table)
/*
inline static double
getStableEdgeValue(unsigned short config, const int forWhom, const double weight) {
  // Assume evaluting for BLACK -- flip color if evaluating for white
  double result;
  if(forWhom == WHITE)
    config ^= 0xffff;
  switch(config) {
    case 0x9556:  // OXXXXXXO
      result = weight * 6;
      break;
    case 0xa556:  // OOXXXXXO
    case 0x955a:  // OXXXXXOO
      result = weight * 5;
      break;
    case 0xa55a:  // OOXXXXOO
    case 0x956a:  // OXXXXOOO
    case 0xa956:  // OOOXXXXO
      result = weight * 4;
      break;
    case 0xaa56:  // OOOOXXXO
    case 0xa95a:  // OOOXXXOO
    case 0xa56a:  // OOXXXOOO
    case 0x95aa:  // OXXXOOOO
      result = weight * 3;
      break;
    case 0xaa96:  // OOOOOXXO
    case 0xaa5a:  // OOOOXXOO
    case 0xa96a:  // OOOXXOOO
    case 0xa5aa:  // OOXXOOOO
    case 0x96aa:  // OXXOOOOO
      result = weight * 2;
      break;
    case 0xaaa6:  // OOOOOOXO
    case 0xaa9a:  // OOOOOXOO
    case 0xaa6a:  // OOOOXOOO
    case 0xa9aa:  // OOOXOOOO
    case 0xa6aa:  // OOXOOOOO
    case 0x9aaa:  // OXOOOOOO
      result = weight;
    default:
      result = 0;
  }
  return result;
}
*/


/******* Row/Col/Diag (for mobility) & liberty bookkeeping *******/

// Init configIndices[] with an array ('standard' 91-sq)
void initConfigIndices(char *a) {
    ushort *ci = configIndices;
    // To fill the 'out-of-bound portion of diag index with EMPTY
    for(int i=0; i<47; i++)
        ci[i] = (ushort)0;
    for(int i=10; i<81; i++) {
        uint e = configEntryMap[i];
        if(a[i] == EMPTY || e == 0)
            continue;
        for(int j=0; j<4; j++) {
            int ej = CIE(e, j);
            int pj = CIP(e, j);
            ci[ej] |= a[i] << (pj << 1);  // ... then set it
            assert((ci[ej] >> (pj << 1) & 3) > 0 && (ci[ej] >> (pj << 1) & 3) < 3);
        }
    }
}

// Update the config indices when disc at 'place' is flipped
inline void updateConfigIndicesFlip(const int place) {
    ushort *ci = configIndices;
    const uint e = configEntryMap[place];
    //assert(e != 0);
    // unrolled loop
    // 1
    int y = e & 63;
    ci[CIE(e, 1)] ^= 3 << (y << 1); // flip it BLACK <-> WHITE 
    // 0
    int x = ((e >> 6) & 63) - 8;
    ci[CIE(e, 0)] ^= 3 << (x << 1);
    // 2
    ci[CIE(e, 2)] ^= 3 << (x << 1);
    // 3
    ci[CIE(e, 3)] ^= 3 << (x << 1);
}

// ... when a disc of 'color' is placed at 'place'
inline void updateConfigIndicesPlace(const int place, const int color) {
    ushort *ci = configIndices;
    const uint e = configEntryMap[place];
    assert(e != 0);
    // 1
    int y = e & 63;
    ci[CIE(e, 1)] |= color << (y << 1);
    // 0, 2, 3
    int x = ((e >> 6) & 63) - 8;
    ci[CIE(e, 0)] |= color << (x << 1); // before this, ci[CIE(e, 0)] & color << (x << 1) should be 0
    ci[CIE(e, 2)] |= color << (x << 1);
    ci[CIE(e, 3)] |= color << (x << 1);
}

// ... when a disc is removed from 'place'
inline void updateConfigIndicesRemove(const int place) {
    ushort *ci = configIndices;
    const uint e = configEntryMap[place];
    //assert(e != 0);
    // 1
    int y = e & 63;
    ci[CIE(e, 1)] &= (3 << (y << 1)) ^ 0xffff;
    // 0, 2, 3
    int x = ((e >> 6) & 63) - 8;
    ci[CIE(e, 0)] &= (3 << (x << 1)) ^ 0xffff; 
    ci[CIE(e, 2)] &= (3 << (x << 1)) ^ 0xffff;
    ci[CIE(e, 3)] &= (3 << (x << 1)) ^ 0xffff;
}

/* Update the liberty map after 'move' is played */
inline void updateLibmap(const int move) {
    char *lmp = libmap;
    /*for(int i=0; i<8; i++)
        lmp[move+directions[i]]--;
      */
    // equivalent the previous loop
    lmp[move-10]--;
    lmp[move-9]--;
    lmp[move-8]--;
    lmp[move-1]--;
    lmp[move+1]--;
    lmp[move+8]--;
    lmp[move+9]--;
    lmp[move+10]--;
}

/* Update the liberty map after 'move' is undone */
inline void updateLibmapUndo(const int move) {
    char *lmp = libmap;
    /*for(int i=0; i<8; i++)
        lmp[move+directions[i]]++;
      */
    // equivalent the previous loop
    lmp[move-10]++;
    lmp[move-9]++;
    lmp[move-8]++;
    lmp[move-1]++;
    lmp[move+1]++;
    lmp[move+8]++;
    lmp[move+9]++;
    lmp[move+10]++;
}

// Update all indices: liberty, p.mobility, and config
void updateAllIndicesMove(char *a, int color, int place, char *flipped, int flipCount) {
    updateLibmap(place);
#if USE_POTENTIAL_MOBILITY
    pMobDiff += libmap[place] * ((color << 1) - 3);
    for(int i=0; i<8; i++)
        if(a[place+directions[i]] == BLACK)
            pMobDiff++;
        else if(a[place+directions[i]] == WHITE)
            pMobDiff--;
#endif
    updateConfigIndicesPlace(place, color);
#if USE_POTENTIAL_MOBILITY
    const int coeff = ((color << 2) - 6);
#endif
    for(int i=0; i<flipCount; i++) {
        int flp = flipped[i];
        updateConfigIndicesFlip(flp);
#if USE_POTENTIAL_MOBILITY
        pMobDiff += libmap[flp] * coeff; // since flipping doubles the effect
        //printf("adj: %d, %d: %d\n", place, flp, ADJACENT(place, flp));
        if(ADJACENT(place, flp)) {
            pMobDiff -= 2;
        }
#endif
    }
}

// ! must be placed BEFORE undo()!!
void updateAllIndicesUndo(char *a, int color, int place, char *flipped, int nFlipped) {
    updateConfigIndicesRemove(place); // reset config. indices
#if USE_POTENTIAL_MOBILITY
    const int coeff = ((color << 2) - 6);
#endif
    for(int i=0; i<nFlipped; i++) {
        int flp = flipped[i];
        updateConfigIndicesFlip(flp);
#if USE_POTENTIAL_MOBILITY
        pMobDiff -= libmap[flp] * coeff;
        //printf("adj: %d, %d: %d\n", place, flp, ADJACENT(place, flp));
        if(ADJACENT(place, flp)) {
            pMobDiff += 2;
        }
#endif
    }
#if USE_POTENTIAL_MOBILITY
    for(int i=0; i<8; i++)
        if(a[place+directions[i]] == BLACK)
            pMobDiff--;
        else if(a[place+directions[i]] == WHITE)
            pMobDiff++;
    pMobDiff -= libmap[place] * ((color << 1) - 3);
#endif
    updateLibmapUndo(place); // reset liberty map
}

// Update all indices: liberty, config, but not pMobDiff
void updateIndicesMove(int color, int place, char *flipped, int flipCount) {
    updateLibmap(place);
    updateConfigIndicesPlace(place, color);
    for(int i=0; i<flipCount; i++) {
        updateConfigIndicesFlip(flipped[i]);
    }
}

void updateIndicesUndo(int color, int place, char *flipped, int nFlipped) {
    updateLibmapUndo(place); // reset liberty map
    updateConfigIndicesRemove(place); // reset config. indices
    for(int i=0; i<nFlipped; i++) {
        updateConfigIndicesFlip(flipped[i]);
    }
}



/**************************************
  End game solver, use both mobility and parity ordering
  
  Largely taken from Andersson/Smith's end game solver. Code is 
  rearranged for this program.
  *************************************/

uint parities;  // bitmap of region parities
uint holeID[91];  // bit masks for each square for retrieving parity of its region

/* Set up the parity information */
static void setUpParity(char *a) {
//  printf("In setUpParity.\n");
//  fflush(stdout);
  uint k = 1;
  for(int i=10; i<=80; i++) {
    if(a[i] == EMPTY){
      if( a[i-10] == EMPTY) holeID[i] = holeID[i-10];
      else if(a[i-9] == EMPTY) holeID[i] = holeID[i-9];
      else if(a[i-8] == EMPTY) holeID[i] = holeID[i-8];
      else if(a[i-1] == EMPTY) holeID[i] = holeID[i-1];
      else{ 
        holeID[i] = k; 
        k <<= 1; 
      }
    }
    else holeID[i] = 0;
  }
  /* Make multiple passes to make the parity information more accurate. The more
    passes, the more accurate the information, but the longer it takes */
#define MAX_PASSES_FOR_PARITY 1  // the value used by Andersson/Smith's end solver
  for(int z=MAX_PASSES_FOR_PARITY; z>0; z--){
    for(int i=80; i>=10; i--){
      if(a[i] == EMPTY){
        k = holeID[i];
        if(a[i+10] == EMPTY) holeID[i] = MIN(k, holeID[i+10]);
        if(a[i+9] == EMPTY) holeID[i] = MIN(k, holeID[i+9]);
        if(a[i+8] == EMPTY) holeID[i] = MIN(k, holeID[i+8]);
        if(a[i+1] == EMPTY) holeID[i] = MIN(k, holeID[i+1]);
      }
    }
    for(int i=10; i<=80; i++){
      if(a[i] == EMPTY){
        k = holeID[i];
        if(a[i-10] == EMPTY) holeID[i] = MIN(k, holeID[i-10]);
        if(a[i-9] == EMPTY) holeID[i] = MIN(k, holeID[i-9]);
        if(a[i-8] == EMPTY) holeID[i] = MIN(k, holeID[i-8]);
        if(a[i-1] == EMPTY) holeID[i] = MIN(k, holeID[i-1]);
      }
    }
  }
  /* Build up the parity bitmap */
  parities = 0;
  for(int i=10; i<=80; i++){
    parities ^= holeID[i];
  }
}

/* Solve end game using fixed ordering */
static int fixedEndSolve(char *a, int lastMove, int color, int empties, int discDiff,
                      int alpha, int beta) {
//  printf("In fixedEndSolve, empties: %d\n", empties);
//  fflush(stdout);
  if(true || COUNT_PRUNING) {
    countSearching++;
  }
  //checkTime();
  int score = - LARGE_INT;
  int currScore;
  int otherColor = OTHER(color);
  int nFlips, place;
  char flipped[20];
  EmptyList *currNode = emHead->next;
  while(currNode != NULL) {
    place = currNode->square;
    nFlips = tryMove(a, color, place, flipped, currNode);
    // no need to update indices since it will never again need to call legalMove or evaluateBoard
    if(nFlips) {
      if(empties == 2) { // now only one empty square left (just filled one)
        if(true || COUNT_PRUNING) {
          countSearching++;
          countEval++;
        }
        //checkTime();
        char lastEmpty = emHead->next->square;
        int nfpd = countFlips(a, otherColor, lastEmpty);
        if(nfpd) {
          currScore = discDiff + 2 * (nFlips - nfpd);
        }
        else { // the other player must pass
          int nfpd2 = countFlips(a, color, lastEmpty);
          if(nfpd2) { // this player can make the last move
            currScore = discDiff + 2 * (nFlips + nfpd2 +1);
          }
          else { // no one can make that last move
            currScore = discDiff + nFlips + nFlips; // which is actually score-1
            // since winner gets empties
            if(currScore >= 0)
              currScore += 2;
          }
        }
      }
      else {
        currScore = - fixedEndSolve(a, place, otherColor, empties-1, -discDiff-nFlips*2-1, 
                                    -beta, -alpha);
      }
      undo(a, color, place, flipped, nFlips, currNode);
      if(currScore > score) {
        score = currScore;
        if(score > alpha) {
          alpha = score;
          if(alpha >= beta) {
            if(COUNT_PRUNING) {
              countPruning++;
            }
            return score;
          }
        }
      }
    }
    currNode = currNode->next;
  }
  if(score == - LARGE_INT) { // no legal moves found for 'color'
    if(lastMove == PASS) {  // game over
      if(COUNT_PRUNING) {
        countEval++;
      }
      if(discDiff > 0)
        score = discDiff + empties;
      else if(discDiff < 0)
        score = discDiff - empties;
      else
        score = discDiff;
    }
    else {
      score = - fixedEndSolve(a, PASS, otherColor, empties, -discDiff, -beta, -alpha);
    }
  }
  return score;
}

/* Solve end game using parity for ordering */
static int parEndSolve(char *a, int lastMove, int color, int empties, int discDiff,
                      int alpha, int beta) {
//  printf("In parEndSolve.\n");
//  fflush(stdout);
  if(true || COUNT_PRUNING) {
    countSearching++;
  }
  checkTime();
  int score = - LARGE_INT;
  int currScore;
  int otherColor = OTHER(color);
  int place, nFlips;
  char flipped[20];
  uint holePar;
  EmptyList *currNode;
  /* Try the squares in odd regions first */
  int par;
  uint parMask;
  for(par=1, parMask=parities; par>=0; par--, parMask=~parMask) {
    currNode = emHead->next;
    while(currNode != NULL) {
      place = currNode->square;
      holePar = holeID[place];
      if(parMask & holePar) {
        nFlips = tryMove(a, color, place, flipped, currNode);
        // no need to update indices since it will never again need to call legalMove or evaluateBoard
        if(nFlips) {
          parities ^= holePar;  // update parity map
          if(empties < 1 + MIN_EMPTIES_FOR_PARITY)
            currScore = - fixedEndSolve(a, place, otherColor, empties-1, 
                                      -discDiff-2*nFlips-1, -beta, -alpha);
          else
            currScore = - parEndSolve(a, place, otherColor, empties-1,
                                      -discDiff-2*nFlips-1, -beta, -alpha);
          undo(a, color, place, flipped, nFlips, currNode);
          parities ^= holePar;  // restore parity  
          if(currScore > score) {  
            score = currScore;
            if(score > alpha) {
              alpha = score;
              if(alpha >= beta) {
                if(COUNT_PRUNING) {
                  countPruning++;
                }
                return score;
              }
            }
          }
        }
      }
      currNode = currNode->next;
    }
  }
  if(score == - LARGE_INT) { // no legal moves found for 'color'
    if(lastMove == PASS) {  // game over
      if(COUNT_PRUNING) {
        countEval++;
      }
      if(discDiff > 0)
        score = discDiff + empties;
      else if(discDiff < 0)
        score = discDiff - empties;
      else
        score = discDiff;
    }
    else {
      score = - parEndSolve(a, PASS, otherColor, empties, -discDiff, -beta, -alpha);
    }
  }
  return score;
}

/* Solve end game using 1-ply mobility ordering */
static int mobEndSolve(char *a, int lastMove, int color, int empties, int discDiff,
                      int alpha, int beta) {//
//  printf("In mobEndSove.\n");
//  fflush(stdout);
  if(true || COUNT_PRUNING) {
    countSearching++;
  }
  checkTime();
  int score = - LARGE_INT;
  int currScore;
  int otherColor = OTHER(color);
  int nFlips, place;
  char flipped[20];
  uint holePar;
  EmptyList *currNode;
  /* Move ordering -- the lower the oppponent mobility, the better */
  int nMoves = 0;
  char oppMob[64];
  EmptyList *moves[64];
  currNode = emHead->next;
  while(currNode != NULL) {
    place = currNode->square;
    nFlips = tryMove(a, color, place, flipped, currNode);
    if(nFlips) {
      updateIndicesMove(color, place, flipped, nFlips);
      char mob = findLegalMoves(a, otherColor);
      int j = nMoves;
      while(j > 0 && mob < oppMob[j-1])
        j--;
      for(int k=nMoves; k>j; k--) {
        moves[k] = moves[k-1];
        oppMob[k] = oppMob[k-1];
      }
      moves[j] = currNode;
      oppMob[j] = mob;
      nMoves++;
      undo(a, color, place, flipped, nFlips, currNode);
      updateIndicesUndo(color, place, flipped, nFlips);
    }
    currNode = currNode->next;
  }
  /* Now do the full search */
  if(nMoves) {
    for(int i=0; i<nMoves; i++) {
      currNode = moves[i];
      place = currNode->square;
      holePar = holeID[place];
      nFlips = tryMove(a, color, place, flipped, currNode);
      parities ^= holePar;
      if(empties < MIN_EMPTIES_FOR_MOBILITY + 1)
        currScore = - parEndSolve(a, place, otherColor, empties-1, -discDiff-2*nFlips-1,
                                  -beta, -alpha);
      else {
        updateIndicesMove(color, place, flipped, nFlips);
        currScore = - mobEndSolve(a, place, otherColor, empties-1, -discDiff-2*nFlips-1,
                                  -beta, -alpha);
        updateIndicesUndo(color, place, flipped, nFlips);
      }
      undo(a, color, place, flipped, nFlips, currNode);
      parities ^= holePar;
      if(currScore > score) {  
        score = currScore;
        if(score > alpha) {
          alpha = score;
          if(alpha >= beta) {
            if(COUNT_PRUNING) {
              countPruning++;
            }
            return score;
          }
        }
      }
    }
  }
  else { // no move to make
    if(lastMove == PASS) {  // game over
      if(COUNT_PRUNING) {
        countEval++;
      }
      if(discDiff > 0)
        score = discDiff + empties;
      else if(discDiff < 0)
        score = discDiff - empties;
      else
        score = discDiff;
    }
    else {
      score = - mobEndSolve(a, PASS, otherColor, empties, -discDiff, -beta, -alpha);
    }
  }
  return score;
}

/* End solving use evaluation function for move ordering */
static int evalEndSolve(char *a, int lastMove, int color, int empties, int discDiff,
                      int alpha, int beta) {//
//  printf("In EvalEndSove.\n");
//  fflush(stdout);
  if(true || COUNT_PRUNING) {
    countSearching++;
  }
  checkTime();
  int selfPieces = (64 - empties + discDiff) >> 1;
  int oppPieces = 64 - empties - selfPieces;
  int score = - LARGE_INT;
  int currScore;
  int otherColor = OTHER(color);
  int nFlips, place;
  char flipped[20];
  uint holePar;
  EmptyList *currNode;
  /* Move ordering -- the lower the oppponent eval score, the better */
  int nMoves = 0;
  double oppEval[MAX_EMPTIES_FOR_END_GAME_SOLVER];
  EmptyList *moves[64];
  currNode = emHead->next;
  while(currNode != NULL) {
    place = currNode->square;
    nFlips = tryMove(a, color, place, flipped, currNode);
    if(nFlips) {
      updateAllIndicesMove(a, color, place, flipped, nFlips);
      // eval for OPPONENT, hence reverse 'self' and 'opp' roles
      double eval = evaluateBoard(a, otherColor, otherColor, 0, 0, 
                                  oppPieces-nFlips, selfPieces+nFlips+1);
      int j = nMoves;
      while(j > 0 && eval < oppEval[j-1])
        j--;
      for(int k=nMoves; k>j; k--) {
        moves[k] = moves[k-1];
        oppEval[k] = oppEval[k-1];
      }
      moves[j] = currNode;
      oppEval[j] = eval;
      nMoves++;
      updateAllIndicesUndo(a, color, place, flipped, nFlips);
      undo(a, color, place, flipped, nFlips, currNode);
    }
    currNode = currNode->next;
  }
  /* Now do the full search */
  if(nMoves) {
    for(int i=0; i<nMoves; i++) {
      currNode = moves[i];
      place = currNode->square;
      holePar = holeID[place];
      nFlips = tryMove(a, color, place, flipped, currNode);
      updateAllIndicesMove(a, color, place, flipped, nFlips);
      parities ^= holePar;
      if(empties < MIN_EMPTIES_FOR_EVAL_IN_END_SOLVE + 1)
        currScore = - mobEndSolve(a, place, otherColor, empties-1, -discDiff-2*nFlips-1,
                                  -beta, -alpha);
      else
        currScore = - evalEndSolve(a, place, otherColor, empties-1, -discDiff-2*nFlips-1,
                                  -beta, -alpha);
      updateAllIndicesUndo(a, color, place, flipped, nFlips);
      undo(a, color, place, flipped, nFlips, currNode);
      parities ^= holePar;
      if(currScore > score) {  
        score = currScore;
        if(score > alpha) {
          alpha = score;
          if(alpha >= beta) {
            if(COUNT_PRUNING) {
              countPruning++;
            }
            return score;
          }
        }
      }
    }
  }
  else { // no move to make
    if(lastMove == PASS) {  // game over
      if(COUNT_PRUNING) {
        countEval++;
      }
      if(discDiff > 0)
        score = discDiff + empties;
      else if(discDiff < 0)
        score = discDiff - empties;
      else
        score = discDiff;
    }
    else {
      score = - evalEndSolve(a, PASS, otherColor, empties, -discDiff, -beta, -alpha);
    }
  }
  return score;
}

/* End solving use search + eval for move ordering */
// doesn't seem to be good in experiemnts
static int searchEndSolve(char *a, int lastMove, int color, int empties, int discDiff,
                      int alpha, int beta) {//
//  printf("In searchEndSove.\n");
//  fflush(stdout);
  if(true || COUNT_PRUNING) {
    countSearching++;
  }
  checkTime();
  int selfPieces = (64 - empties + discDiff) >> 1;
  int oppPieces = 64 - empties - selfPieces;
  int score = - LARGE_INT;
  int currScore;
  int otherColor = OTHER(color);
  int nFlips, place;
  char flipped[20];
  uint holePar;
  EmptyList *currNode;
  /* Move ordering -- the lower the oppponent eval score, the better */
  int nMoves = 0;
  double oppEval[MAX_EMPTIES_FOR_END_GAME_SOLVER];
  EmptyList *moves[64];
  currNode = emHead->next;
  int oldSearchDepth = searchDepth;
  int oldBase = base;
  bool oldUseEndGameSolver = useEndGameSolver;
  searchDepth = MIN(8, empties - MIN_EMPTIES_FOR_SEARCH_IN_END_SOLVE + 2);
  //printf("searchDepth: %d, empties: %d\n", searchDepth, empties); //debug
  base = 64 - empties - 4;
  useEndGameSolver = false;
  while(currNode != NULL) {
    place = currNode->square;
    nFlips = tryMove(a, color, place, flipped, currNode);
    if(nFlips) {
      updateAllIndicesMove(a, color, place, flipped, nFlips);
      // eval for OPPONENT, hence reverse 'self' and 'opp' roles
      //printf("self: %d, opp: %d, emSize: %d\n", selfPieces, oppPieces, emSize); // debug
      //printf("before getMax\n"); //debug
      double eval = getMax(a, place, lastMove, otherColor, 1, 0, 0, 0, 
                          oppPieces-nFlips, selfPieces+nFlips+1, SMALL_FLOAT, LARGE_FLOAT);
      //printf("after getMax\n"); //debug
      int j = nMoves;
      while(j > 0 && eval < oppEval[j-1])
        j--;
      for(int k=nMoves; k>j; k--) {
        moves[k] = moves[k-1];
        oppEval[k] = oppEval[k-1];
      }
      moves[j] = currNode;
      oppEval[j] = eval;
      nMoves++;
      updateAllIndicesUndo(a, color, place, flipped, nFlips);
      undo(a, color, place, flipped, nFlips, currNode);
    }
    currNode = currNode->next;
  }
  searchDepth = oldSearchDepth;
  base = oldBase;
  useEndGameSolver = oldUseEndGameSolver;
  /* Now do the full search */
  if(nMoves) {
    for(int i=0; i<nMoves; i++) {
      currNode = moves[i];
      place = currNode->square;
      holePar = holeID[place];
      nFlips = tryMove(a, color, place, flipped, currNode);
      updateAllIndicesMove(a, color, place, flipped, nFlips);
      parities ^= holePar;  //printf("**empties-1: %d\n", empties-1);//debug
      if(empties < MIN_EMPTIES_FOR_EVAL_IN_END_SOLVE + 1)
        currScore = - mobEndSolve(a, place, otherColor, empties-1, -discDiff-2*nFlips-1,
                                  -beta, -alpha);
      else if(empties < MIN_EMPTIES_FOR_SEARCH_IN_END_SOLVE + 1)
        currScore = - evalEndSolve(a, place, otherColor, empties-1, -discDiff-2*nFlips-1,
                                  -beta, -alpha);
      else {  //printf("empties-1: %d\n", empties-1); //debug
        currScore = - searchEndSolve(a, place, otherColor, empties-1, -discDiff-2*nFlips-1,
                                  -beta, -alpha);}
      updateAllIndicesUndo(a, color, place, flipped, nFlips);
      undo(a, color, place, flipped, nFlips, currNode);
      parities ^= holePar;
      if(currScore > score) {  
        score = currScore;
        if(score > alpha) {
          alpha = score;
          if(alpha >= beta) {
            if(COUNT_PRUNING) {
              countPruning++;
            }
            return score;
          }
        }
      }
    }
  }
  else { // no move to make
    if(lastMove == PASS) {  // game over
      if(COUNT_PRUNING) {
        countEval++;
      }
      if(discDiff > 0)
        score = discDiff + empties;
      else if(discDiff < 0)
        score = discDiff - empties;
      else
        score = discDiff;
    }
    else {
      score = - searchEndSolve(a, PASS, otherColor, empties, -discDiff, -beta, -alpha);
    }
  }
  return score;
}

/* Solve for 'color', assume it is also color's turn to move. */
inline static int endSolve(char *a, int lastMove, char color, int empties, int discDiff, 
                    double alpha_f, double beta_f) {//
//  printf("In endSolve.\n");
//  fflush(stdout);
  int alpha = (int)MAX(MIN(64, alpha_f), -64);
  int beta  = (int)MAX(MIN(64, beta_f), -64);
  if(alpha >= beta)
    return beta;
  //printf("alpha: %d, beta %d\n", alpha, beta); //debug
  int score;
  setUpParity(a);
  if(MIN_EMPTIES_FOR_SEARCH_IN_END_SOLVE <= 32
      && empties >= MIN_EMPTIES_FOR_SEARCH_IN_END_SOLVE)
    score = searchEndSolve(a, lastMove, color, empties, discDiff, alpha, beta);
  else if(MIN_EMPTIES_FOR_EVAL_IN_END_SOLVE <= 32
          && empties >= MIN_EMPTIES_FOR_EVAL_IN_END_SOLVE)
    score = evalEndSolve(a, lastMove, color, empties, discDiff, alpha, beta);
  else if(empties >= MIN_EMPTIES_FOR_MOBILITY)
    score = mobEndSolve(a, lastMove, color, empties, discDiff, alpha, beta);
  else if(empties >= MIN_EMPTIES_FOR_PARITY)
    score = parEndSolve(a, lastMove, color, empties, discDiff, alpha, beta);
  else
    score = fixedEndSolve(a, lastMove, color, empties, discDiff, alpha, beta);
  return score;
}

/* A short-hand helper function */
static bool hasCornerMoves(char *a, char color) {
  return legalMove(a, color, 10) || legalMove(a, color, 17) ||
        legalMove(a, color, 73) || legalMove(a, color, 80);
}






/**************************************************
  Function: getMin(...)
  ---- The MIN part of Minimax ---- 
  getMin always plays for "opponent"
*/
/* NOTE: passes = 1 if there has been odd number of passes on the search path, 
  zero otherwise (same in getMax) */
  
/**** NOW ONLY USE getMax(...), call -getMax for getMin
double getMin(char *a, char lastMove, char secondLast, char color, char depth, char passes, 
             char prevmaxDOF, char prevminDOF, //DOF found in previous getMax and getMin;
             char selfPieces, char oppPieces,
             double alpha, double beta) {
  if(DEBUG_MINIMAX || AB_DEBUG || CURRENT_DEBUG) {
    printf("In getMin, at depth %d -- alpha: %e, beta: %e\n", depth, alpha, beta);
  }
  if(true || (COUNT_PRUNING && showStatistics)) // need for time management
    countSearching++;
  checkTime();
  // Initialization started.
  uchar uPieces = base + depth + 4;
  assert(uPieces + emSize == 64);  // bug detection
  assert(selfPieces >= 0 && oppPieces >= 0);
  char place;
  char flipped[20], nFlips;
  char otherColor = OTHER(color);
  // * If there is only one move left, simply player it (if possible) and count scores
  if(uPieces == 63) {
    double score;
    EmptyList *currNode = emHead->next;
    place = currNode->square;
    nFlips = tryMove(a, color, place, flipped, currNode);
    if(nFlips) { // i.e. move can be played (by min) (min == opponent)
      score = evaluateEndGame(selfPieces-nFlips, oppPieces+nFlips+1);
      undo(a, color, place, flipped, nFlips, currNode);
    }
    else { // see if the other player (max) can play that move
      nFlips = tryMove(a, otherColor, place, flipped, currNode);
      if(nFlips) { // i.e. move can be played by the other player
        score = evaluateEndGame(selfPieces+nFlips+1, oppPieces-nFlips);
        undo(a, otherColor, place, flipped, nFlips, currNode);
      }
      else {  // no one can play this move
        score = evaluateEndGame(selfPieces, oppPieces);
      }
    }
    return score;
  }
  // ** Prepare for deeper search
  EmptyList **legalMoves, *legalMoves1[64];
  char nLegalMoves;
  nLegalMoves = findLegalMoves(a, color, legalMoves1);
  legalMoves = legalMoves1;
  if(CURRENT_DEBUG) {
    printf("legalMoves for color %d:\n", color);
    for(int i=0; i<nLegalMoves; i++)
      printf("%d, ", legalMoves[i]->square);
    printf("\n");
  }
  // ** test if there is any legal move posssible
  if(!nLegalMoves) {
    double score;
    if(lastMove == PASS) // last step is also pass, this branch ends here.
      score = evaluateEndGame(selfPieces, oppPieces);
    else // make a pass (set lastx to -1), depth NOT incremented.
      score = getMax(a, PASS, lastMove, otherColor, depth, 1-passes, prevmaxDOF, 0, 
                    selfPieces, oppPieces, alpha, beta);
    return score;
  }
  // ** Now there are legal moves to make
  double minValue = LARGE_FLOAT;
  double currValue = LARGE_FLOAT;
  // ** Shallow search for move ordering and maybe MPC cutoff 
  char remainingDepth = searchDepth - depth;
  // may need to experiment different searchDepth - depth threshhold values.
  if(USE_MOVE_ORDERING && remainingDepth >= MIN_DEPTH_FOR_ORDERING && 
      base + depth <= 64 - MIN_EMPTIES_FOR_ORDERING) {
    // back-up variables to be changed in the shallow search.
    double oldAlpha = alpha, oldBeta = beta;
    char oldSearchDepth = searchDepth;
    char oldUseEndGameSolver = useEndGameSolver;
    // change some variables
    useEndGameSolver = false;
    char orderingSearchDepth = remainingDepth - ORDERING_SEARCH_DEPTH_OFFSET;
    if(orderingSearchDepth > MAX_ORDERING_SEARCH_DEPTH)
      orderingSearchDepth = MAX_ORDERING_SEARCH_DEPTH;  // shallow search, with MAX
    char upperBound = emSize - MIN_EMPTIES_FOR_EVALUATION;
    if(orderingSearchDepth > upperBound)
      orderingSearchDepth = upperBound;
    searchDepth = depth + orderingSearchDepth;
    EmptyList *orderedMoves[64];
    double orderedMoveValues[64];
    int index;
    for(int i=0; i<nLegalMoves; i++) {
      EmptyList *currNode = legalMoves[i];
      char place = currNode->square;
      nFlips = tryMove(a, color, place, flipped, currNode);
      // can never reach cut-off depth here.
      if(remainingDepth <= MOB_ORDERING_DEPTH)
        currValue = findLegalMoves(a, otherColor);
      else if(remainingDepth <= ONE_PLY_ORDERING_DEPTH)
        currValue = evaluateBoard(a, otherColor, otherColor, prevmaxDOF, nLegalMoves, 
                                  selfPieces-nFlips, oppPieces+nFlips+1);
      else
        currValue = getMax(a, place, lastMove, otherColor, depth+1, passes, prevmaxDOF, 
                           nLegalMoves, selfPieces-nFlips, oppPieces+nFlips+1, 
                           SMALL_FLOAT, LARGE_FLOAT);
      undo(a, color, place, flipped, nFlips, currNode);
      if(currValue < minValue) {
        minValue = currValue;  
        if(minValue < beta) {
          //beta = minValue;
        }
      }
      // keep the order, best to worst (small to large, since in getmin)
      index = 0;
      while(index < i && currValue >= orderedMoveValues[index]) 
        index++;
      // insert this move, shift verything up
      for(int j=i; j>index; j--) {
        orderedMoves[j] = orderedMoves[j-1];
        orderedMoveValues[j] = orderedMoveValues[j-1];
      }
      orderedMoves[index] = currNode;
      orderedMoveValues[index] = currValue;
    }
    // ** Possibly use MPC
    if(USE_MPC && !inEndGame && orderingSearchDepth >= MIN_DEPTH_FOR_MPC &&
        nLegalMoves >= MIN_LEGAL_MOVES_FOR_MPC && emSize >= MIN_EMPTIES_FOR_MPC) {
      double topValue = orderedMoveValues[0]; // smallest  
      if(topValue >= TOP_VALUE_LOWER_BOUND && topValue <= TOP_VALUE_UPPER_BOUND) {  
        double threshold = ((double)MPC_THRESHOLD) / orderingSearchDepth;
        char j = 1;
        while(j < nLegalMoves && orderedMoveValues[j] - topValue <= threshold)
          j++;
        // i.e. low-valued moves are cut off.
        nLegalMoves = MAX(j, MIN(nLegalMoves, MPC_BASE_MIN_MOVES 
                                - (orderingSearchDepth >> 1))); 
      }
    }
    // restore values changed in the ordering search.
    alpha = oldAlpha;
    beta = oldBeta;
    useEndGameSolver = oldUseEndGameSolver;
    searchDepth = oldSearchDepth; 
    minValue = LARGE_FLOAT;
    // update the legalMoves array (now point to the ordered moves).
    legalMoves = orderedMoves;
  }
  if(AB_DEBUG) {
    printf("after ordering, at depth %d -- alpha: %e, beta: %e\n", depth, alpha, beta);
  }
  // ** try the ordered legal moves (or unordered moves if ordering didn't happen)
  for(int i=0; i<nLegalMoves; i++) {
    EmptyList *currNode = legalMoves[i];
    char place = currNode->square;
    nFlips = tryMove(a, color, place, flipped, currNode);
    // ** test if cut-off depth will be reached. If so, avoid an extra recursive call. 
    if(depth + 1 >= searchDepth + passes) { // always evaluate when it is one side's turn
      // Quiescence search for corner trade
      char secondLastIndex = inCorner[secondLast];
      char lastMoveIndex = inCorner[lastMove];
      char placeIndex = inCorner[place];
      if(DO_USE_QUIESCENCE_SEARCH && quiescenceSearch && 
          (secondLastIndex || lastMoveIndex || placeIndex)) {
        bool currValueUpdated = false;
        // search for immediate corner moves
        currValue = LARGE_FLOAT;
        if(secondLastIndex) { 
          double getMaxValue = SMALL_FLOAT;
          bool getMaxValueUpdated = false;
          searchDepth += 2;
          char fpd[20];
          for(int j=0; j<nEmptyCorners; j++) {
            EmptyList *currCornerNode = cornerNodes[j];
            if(currCornerNode != NULL) {
              char currCorner = currCornerNode->square;
              char nfp = tryMove(a, otherColor, currCorner, fpd, currCornerNode);
              if(nfp) {
                double newValue = getMin(a, currCorner, place, color, depth+2, passes, 
                                        prevmaxDOF, nLegalMoves, selfPieces-nFlips+nfp+1,
                                        oppPieces+nFlips+1-nfp, alpha, beta);
                undo(a, otherColor, currCorner, fpd, nfp, currCornerNode);
                if(newValue > getMaxValue) {
                  getMaxValue = newValue;
                  getMaxValueUpdated = true;
                }
              }
            }
          }
          searchDepth -= 2;
          if(getMaxValueUpdated && getMaxValue < currValue) {
            currValue = getMaxValue;
            currValueUpdated = true;
          }
        }
        // non-immediate corner moves
        if(placeIndex) {
          double getMaxValue = SMALL_FLOAT;
          bool getMaxValueUpdated = false;
          char fpd[20];
          for(int j=0; j<2; j++) {
            char adjCornerPlace = adjCorner[placeIndex][j];
            char dir = adjCornerPlace - place;
            while(a[adjCornerPlace] == otherColor ||
                  a[adjCornerPlace] == color) // for config like .OO.XXX.
              adjCornerPlace += dir;
            if(a[adjCornerPlace] == EMPTY && a[adjCornerPlace+dir] == color) {  
              EmptyList *currAdjCornerNode = allNodes[adjCornerPlace];
              assert(currAdjCornerNode != NULL); // bug detection
              char nfpd = tryMove(a, otherColor, adjCornerPlace, fpd, currAdjCornerNode);
              if(nfpd) {
                //printf("Here, adjCornerPlace: %d\n", adjCornerPlace); // temp debug
                if(legalMove(a, otherColor, adjCornerCorners[placeIndex][j])) {
                  searchDepth += 2;
                  double newValue = getMin(a, adjCornerPlace, place, color, depth+2, passes, 
                                    prevmaxDOF, nLegalMoves, selfPieces-nFlips+nfpd+1, 
                                    oppPieces+nFlips+1-nfpd, alpha, beta);
                  if(newValue > getMaxValue) {
                    getMaxValue = newValue;
                    getMaxValueUpdated = true;
                  }
                  searchDepth -= 2;
                }
                undo(a, otherColor, adjCornerPlace, fpd, nfpd, currAdjCornerNode);
              }
            }
          }
          if(getMaxValueUpdated && getMaxValue < currValue) {
            currValue = getMaxValue;
            currValueUpdated = true;
          }
        }
        if(lastMoveIndex) {
          // search 2 extra plies to discover a corner trade.
          if(legalMove(a, color, adjCornerCorners[lastMoveIndex][0]) || 
              legalMove(a, color, adjCornerCorners[lastMoveIndex][1])) {
            searchDepth += 2;
            double getMaxValue = getMax(a, place, lastMove, otherColor, depth+1, passes, 
                prevmaxDOF, nLegalMoves, selfPieces-nFlips, oppPieces+nFlips+1, alpha, beta);
            if(getMaxValue < currValue) {
              currValue = getMaxValue;
              currValueUpdated = true;
            }
            searchDepth -= 2;
          }
        }
        // make sure the position gets evaluated somehow.
        if(!currValueUpdated) { // used normal eval. if quiescence search doesn't happen
          currValue = evaluateBoard(a, otherColor, otherColor, prevmaxDOF, nLegalMoves, 
                                    selfPieces-nFlips, oppPieces+nFlips+1);
        }
      }
      else {
        currValue = evaluateBoard(a, otherColor, otherColor, prevmaxDOF, nLegalMoves, 
                                  selfPieces-nFlips, oppPieces+nFlips+1);
      }
    }
    else {
      if(false && useEndGameSolver && emSize <= MAX_EMPTIES_FOR_END_GAME_SOLVER)
        // only want to called end solve from top level
        currValue = endSolve(a, place, otherColor, emSize, selfPieces-oppPieces-2*nFlips-1,
                            alpha, beta);
      else
        currValue = getMax(a, place, lastMove, otherColor, depth+1, passes, prevmaxDOF, 
                        nLegalMoves, selfPieces-nFlips, oppPieces+nFlips+1, alpha, beta);
    }
    if(DEBUG_MINIMAX || CURRENT_DEBUG) {
      printf("getMax returned: %e, for move: %d. (depth: %d)\n", currValue, 
            legalMoves[i]->square, depth+1);
    }
    undo(a, color, place, flipped, nFlips, currNode);
    if(currValue < minValue) {
      minValue = currValue;  
      if(minValue < beta) {
        beta = minValue;  
        if(alpha >= beta) {
          if(COUNT_PRUNING && showStatistics)
            countPruning++;
          return beta;
        }
      }
    }
  }
  return minValue;
}
*/




// /* ------- Unused code -------- */
//
//
//#if USE_NEW_END_GAME_SOLVER
///* ---------------------------------------------
//  Exactly solve end-of-game
//  Borrowed some idea from Andersson's end of game solver.
//  It is still rather slow, since I haven't fully understood how exactly his 
//  code works. However, it is better than simply using the minimax core for 
//  mid-game. 
//  -----------------------------------------------*/
//
//// Use 91-square board -- more efficient (idea from Andersson's code)
//#ifndef DUMMY
//#define DUMMY 3
//#endif
//static const char directions91[] = {1, -1, 8, -8, 9, -9, 10, -10};
//static const uchar dirmask91[] = {
//  0,  0,  0,  0,  0,  0,  0,  0,  0,
//  0,  81, 81, 87, 87, 87, 87, 22, 22,
//  0,  81, 81, 87, 87, 87, 87, 22, 22,
//  0,  121,121,255,255,255,255,182,182,
//  0,  121,121,255,255,255,255,182,182,
//  0,  121,121,255,255,255,255,182,182,
//  0,  121,121,255,255,255,255,182,182,
//  0,  41, 41, 171,171,171,171,162,162,
//  0,  41, 41, 171,171,171,171,162,162,
//  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
//};
///* fixed square ordering: */
///* jcw's order, which is the best of 4 tried: */
//
//static char worst2best[64] =
//{
///*B2*/      20 , 25 , 65 , 70 ,
///*B1*/      11 , 16 , 19 , 26 , 64 , 71 , 74 , 79 ,
///*C2*/      21 , 24 , 29 , 34 , 56 , 61 , 66 , 69 ,
///*D2*/      22 , 23 , 38 , 43 , 47 , 52 , 67 , 68 ,
///*D3*/      31 , 32 , 39 , 42 , 48 , 51 , 58 , 59 ,
///*D1*/      13 , 14 , 37 , 44 , 46 , 53 , 76 , 77 ,
///*C3*/      30 , 33 , 57 , 60 ,
///*C1*/      12 , 15 , 28 , 35 , 55 , 62 , 75 , 78 ,
///*A1*/      10 , 17 , 73 , 80 , 
///*D4*/      40 , 41 , 49 , 50
//};
//
//// Set up according to worst2best, higher == better
///* --- Now not used ---
//static char fixedGoodness[91] = {
//  0, 0, 0, 0, 0, 0, 0, 0, 0,
//  0, 9, 2, 8, 6, 6, 8, 2, 9,
//  0, 2, 1, 3, 4, 4, 3, 1, 2,
//  0, 8, 3, 7, 5, 5, 7, 3, 8,
//  0, 6, 4, 5, 10,10,5, 4, 6,
//  0, 6, 4, 5, 10,10,5, 4, 6,
//  0, 8, 3, 7, 5, 5, 7, 3, 8,
//  0, 2, 1, 3, 4, 4, 3, 1, 2,
//  0, 9, 2, 8, 6, 6, 8, 2, 9,
//  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
//}
//*/
//
///* Test if flip in a direction is possible */
//inline bool canFlip91(char *a, char *placePointer, char dir, char color, char oppcolor) {
//  bool result = false;
//  char *p = placePointer + dir;
//  if(*p == oppcolor) {
//    p += dir;
//    if(*p == oppcolor) {
//      p += dir;
//      if(*p == oppcolor) {
//        p += dir;
//        if(*p == oppcolor) {
//          p += dir;
//          if(*p == oppcolor) {
//            p += dir;
//            if(*p == oppcolor) {
//              p += dir;
//            }
//          }
//        }
//      }
//    }
//    if(*p == color)
//      result = true;
//  }
//  return result;
//}
//
///* test if a move is legal one on a board array for the given color */
//bool legalMove91(char *a, char color, char place) {
//  char *p = a + place; // place pointer
//  if(*p != EMPTY)
//    return false;
//  bool result = false;
//  char oppcolor = OTHER(color);
//  uchar dirs = dirmask91[place];
//  if(dirs & 1) {
//    result = canFlip91(a, p, directions91[0], color, oppcolor);
//    if(result)
//      return result;
//  }
//  if(dirs & 2) {
//    result = canFlip91(a, p, directions91[1], color, oppcolor);
//    if(result)
//      return result;
//  }
//  if(dirs & 4) {
//    result = canFlip91(a, p, directions91[2], color, oppcolor);
//    if(result)
//      return result;
//  }
//  if(dirs & 8) {
//    result = canFlip91(a, p, directions91[3], color, oppcolor);
//    if(result)
//      return result;
//  }
//  if(dirs & 16) {
//    result = canFlip91(a, p, directions91[4], color, oppcolor);
//    if(result)
//      return result;
//  }
//  if(dirs & 32) {
//    result = canFlip91(a, p, directions91[5], color, oppcolor);
//    if(result)
//      return result;
//  }
//  if(dirs & 64) {
//    result = canFlip91(a, p, directions91[6], color, oppcolor);
//    if(result)
//      return result;
//  }
//  if(dirs & 128) {
//    result = canFlip91(a, p, directions91[7], color, oppcolor);
//    if(result)
//      return result;
//  }
//  return result;
//}
//
///* find all the legal moves. Returns the number of legalMoves found */
//char findLegalMoves91(char *a, char color, EmptyList **legalMoves) {
//  char count = 0;
//  EmptyList *currNode = emHead->next;
//  while(currNode != NULL) {
//    char place = currNode->square;
//    if(legalMove91(a, color, place)) {
//      legalMoves[count] = currNode;
//      count++;
//    }
//    currNode = currNode->next;
//  }
//  return count;
//}
//
///* Return the cumulative disc flips */
//inline char flip91(char *a, char *placePointer, char dir, char color, char oppcolor, 
//                char *flipped, char cumulativeFlips) {
//  char count = cumulativeFlips;
//  char *p = placePointer + dir;
//  if(*p == oppcolor) {
//    p += dir;
//    if(*p == oppcolor) {
//      p += dir;
//      if(*p == oppcolor) {
//        p += dir;
//        if(*p == oppcolor) {
//          p += dir;
//          if(*p == oppcolor) {
//            p += dir;
//            if(*p == oppcolor) {
//              p += dir;
//            }
//          }
//        }
//      }
//    }
//    if(*p == color) {
//      p -= dir;
//      while(p != placePointer) {
//        *p = color;
//        flipped[count] = p - a;
//        count++;
//        p -= dir;
//      }
//    }
//  }
//  // number of element in the current 'flipped' array minus previous count.
//  return count;
//}
//
///* Return the number discs flipped (91-square board) */
//char tryMoveEnd(char *a, char color, char place, char *flipped, EmptyList *currNode) {
//  if(DEBUG_MINIMAX) {
//    printf("tryMoveEnd - place: %d, color %d\n", place, color);
//  }
//  char *p = a + place; // place pointer
//  if(*p != EMPTY)
//    return 0;
//  char flipCount = 0;
//  char oppcolor = OTHER(color);
//  uchar dirs = dirmask91[place];
//  if(dirs & 1) {
//    flipCount = flip91(a, p, directions91[0], color, oppcolor, flipped, flipCount);
//  }
//  if(dirs & 2) {
//    flipCount = flip91(a, p, directions91[1], color, oppcolor, flipped, flipCount);
//  }
//  if(dirs & 4) {
//    flipCount = flip91(a, p, directions91[2], color, oppcolor, flipped, flipCount);
//  }
//  if(dirs & 8) {
//    flipCount = flip91(a, p, directions91[3], color, oppcolor, flipped, flipCount);
//  }
//  if(dirs & 16) {
//    flipCount = flip91(a, p, directions91[4], color, oppcolor, flipped, flipCount);
//  }
//  if(dirs & 32) {
//    flipCount = flip91(a, p, directions91[5], color, oppcolor, flipped, flipCount);
//  }
//  if(dirs & 64) {
//    flipCount = flip91(a, p, directions91[6], color, oppcolor, flipped, flipCount);
//  }
//  if(dirs & 128) {
//    flipCount = flip91(a, p, directions91[7], color, oppcolor, flipped, flipCount);
//  }
//  if(flipCount) {
//    *p = color;
//    // remove the node from the list
//    EmptyList *prev = currNode->prev;
//    EmptyList *next = currNode->next;
//    prev->next = next;
//    if(next != NULL)
//      next->prev = prev;
//    emSize--;
//  }
//  if(DEBUG_MINIMAX) {
//    printBoardArray(a);
//    printf("flipCount: %d\n", flipCount);
//  }
//  return flipCount;
//}
//
///* For 91-square board */
//inline void undo_end(char *a, char color, char place, char *flipped, char nFlipped,
//                    EmptyList *currNode) {
//  a[place] = EMPTY;
//  char otherColor = OTHER(color);
//  for(char i=0; i<nFlipped; i++) {
//    a[flipped[i]] = otherColor;
//  }
//  // insert this node back to the list
//  currNode->prev->next = currNode;
//  EmptyList *next = currNode->next;
//  if(next != NULL)
//    next->prev = currNode;
//  if(nFlipped)
//    emSize++;
//  if(DEBUG_MINIMAX) {
//    //printf("undo move: %d\n", place);
//    //printBoardArray(a);
//  }
//}
//
///* Near the end of game, perfect move */
//char getExactMove(char *a, char color, char selfPieces, char oppPieces, 
//                  EmptyList **legalMoves, char nLegalMoves) {
//  double alpha, beta;
//  double bestValue, currValue;
//  char bestMove;
//  if(winLarge) {
//    alpha = SMALL_FLOAT;
//    beta = LARGE_FLOAT;
//  }
//  else if(loseSmall) {
//    alpha = SMALL_FLOAT;
//    beta = 1;
//  }
//  else {  // WDL only
//    alpha = -1;
//    beta = 1;
//  }
//  bestValue = SMALL_FLOAT;
//  bestMove = PASS;
//  char *b = a;
//  /* Build up the empty list in best->worst order by converting 64-square-board empty list
//    to the 91-square one */
//  EmptyList *currNode = emHead->next;
//  while(currNode != NULL) {
//    char sq = currNode->square;
//    currNode->square = CONV_64_91(sq);
//    currNode = currNode->next;
//  }
//  // bug detection
//  assert(selfPieces + oppPieces + emSize == 64);
//  assert(selfPieces >= 0 && oppPieces >= 0);
//  // Try for all legal moves emptie squares (legalMoves already ordered in getMinimaxMove)
//  char oppcolor = OTHER(color);
//  char flipped[20], nFlips;
//  for(int i=0; i<nLegalMoves; i++) {
//    currNode = legalMoves[i];
//    char place = currNode->square;
//    nFlips = tryMoveEnd(b, color, place, flipped, currNode);
//    if(showDots) { // show progress
//      printf(".");
//      fflush(stdout);
//    }
//    if(nFlips) {
//      currValue = getMin_exact(b, place, oppcolor, selfPieces+nFlips+1, oppPieces-nFlips, 
//                              alpha, beta);
//      undo_end(b, color, place, flipped, nFlips, currNode);
//      //printf("move: %d, value: %f\n", place, currValue);  // debug
//      if(currValue > bestValue) {
//        bestValue = currValue;
//        bestMove = place;
//      }
//      if(bestValue > alpha)
//        alpha = bestValue;
//      if(alpha >= beta)
//        break;
//    }
//  }
//  // Convert the empty list back to 64-square form
//  currNode = emHead->next;
//  while(currNode != NULL) {
//    char sq = currNode->square;
//    currNode->square = CONV_91_64(sq);
//    currNode = currNode->next;
//  }
//  //printf("bestMove: %d\n", bestMove); // debug
//  bestValue_global = bestValue;
//  assert(selfPieces + oppPieces + emSize == 64);
//  assert(selfPieces >= 0 && oppPieces >= 0);
//  return bestMove;
//}
//
///* --- min part --- */
//double getMin_exact(char *a, char lastMove, char color, char selfPieces, char oppPieces, 
//                  double alpha, double beta) {
//  if(COUNT_PRUNING && showStatistics)
//    countSearching++;
//  char uPieces = selfPieces + oppPieces;
//  //assert(uPieces + emSize == 64);
//  //assert(selfPieces >= 0 && oppPieces >= 0);
//  char nFlips, flipped[20];
//  char oppcolor = OTHER(color);
//  char place;
//  EmptyList *currNode;
//  /* Only the last square left */
//  if(uPieces == 63) {
//    double score;
//    currNode = emHead->next;
//    place = currNode->square;
//    nFlips = tryMoveEnd(a, color, place, flipped, currNode);
//    if(nFlips) { // i.e. move can be played (by min) (min == opponent)
//      score = evaluateEndGame_exact(selfPieces-nFlips, oppPieces+nFlips+1);
//      undo_end(a, color, place, flipped, nFlips, currNode);
//    }
//    else { // see if the other player (max) can play that move
//      nFlips = tryMoveEnd(a, oppcolor, place, flipped, currNode);
//      if(nFlips) { // i.e. move can be played by the other player
//        score = evaluateEndGame_exact(selfPieces+nFlips+1, oppPieces-nFlips);
//        undo_end(a, oppcolor, place, flipped, nFlips, currNode);
//      }
//      else {  // no one can play this move
//        score = evaluateEndGame_exact(selfPieces, oppPieces);
//      }
//    }
//    return score;
//  }
//  /* search for every empty place */
//  bool hasLegalMove = false;
//  double minValue, currValue;
//  minValue = LARGE_FLOAT;
//  currNode = emHead;
//  for(char i=0; i<emSize; i++) {
//    currNode = currNode->next;
//    place = currNode->square;
//    nFlips = tryMoveEnd(a, color, place, flipped, currNode);
//    if(nFlips) {
//      hasLegalMove = true;
//      currValue = getMax_exact(a, place, oppcolor, selfPieces-nFlips, oppPieces+nFlips+1,
//                              alpha, beta);
//      undo_end(a, color, place, flipped, nFlips, currNode);
//      if(currValue < minValue)
//        minValue = currValue;
//      if(minValue < beta)
//        beta = minValue;
//      if(beta <= alpha) {
//        if(COUNT_PRUNING && showStatistics)
//          countPruning++;
//        return beta;
//      }
//    }
//  }
//  /* if no legal move, see if should search on or not */
//  if(!hasLegalMove) {
//    if(lastMove == PASS) {
//      return evaluateEndGame_exact(selfPieces, oppPieces);
//    }
//    else {
//      return getMax_exact(a, PASS, oppcolor, selfPieces, oppPieces, alpha, beta);
//    }
//  }
//  return minValue;
//}
//
///* --- max part --- */
//double getMax_exact(char *a, char lastMove, char color, char selfPieces, char oppPieces, 
//                  double alpha, double beta) {
//  if(COUNT_PRUNING && showStatistics)
//    countSearching++;
//  char uPieces = selfPieces + oppPieces;
//  //assert(uPieces + emSize == 64);
//  //assert(selfPieces >= 0 && oppPieces >= 0);
//  char nFlips, flipped[20];
//  char oppcolor = OTHER(color);
//  char place;
//  EmptyList *currNode;
//  /* Only the last square left */
//  if(uPieces == 63) {
//    double score;
//    currNode = emHead->next;
//    place = currNode->square;
//    nFlips = tryMoveEnd(a, color, place, flipped, currNode);
//    if(nFlips) { // i.e. move can be played (by min) (min == opponent)
//      score = evaluateEndGame_exact(selfPieces+nFlips+1, oppPieces-nFlips);
//      undo_end(a, color, place, flipped, nFlips, currNode);
//    }
//    else { // see if the other player (max) can play that move
//      nFlips = tryMoveEnd(a, oppcolor, place, flipped, currNode);
//      if(nFlips) { // i.e. move can be played by the other player
//        score = evaluateEndGame_exact(selfPieces-nFlips, oppPieces+nFlips+1);
//        undo_end(a, oppcolor, place, flipped, nFlips, currNode);
//      }
//      else {  // no one can play this move
//        score = evaluateEndGame_exact(selfPieces, oppPieces);
//      }
//    }
//    return score;
//  }
//  /* search for every empty place */
//  bool hasLegalMove = false;
//  double maxValue, currValue;
//  maxValue = SMALL_FLOAT;
//  currNode = emHead;
//  for(char i=0; i<emSize; i++) {
//    currNode = currNode->next;
//    place = currNode->square;
//    nFlips = tryMoveEnd(a, color, place, flipped, currNode);
//    if(nFlips) {
//      hasLegalMove = true;
//      currValue = getMin_exact(a, place, oppcolor, selfPieces+nFlips+1, oppPieces-nFlips,
//                              alpha, beta);
//      undo_end(a, color, place, flipped, nFlips, currNode);
//      if(currValue > maxValue)
//        maxValue = currValue;
//      if(maxValue > alpha)
//        alpha = maxValue;
//      if(alpha >= beta) {
//        if(COUNT_PRUNING && showStatistics)
//          countPruning++;
//        return alpha;
//      }
//    }
//  }
//  if(!hasLegalMove) {
//    if(lastMove == PASS) {
//      return evaluateEndGame_exact(selfPieces, oppPieces);
//    }
//    else {
//      return getMin_exact(a, PASS, oppcolor, selfPieces, oppPieces, alpha, beta);
//    }
//  }
//  return maxValue;
//}
//
///* --- Return exact score --- */
//inline int evaluateEndGame_exact(char selfPieces, char oppPieces) {
//  if(COUNT_PRUNING && showStatistics)
//    countEval++; 
//  int score;
//  if(selfPieces > oppPieces)
//    score = 64 - oppPieces - oppPieces;
//  else if(selfPieces < oppPieces)
//    score = selfPieces + selfPieces - 64;
//  else
//    score = 0;
//  //assert(selfPieces+oppPieces+emSize == 64); // debug
//  //assert(selfPieces >= 0 && oppPieces >= 0);
//  return score; // make sure a secured win out-weighs any estimated advantage.
//}
//#endif
//



/* Quiescence search, mainly for avoiding horizon effect in corner trade */
/* --- Not quite complete and might not be even correct (Currently not used) --- 
static double getQuiescenceValueMax(char *a, char lastMove, char secondLast, char color, 
                      char prevmaxDOF, char prevminDOF, char selfPieces, char oppPieces, 
                      double alpha, double beta) {
  double result;
  double value1, value2;
  bool updated;
  char oppcolor;
  char nLegalMoves1, nLegalMoves2;
  char legalMoves1[64];
  char nFlips1, nFlips2;
  char flipped1[20], flipped2[20];
  // Init
  updated = false;
  oppcolor = OTHER(color);
  result = SMALL_FLOAT;
  nLegalMoves1 = findLegalMoves(a, color, legalMoves1);
  // First see if there is an immediate corner move available
  bool hasImmediateCornerMove = false;
  for(int i=0; i<4; i++) {
    char place = corners[i];
    nFlips1 = tryMove(a, color, place, flipped1);
    if(nFlips1) {
      double newValue = getQuiescenceValueMin(a, place, lastMove, oppcolor, nLegalMoves1, 
                  prevminDOF, selfPieces+nFlips1+1, oppPieces-nFlips1, alpha, beta);
      undo(a, color, place, flipped1, nFlips1);
      if(newValue > result) {
        result = newValue;
        hasImmediateCornerMove = true;
      }
      if(result > alpha)
        alpha = result;
      if(alpha >= beta)
        break;
    }
  }
  if(hasImmediateCornerMove) {
    assert(selfPieces+oppPieces+nEmpties == 64);
    return result;
  }
  // At this point, there is no immediate corner move.
  char lastMoveIndex = inCorner[lastMove];
  char secondLastIndex = inCorner[secondLast];
  for(char i=0; i<nLegalMoves1; i++) { // getmax loop
    char place = legalMoves1[i];
    value1 = LARGE_FLOAT;
    bool updated2 = false; // whether value1 is updated in this iteration
    nFlips1 = tryMove(a, color, place, flipped1);
    bool searchOn = false;
    for(char j=0; j<4; j++) {
      if(legalMove(a, oppcolor, corners[j]))
        searchOn = true;
    }
    if(!searchOn) { // i.e. no further corner moves possible
      undo(a, color, place, flipped1, nFlips1);
      continue;
    }
    nLegalMoves2 = findLegalMoves(a, color);
    for(char j=0; j<4; j++) { // getmin loop
      char cornerPlace = corners[j];
      nFlips2 = tryMove(a, oppcolor, cornerPlace, flipped2);
      if(nFlips2) {
        updated = true;
        updated2 = true;
        value2 = evaluateBoard(a, color, color, nLegalMoves1, nLegalMoves2,
                    selfPieces+nFlips1+1-nFlips2, oppPieces-nFlips1+nFlips2+1);
        if(value2 < value1)
          value1 = value2;
        undo(a, oppcolor, cornerPlace, flipped2, nFlips2);
      }
    }
    undo(a, color, place, flipped1, nFlips1);
    if(updated2 && value1 > result)
      result = value1;
    if(result > alpha)
      alpha = result;
    if(alpha >= beta)
      break;
  }
  if(!updated) { // if already quiescent, i.e. no evaluation for further search.
    result = evaluateBoard(a, color, color, prevmaxDOF, prevminDOF, selfPieces, oppPieces);
  }
  assert(selfPieces+oppPieces+nEmpties == 64);
  return result;
}

// min part -- by changing prospect
inline static double 
getQuiescenceValueMin(char *a, char lastMove, char secondLast, char color, 
                    char prevmaxDOF, char prevminDOF, char selfPieces, char oppPieces, 
                    double alpha, double beta) {
  return 0 - getQuiescenceValueMax(a, lastMove, secondLast, color, prevminDOF, prevmaxDOF, 
                                  oppPieces, selfPieces, 0-beta, 0-alpha);
} */


/* test /degug */
//int main() {
//  char b[64];
//  nEmpties = 0;
//  for(int i=0; i<64; i++) {
//    b[64] = EMPTY;
//  }
//  b[27] = WHITE;
//  b[28] = BLACK;
//  b[31] = BLACK;
//  b[32] = WHITE;
//  for(int i=0; i<64; i++) {
//    if(b[i] == EMPTY)
//      empties[nEmpties++] = i;
//  }
//  printBoardArray(b);
//  char lm[64], nlm;
//  nlm = findLegalMoves(b, BLACK, lm);
//  for(int i=0; i<nlm; i++)
//    printf("%d\n", lm[i]);
//  return 0;
//}




/******** Experiment code now not used ******************

#define MOBILITY_RATIO_EXPONENT 0.5 // with what factor M.R. affects corner weights.
#define RAW_SCORE_NORMALIZATION 0.05 // also used for corner weights.

static double situationFactor;  // adjucting corner weight
static double scoreCorrection;  // doesn't really affect the result of minimax

* Make a rough estimation of the situation, so as to see if the computer 
    should favor a trade of corners or be avert to it. *
  if(searchDepth == origialSearchDepth &&  // don't do this if brute-forcing
     searchDepth >= 8) { // only do this with > certain depth level
    searchDepth -= 4;  // shallow search estimate
    double estiScore, currScore1, currScore2;
    estiScore = getMax(lastMove, color, 0, 0, nLegalMoves, nLegalMoves, selfPieces, 
                        oppPieces, SMALL_FLOAT, LARGE_FLOAT);
    situationFactor = (double)pow(2, estiScore * RAW_SCORE_NORMALIZATION);
    currScore2 = getMax(lastMove, color, 0, 0, nLegalMoves, nLegalMoves, selfPieces, 
                        oppPieces, SMALL_FLOAT, LARGE_FLOAT);
    scoreCorrection = currScore1 - currScore2;
    searchDepth += 4;  // restore
  }


*/

/* --- Old Evaluation Function --- */
//double evaluateBoard__(char *a, char forWhom, char whoseTurn, char prevmaxDOF, 
//                    char prevminDOF, char selfPieces, char oppPieces) {
//  if(DEBUG_MINIMAX) {
//    printBoardArray(a);
//    printf("Depth: %d, eval. value: %e\n", selfPieces+oppPieces-base-4, 
//            (double)(selfPieces-oppPieces));
//  }
//  if(COUNT_PRUNING) {
//    countEval++;
//    countSearching++;
//  }
//  
//  char nLegalMoves = findLegalMoves(a, whoseTurn); // slow step.
//  if(forWhom == whoseTurn)
//    prevmaxDOF = nLegalMoves;
//  else
//    prevminDOF = nLegalMoves;
//  
//  // disc count unimportant during midd
//  double result = (0.01 * (selfPieces - oppPieces)) + 
//                (prevmaxDOF / (prevminDOF + extra));
//  /* A little simple account for corners */
//  char sign;
//  if(forWhom == BLACK)
//    sign = 1; 
//  else
//    sign = -1;
//  // BLACK = 1, WHITE = 2, EMPTY = 0
//  // Approach 1 -- About the same speed as Approch 2
//  char mean = EMPTY+1;
//  result += (((a[0]+1) % 3 - mean) * sign) * ESTIMATED_CORNER_WORTH;
//  result += (((a[7]+1) % 3 - mean) * sign) * ESTIMATED_CORNER_WORTH;
//  result += (((a[56]+1) % 3 - mean) * sign) * ESTIMATED_CORNER_WORTH;
//  result += (((a[63]+1) % 3 - mean) * sign) * ESTIMATED_CORNER_WORTH;
//  // */
//  /* // Approach 2 -- need to set sign = -2.0 for BLACK, 2.0 for WHITE
//  double mean = ((double)BLACK + (double)WHITE) * 0.5;
//  result += ((double)(a[0]) - mean) * (a[0] / (a[0] + 1.0e-20)) * sign * cornerWorth;
//  result += ((double)(a[7]) - mean) * (a[7] / (a[7] + 1.0e-20)) * sign * cornerWorth;
//  result += ((double)(a[56]) - mean) * (a[56] / (a[56] + 1.0e-20)) * sign * cornerWorth;
//  result += ((double)(a[63]) - mean) * (a[63] / (a[63] + 1.0e-20)) * sign * cornerWorth;
//  // */
//  return result;
//}

/*
          char lm[64];
          char nlm = findLegalMoves(a, OTHER(color), lm);
          for(char j=0; j<nlm; j++) {
            // This is in fact the getMax part.
            char cornerPlace = corners[j];
            char nfpd = tryMove(a, OTHER(color), cornerPlace, fpd);
            if(nfpd) {
              currValue = getMin(a, cornerPlace, color, depth+2, passes, nlm, nLegalMoves,
                                  alpha, beta);
              undo(a, OTHER(color), cornerPlace, fpd, nfpd);
            }
          }
*/
