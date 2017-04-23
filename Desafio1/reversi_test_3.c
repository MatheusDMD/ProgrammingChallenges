#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


int opponent(int player);
int play(int player, int board[8][8], int *p_row, int *p_col);
 check_sum_probs(int player, int board[8][8], int x, int y);

                  // 0   1   2   3   4   5   6   7
int weights[8][8]={{120,-20, 20,  5,  5, 20,-20,120},//0
                   {-20,-40, -5, -5, -5, -5,-40,-20},//1
                   { 20, -5, 15,  3,  3, 15, -5, 20},//2
                   {  5, -5,  3,  3,  3,  3, -5,  5},//3
                   {  5, -5,  3,  3,  3,  3, -5,  5},//4
                   { 20, -5, 15,  3,  3, 15, -5, 20},//5
                   {-20,-40, -5, -5, -5, -5,-40,-20},//6
                   {120,-20, 20,  5,  5, 20,-20,120}};//7

int play(int player, int board[8][8], int *p_row, int *p_col) {

  int opp = opponent(player);
  int prob = INT_MIN;
  int num = INT_MIN;
  int found = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == player) {
        int x = i;
        int y = j;
        for (int k = -1; k <= 1; k++) {
          for (int l = -1; l <= 1; l++) {
            int new_x = x+l;
            int new_y = y+k;
            int val_dir = 0;
            while(new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8){
              int pos = board[new_x][new_y];
              if(pos == player || (val_dir == 0 && pos == -1)){
                val_dir = 0;
                break;
              }
              else if(pos == opp){
                new_x += l;
                new_y += k;
                val_dir = 1;
              }
              else if(val_dir == 1 && pos == -1){
                int new_prob = weights[new_x][new_y];
                if(new_prob > prob){
                  prob = new_prob;
                  *p_row = new_x;
                  *p_col = new_y;
                  found = 1;
                }
                if(new_prob == prob){
                  int new_num = check_sum_probs(player, board, new_x, new_y);
                  if(new_num > num){
                    prob = new_prob;
                    *p_row = new_x;
                    *p_col = new_y;
                    found = 1;
                  }
                }
                break;
              }
            }
          }
        }
      }
    }
  }
  return found;
}

int check_sum_probs(int player,int board[8][8], int x, int y){
  int opp = opponent(player);
  int num = 0;
  for (int k = -1; k <= 1; k++) {
    for (int l = -1; l <= 1; l++) {
      int new_x = x+l;
      int new_y = y+k;
      int val_dir = 0;
      while(new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8){
        int pos = board[new_x][new_y];
        if(pos == -1 || (val_dir == 0 && pos == player)){
          val_dir = 0;
          break;
        }
        else if(pos == opp){
          new_x += l;
          new_y += k;
          num += 1;
          val_dir = 1;
        }
        else if(val_dir == 1 && pos == player){
          num += 1;
          break;
        }
      }
    }
  }
  return num;
}

int opponent(int player){
  if(player == 0){
    return 1;
  }else{
    return 0;
  }
  return -1;
}
