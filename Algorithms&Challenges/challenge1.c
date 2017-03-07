#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAX_NUMBER 100

void generate_vector(int vec[], int size){
  /*Função que recebe um vetor e seu comprimento.
    E preenchem seu conteúdo com inteiros aleatórios*/
  for (int i = 0; i < size; i++) {
    //Preenche o vetor com ints aleatórios.
    vec[i] = rand() % MAX_NUMBER;
  }
}


int main(int argc, char const *argv[]) {
  //Código de teste
  int size = 10;
  int vec[size];
  srand(time(NULL));
  
  printf("%s\n", "Random int vector");
  generate_vector(vec,size);
  for (int i = 0; i < size; i++) {
    printf("%d,", vec[i]);
  }
  return 0;
}
