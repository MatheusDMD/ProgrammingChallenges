/***
  game.hpp
  
  */

#ifndef _GAME_HPP_
#define _GAME_HPP_

#include <string.h>
#include <time.h>
#include "board.hpp"
#include "minimax.hpp"

#define HUMAN 1
#define COMPUTER 2


extern double totalTimeUsed;
extern bool showDots;
extern bool showStatistics;
extern bool showTime;
extern bool gameUnfinished;

extern char originalSearchDepth;
extern char bruteForceDepth;
extern bool useAndersson;

void playGame(Board *board);
void saveGame(Board *b, const char *name);
void saveGameNoPrompt(Board *b, const char *name);
Board* loadGame(const char *name);

#endif

