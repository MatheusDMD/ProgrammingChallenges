/****
  Minimax.hpp
  
  */
  
#ifndef _MINIMAX_HPP_
#define _MINIMAX_HPP_

#include <time.h>
#include <assert.h>
#include "board.hpp"

#define MAX(x, y)   ((x) > (y)? (x) : (y))
#define MIN(x, y)   ((x) < (y)? (x) : (y))

#define MAX_INT 2147483647
#define MIN_INT -2147483648
#define LARGE_INT 1000

#define LARGE_FLOAT 1.0e35
#define SMALL_FLOAT -1.0e35
#define HUGE_FLOAT 2.0e38
#define TINY_FLOAT -2.0e38

#ifndef DUMMY
#define DUMMY 3
#endif

#ifndef uint
#define uint unsigned int
#endif

// The following two values are for 64-square board
#define HARD_WIRED_FIRST_MOVE 37  // F-5 position
#define HARD_WIRED_FIRST_MOVE_BOARD_FLIPPED 34  // C-5

#define CORNER_1 10
#define CORNER_2 17
#define CORNER_3 73
#define CORNER_4 80

#define CONV_21_91(x, y)  (10+(x)+9*(y))
#define CONV_64_91(n)     (10+((n)&7)+9*((n)>>3))
#define CONV_91_64(n)     (((n)-((n)/9)*9-1)+(((n)/9-1)<<3))


/* --- Evaluation parameters --- */

/* Manually given weight -- ideally should be automatically learnt */
#define USE_DISC_COUNT      1  // Traditional discDiff counting in mid-game eval

#define DISC_WEIGHT_TEST -0.28  // was used when testing (no longer used)
#define DISC_WEIGHT_STAGE_1     -0.25
#define DISC_WEIGHT_STAGE_1_2   -0.18
#define DISC_WEIGHT_STAGE_2     -0.12
#define DISC_WEIGHT_STAGE_3     -0.08
#define DISC_WEIGHT_STAGE_4     -0.05
#define DISC_WEIGHT_STAGE_5     +0.05
#define STAGE_1_END       12
#define STAGE_1_2_START   16
#define STAGE_1_2_END     18  // was 24
#define STAGE_2_START     30
#define STAGE_2_END       36
#define STAGE_3_START     44
#define STAGE_3_END       48
#define STAGE_4_START     52
#define STAGE_4_END       52
#define STAGE_5_START     56

// computer stop give negative weight to disc difference when oppDisc - selfDisc > this value
/* Has no effect when playing against a strong player, but might help win more against 
  bad players. To disable this criteria, simply give it a big value, e.g. 1000. */
// proven necessary
#define DISC_DIFF_CEILING   10  // has been experimented for a reasonable value

/* Next two boolean variable should not be changed unless there is clear reason */
#define USE_MOBILITY_RATIO  1    // default 1
#define USE_DISC_RATIO      0    // default 0

/* Used a disc weight array indexed by number of the number of empty squares, so
  as to accurate find the stage even in the case of quiescence search (variable depth) */
#define USE_DISC_WEIGHT_ARR   1

/* More coefficients -- ideally should all be automatically learnt */
#define ESTIMATED_CORNER_WORTH 5.0  // estimate
//#define NEAR_TO_CORNER_NEG_DIAG 0.5
//#define NEAR_TO_CORNER_NEG_EDGE 0.1
#define NEAR_TO_CORNER_NEG_DIAG_COEFF 0.01
#define DENOMINATOR_EXTRA_DOF 1.0  // avoiding divide by 0
#define MOVE_SIDE_SURPLUS 0.25
#define RANDOM_FACTOR_CONSTANT 0.05
#define MIN_MOBILITY_CONSIDERED 1.0  // so as to regard mobility at least this value in eval
                     // -- help avoiding mistake (esp. at corner) due to extreme mob. ratio

/* Potential mobility accounting */
#define USE_POTENTIAL_MOBILITY  0  // Not yet implemented
// Potential mobiltiy weights
// ### ... TO ADD ...

/* Edge configurations (very experimental !! For sure not sophisticated enough) 
  NOTE: Probably can't get any better with these values -- they have been already quite
        good. But the program still can't play too well without more sophisticated 
        patterns matching */
#define USE_EDGE_VALUE 1
// Coefficients are multiplied by the number of empty squares
#define UNBALANCED_EDGE_1_COEFF -0.002  // .X......
#define UNBALANCED_EDGE_2_COEFF -0.004  // .XX.....
#define UNBALANCED_EDGE_3_COEFF -0.001
#define UNBALANCED_EDGE_4_COEFF -0.001
#define UNBALANCED_EDGE_5_COEFF -0.0001
#define BALANCED_EDGE_6_COEFF    0.002  // counted from both corner (twice)
#define SINGLE_DISC_NEAR_CORNER_COEFF   -0.012

#define EDGE_VALUE_NOMALIZATION  5.0
#define EMPTIES_OFFSET 10

/* Multi-probability Cutoff (experimental) */
/* NOTE: This so-called MPC is really just some kind of toy stuff, as it is 
  based on no statistical model. Its only support is that it "seems to work".
  Moreover, real effective MPC needs accurate evaluation function as well.
  */
// Current combination tends to be conservative (cut off fewer)
#define USE_MPC 1  // for speed, though may be not very safe -- more accurate evaluation function needed!

