#include <stdio.h>
#include <limits.h>
#include <time.h>
#define MAX_NUMBER 100

int is_sequence(int vec[], int size){
  /*Função que recebe um vetor e seu comprimento.
    E devolve 1 caso esteja em ordem crescente e 0 caso contrário.*/
  for (int i = 1; i < size; i++) {
    //Percorre o vetor buscando fatores fora de ordem.
    if(vec[i-1] > vec[i]){
      /*Caso algum item do vetor esteja desordenado, ou seja,
      o fator seguinte seja menor do que seu anterior.*/
      return 0;
    }
  }
  //Após percorrer todo o vetor, caso não seja retornado 0, o vetor está ordenado.
  return 1;
}


int mystery_sort(int vec[], int size){
  int lowest = INT_MAX;
  int highest = INT_MIN;
  int n_l = 0;
  int n_h = 0;
  for (int i = 0; i < (size - 1); i++) {
    if(is_sequence(vec,size)){
      break;
    }
    for (int j = i; j < (size - i) - 1; j++) {
      if(lowest > vec[j]){
          lowest = vec[j];
          n_l = j;
      }
      if(highest < vec[j]){
          highest = vec[j];
          n_h = j;
      }
    }
    int temp = vec[i];
    vec[i] = lowest;
    vec[n_l] = temp;

    int temp1 = vec[size - i - 1];
    vec[size - i - 1] = highest;
    vec[n_h] = temp1;

    lowest = INT_MAX;
    highest = INT_MIN;
  }
}

/////////////TEST CODE/////////////////////////////////////////////////

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
  mystery_sort(vec,size);
  for (int i = 0; i < size; i++) {
    printf("%d,", vec[i]);
  }
  return 0;
}
