#include <stdio.h>

void troca(short *a, short *b){
  short temp;
  temp = *a;
  *a = *b;
  *b = temp;
}

void main(){
  short a = 1;
  short b = 2;
  troca(&a,&b);
  printf("%d\n", a);
  printf("%d\n", b);
}
