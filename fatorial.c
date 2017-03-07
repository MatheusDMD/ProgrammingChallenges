#include <stdio.h>

int fatorial(int n){
  if(!n){
    return 1;
  }
  return n*fatorial(n-1);
}

int potencia(int b, int e){
  if(e == 1){
    return b;
  }
  return b*potencia(b,e-1);
}

int soma(int v[],int n){
  if(n == 1){
    return v[0];
  }
  return v[n - 1] + soma(v,n-1);
}

int main(int argc, char const *argv[]) {
  int v[] = {1,2,3,4};
  int size = 4;
  printf("\n Result: %d\n", soma(v, size));
  return 0;
}