#define MPC_THRESHOLD 3.0  // a guess (will be divided by ordering search depth when used)
#define MIN_DEPTH_FOR_MPC 4
#define MIN_EMPTIES_FOR_MPC 20
#define MIN_LEGAL_MOVES_FOR_MPC 3
#define MPC_BASE_MIN_MOVES  8
#define TOP_VALUE_LOWER_BOUND -100.5 //big value - i.e. always try the cut
#define TOP_VALUE_UPPER_BOUND 100.5

#define MPC_FEEDBACK    0  // for console testing/debuggin

/* Quiescence search */
/* Will add a bit running time. However, it must be done in order to give more 
  accurate evaluation and avoid big blunders. */
// Seems to be working now.
#define DO_USE_QUIESCENCE_SEARCH  1
// since corner trading is very unlike in the very early stage of the game
#define QUIESCENCE_START 12

/* Move ordering Part */
// need to be experimented for best values
#define USE_MOVE_ORDERING   1 // proved to be very necessary

#define MIN_DEPTH_FOR_ORDERING        3  // min depth to use ordering
#define MAX_ORDERING_SEARCH_DEPTH     8 // max depth of an ordering search
#define ORDERING_SEARCH_DEPTH_OFFSET  4 // how many plies fewer is ordering search than normal search
// depth threshold (upper bound) for using simpler mover ordering methods (full ordering beyond)
#define ONE_PLY_ORDERING_DEPTH    5
#define MOB_ORDERING_DEPTH        4
// Following two marcos are alreday experimented for optimal values, (12, 10) also good
#define MIN_EMPTIES_FOR_ORDERING    14 
#define MIN_EMPTIES_FOR_EVALUATION  12

/* Because the current mid-game evaluation function works pretty awfully when there are 
  very few empty squares left. Should be removed once the evaluation function is improved
  for near-end-game. */
#define APPLY_SEARCH_LIMIT_TO_MID_GAME    1
#define MIN_EMPTIES_FOR_MID_GAME_EVAL     12

/* Whether or not use WDL search to secure a win/draw if can be done. Draw back is 
  that this takes extra time and may decrease winning margin. Default is 1 */
#define USE_WDL_SEARCH 1
#define BEST_VALUE_WDL_THRESHOLD -1000.0  // i.e. do the WDL search anyway

/* Whether to use mobility computed from the current position for both sides (1)
  or use the mobility of the previous position for the side that is not doing
  the current move (0) */
// Default value is 1 (experimentally determined & standard approach)
// It has been hard-set to 1 in minimax.cpp (evaluateBoard())
#define USE_CURRENT_MOBILITY_FOR_BOTH_SIDES   1

/* New end of game solver */
#define USE_NEW_END_GAME_SOLVER           1
#define MAX_EMPTIES_FOR_END_GAME_SOLVER   32
#define MIN_EMPTIES_FOR_END_GAME_SOLVER   6
// different types ordering methods
#define MIN_EMPTIES_FOR_SEARCH_IN_END_SOLVE   18 // seems useful in some/many cases
#define MIN_EMPTIES_FOR_EVAL_IN_END_SOLVE     17 // slight helpful
#define MIN_EMPTIES_FOR_MOBILITY              8
#define MIN_EMPTIES_FOR_PARITY                5

/* Count for stable discs (might affect speed) */
#define USE_STABLE_COUNT      1
#define STABLE_DISC_WEIGHT    0.75
// The value of stable disc on edge might be balanced out by the potential of attacking
// along an empty edge (if they are not there).
#define EDGE_STABLE_DISCS_WEIGHT_FACTOR         0.2
// Extra bonus when it gets to line 3 -- since it is usually very good sign.
#define THIRD_LINE_STABLE_DISCS_WEIGHT_FACTOR   1.5

/* For control of deeper search at some stage (just some experiment, usually not used */
#define EXTRA_PLIES_START   30

/* For time-based cutoff -- support for GUI */
#define TIME_CHECK_INTERVAL     1024    // Must be a power of 2!

/* Whether Andersson/Smith's end game solver can be called with -S switch.
  The current end game solver (since 0.01n)  is complete based on the idea of Andersson's
  solver (mobility, parity), and is about same speed as the later. Therefore
  it is no longer necessary to keep keep the external end game solver.
  To enable it, change the value of ANDERSSON_ENABLED to 1 and make the proper
  change to Makefile */
#define ANDERSSON_ENABLED 0

#if ANDERSSON_ENABLED
#include "endgamecx.h"
#endif


/* test/debug */
#define DEBUG_MINIMAX 0
#define AB_DEBUG 0
#define DEEP_DEBUG 0
#define CURRENT_DEBUG 0
#define CURRENT_DEBUG2 0
#define LEGAL_MOVES_DEBUG 0


/* ---- produce some statistics --- */
// actually counts also nodes searched and positions evaluated
#define COUNT_PRUNING 1



extern char searchDepth;
extern char originalSearchDepth;
extern char bruteForceDepth; // for approaching the end of game.

extern bool winLarge;
extern bool loseSmall;
extern char randomnessLevel;
extern bool useAndersson;  // use Andersson's sophisticated end game solver
extern bool extraPlies;

extern bool boardFlipped;
extern bool showDots;  // simply for output
extern bool showStatistics;

extern int guiMode;
extern unsigned int timeLimit;

// for the new end of game solver
struct EmptyList {
  EmptyList *prev;
  EmptyList *next;
  int square;
};

// Interface function to game
char getMinimaxMove(Board *board, bool *lm);


#endif
