#include <stdio.h>

void hanoi(int n, char src, char aux, char dst){
  if(n > 1){
    hanoi(n-1,src,dst,aux);
  }
    printf("mova disco %d de %c para %c\n", n, src, dst);
  if(n > 1){
    hanoi(n-1,aux,src,dst);
  }
}

int main(int argc, char const *argv[]) {
  int n = 5;
  char src = 'A';
  char aux = 'B';
  char dst = 'C';
  hanoi(n, src, aux, dst);
  return 0;
}
