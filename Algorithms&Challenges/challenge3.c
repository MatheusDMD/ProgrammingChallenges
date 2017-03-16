#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAX_NUMBER 100

void selection_sort(int vec[], int size){
  int temp_pos;
  for(int j = 0; j < size; j++) {
    /*O for externo indica a posição em que os valores desordenados estão.*/
    int temp = vec[j];
    temp_pos = j;
    /*Define também uma temp para o valor do último número desordenado e sua posição,
      referência base como o menor número até o momento*/
    for(int i = j; i < size; i++) {
      /*O indice iterador do for interno começa no primeiro desordenado*/
      if(vec[i] < temp) {
        /*E busca pelo menor valor no vetor*/
        temp = vec[i];
        temp_pos = i;
        /*Redefine o menor valor na temp e sua posição*/
      }
    }
    vec[temp_pos] = vec[j];
    vec[j] = temp;
    /*Troca os valores de posição, mantendo assim o menor dos desordenados à esquerda. Portanto, ordenado.*/
  }
}

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

  printf("%s\n", "Orinal");
  generate_vector(vec,size);
  for (int i = 0; i < size; i++) {
    printf("%d,", vec[i]);
  }
  printf("\n%s\n", "Sorted");
  selection_sort(vec,size);
  for (int i = 0; i < size; i++) {
    printf("%d,", vec[i]);
  }
  return 0;
}
