#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAX_NUMBER 100

int optimized_bubble_sort(int v[], int size) {
  int switched;
  int n_iterations = 0;
  for(int j = size - 1; j >= 1; j--) {
    switched = 0;
    for(int i = 1; i <= j; i++) {
      if(v[i - 1] > v[i]) {
        int temp = v[i - 1];
        v[i - 1] = v[i];
        v[i] = temp;
        switched++;
      }
      n_iterations++;
    }
    if(!switched){
      return n_iterations;
    }
  }
  return n_iterations;
}

int bubble_sort(int v[], int n) {
  int n_iterations = 0;
  for(int j = n - 1; j >= 1; j--) {
    for(int i = 1; i <= j; i++) {
      if(v[i - 1] > v[i]) {
        int temp = v[i - 1];
        v[i - 1] = v[i];
        v[i] = temp;
      }
      n_iterations++;
    }
  }
  return n_iterations;
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
  int n_iterations;
  srand(time(NULL));

  printf("%s\n", "Orinal");
  generate_vector(vec,size);
  for (int i = 0; i < size; i++) {
    printf("%d,", vec[i]);
  }
  printf("\n%s\n", "Sorted");
  n_iterations = bubble_sort(vec,size);
  for (int i = 0; i < size; i++) {
    printf("%d,", vec[i]);
  }
  printf("\n%d\n", n_iterations);

  printf("\n%s\n", "Sorted");
  n_iterations = optimized_bubble_sort(vec,size);
  for (int i = 0; i < size; i++) {
    printf("%d,", vec[i]);
  }
  printf("\n%d\n", n_iterations);


  return 0;
}
