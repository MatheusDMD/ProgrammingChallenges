/*********
  othello.hpp
  */

#ifndef _OTHELLO_HPP_
#define _OTHELLO_HPP_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "game.hpp"

// It seems that even number search depth plays much better than odd ones
#define SEARCH_STOOGE             1
#define SEARCH_MINDLESS           2
#define SEARCH_NOVICE             4
#define SEARCH_BEGINNER           6
#define SEARCH_AMATEUR            8
#define SEARCH_EXPERIENCED        10

#define BRUTE_FORCE_STOOGE        8
#define BRUTE_FORCE_MINDLESS      10
#define BRUTE_FORCE_NOVICE        12
#define BRUTE_FORCE_BEGINNER      14
#define BRUTE_FORCE_AMATEUR       16
#define BRUTE_FORCE_EXPERIENCED   18

#define USE_ANDERSSON_THRESHOLD   64  // change to smaller value if want to turn it on auto.

// Default values
#define DEF_WIN_LARGE         1
#define DEF_LOSE_SMALL        1
#define DEF_IS_FLIPPED        0
#define DEF_RANDOMNESS_LEVEL  1

#define PROGRAM_NAME "MiniOthello"
#define VERSION "0.03b1"

const char *PROGRAM_INFO = "%s %s   Written by: Yunpeng Li (yl@middlebury.edu)\
  \nFor help type \"help\", or use \"%s --help\" at command-line for options.\n";

const char *OPTIONS_HELP = "\nUsage: %s [options]\n\
  \nPlayer Options: \n\
    --man <side>      Human plays which side  (default black)\n\
    -b    <side> = black       Human plays black                             \n\
    -w             white       Human plays white                             \n\
    -a             both        Human plays both sides, i.e. no computer      \n\
    -n             neither     Human plays neither side, i.e. two computers  \n\
  \nComputer A.I. Options (higher = stronger, slower): \n\
    -D,  --depth      <depth>   Mid-game search depth\n\
    -E,  --exact      <depth>   End game brute force depth\n\
    -W,  --win-max    <0/1>     Computer tries to maximze a win or minimize\n\
                               a loss (default %d)\n\
  \nOther Options: \n\
    -r,  --randomness  <0-9>    Randomness level (default %d)\n\
    -f,  --flip-board           Start with the game board mirrored\n\
    -sp, --self-play   <n>      Self-play for n moves, then hand over to human\n\
    -ld, --load     <filename>  Load a game at starting up\n\
    -p,  --show-progress        Show computer's thinking progress\n\
    -t,  --show-time            Show the time taken for each computer move\n\
    -st, --show-statistics      Show evaluation information\n\
    -nl, --show-no-legal-moves  Don't show legal moves as '+' on the board\n\
         --quiet                Don't print out the informational message\n\
    -?,  --help                 Display this message\n\
    -h,  --more-help            Display control commands\n\
  \nShort-cut switches: \n\
    -L0, --novice      ==   -D %d  -E %d \n\
    -L1, --beginner    ==   -D %d  -E %d \n\
    -L2, --amateur     ==   -D %d  -E %d    (default level)\n\
    -L3, --exprienced  ==   -D %d  -E %d   (NOT recommended on slow computers)\n\
    -L<n>+ adds 2 to the depth of end game search, e.g. -L2+ == -D %d -E %d\n\
    On the easy end: \"--mindless\" == \"-D %d -E %d\", \"--stooge\" == \"-D %d -E %d\"\n\
  \nExamples: \n\
    %s --novice\n\
    %s --depth 8 --end-game 16 -w -W 1\n";

const char *SETTINGS = "\nGame started with the following settings:\n\
  Black:                              %s\n\
  White:                              %s\n\
  Mid-game search depth:              %d\n\
  End-game exact moves:               %d\n\
  Maximize winning score:             %s\n\
  Randomness level:                   %d\n";

const char *HELP_MESSAGE = 
"Symbols: 'X' = Black, 'O' = White, '.' = Empty,  '%s' = Possible move\
  \nTo move, enter a letter followed by a number.\
  \nExamples: d5, G6, c 8, a-1\
  \n\
  \nSpecial Command (during a game):\n\
  undo        --  Undo the last move\n\
  redo        --  Redo the move that was undone\n\
  undo <n>    --  Undo the last n moves\n\
  redo <n>    --  Redo n moves\n\
  undo all    --  Undo all moves\n\
  redo all    --  Redo all moves\n\
  pm [symbol] --  Turn off/on display of possible moves [as 'symbol'] \n\
  diff        --  Display the current A.I. level\n\
  swapsides   --  Swap the players' colors\n\
  skip        --  Make the play the current move for the human player\n\
  handover    --  Let computer play the rest of game for the human player\n\
  save        --  Save current game\n\
  load        --  Load a saved game\n\
  mopt        --  Display more options\n\
  help        --  Display this message\n";
  
    
void printHead(char *arg0);
void printUsage(char *arg0);
void printThanks();
void printSettings(char player1, char player2);

void catch_int(int signum);

#endif
