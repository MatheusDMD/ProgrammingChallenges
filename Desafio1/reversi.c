#include <stdio.h>

int opponent(int player);
int play(int player, int board[8][8], int *p_row, int *p_col);
 check_sum_probs(int player, int board[8][8], int x, int y);

                    // 0   1   2   3   4   5   6   7
int weights[8][8]={ {120,-20, 20,  5,  5, 20,-20,120},//0
                    {-20,-40, -5, -5, -5, -5,-40,-20},//1
                    { 20, -5, 15,  3,  3, 15, -5, 20},//2
                    {  5, -5,  3,  3,  3,  3, -5,  5},//3
                    {  5, -5,  3,  3,  3,  3, -5,  5},//4
                    { 20, -5, 15,  3,  3, 15, -5, 20},//5
                    {-20,-40, -5, -5, -5, -5,-40,-20},//6
                    {120,-20, 20,  5,  5, 20,-20,120}};//7

int play(int player, int board[8][8], int *p_row, int *p_col) {

  int opp = opponent(player);
  int prob = 0;
  int found = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == player) {
        int x = i;
        int y = j;
        printf("player pos (%d,%d)\n",x,y);
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
                printf("valid pos(%d,%d)\n", new_x, new_y);
                int new_prob = check_sum_probs(player, board, new_x, new_y);
                printf("SUM PROB %d\n",new1_prob );
                //int new_prob = weights[new_x][new_y];
                if(new_prob > prob){
                  prob = new_prob;
                  *p_row = new_x;
                  *p_col = new_y;
                  found = 1;
                }
                break;
              }
            }
          }
        }
      }
    }
  }
  printf("final: %d,%d\n", *p_row,*p_col);
  return found;
}

int check_sum_probs(int player,int board[8][8], int x, int y){
  int opp = opponent(player);
  int new_prob = 0;
  int prob_count = 0;
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
          prob_count += weights[new_x][new_y];
          val_dir = 1;
        }
        else if(val_dir == 1 && pos == player){
          printf("valid pos(%d,%d)\n", new_x, new_y);
          new_prob += prob_count;
          prob_count = 0;
          break;
        }
      }
    }
  }
  return new_prob;
}

int opponent(int player){
  if(player == 0){
    return 1;
  }else{
    return 0;
  }
  return -1;
}

int main(int argc, char const *argv[]) {
  int init_board[8][8] = {
  // 0   1   2   3   4   5   6   7
   {-1, -1, -1, -1, -1, -1, -1, -1},//0
   {-1, -1, -1, -1, -1, -1, -1, -1},//1
   {-1, -1, -1, -1, -1, -1, -1, -1},//2
   {-1, -1, -1,  0,  1, -1, -1, -1},//3
   {-1, -1, -1,  1,  0,  1, -1, -1},//4
   {-1, -1, -1, -1, -1,  0, -1, -1},//5
   {-1, -1, -1, -1, -1, -1, -1, -1},//6
   {-1, -1, -1, -1, -1, -1, -1, -1},//7
};
  int  player = 1;
  int x = 0;
  int y = 0;
  int *p_row;
  int *p_col;
  p_row = &x;
  p_col = &y;
  play(player, init_board, p_row, p_col);
  return 0;
}
