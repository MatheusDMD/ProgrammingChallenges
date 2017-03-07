#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAX_NUMBER 100

void insertion_sort(int vec[], int size){
  int temp,i;
  for (int i = 1; i <= size - 1; i++) {
    temp = vec[i];
    for (int j = i-1; j >= 0; j--) {
      if(vec[j] > temp){
        vec[j] = vec[j-1];
      }else{
        vec[j] = temp;
      }
    }
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
  insertion_sort(vec,size);
  for (int i = 0; i < size; i++) {
    printf("%d,", vec[i]);
  }
  return 0;
}
