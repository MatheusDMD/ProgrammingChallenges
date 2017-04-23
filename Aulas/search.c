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


int search(int v[], int n, int q) {
  for(int i = 0; i < n; i++) {
    if(v[i] == q) {
      return i;
    }
  }
  return -1;
}

int search1(int v[], int n, int q) {
  for(int i = 0; i < n; i++) {
    if(v[i] == q) {
      return i;
    }
    if(v[i] > q) {
      return -1;
    }
  }
  return -1;
}

int search2_helper(int v[], int last, int first, int q){
  int index = (first+last)/2;
  int analized = v[index];
  while(first <= last){
    if(analized < q){
      index = search2_helper(v,last,index,q);
    }
    else if(analized > q){
      index = search2_helper(v,index,first,q);
    }
    else if(analized == q){
      return index;
    }
    else{
      return -1;
    }
    return index;
  }
  return -1;
}

int search2(int v[], int n, int q) {
  if(v[n-1] < q || v[0] > q){
    return -1;
  }
  else{
    return search2_helper(v,n,0,q);
  }
}

int main(int argc, char const *argv[]) {
  int size = 10;
  int found, found1 = 0;

  //generate_vector(vec,size);
  //selection_sort(vec,size);
  int vec[] = {1,2,3,4,6,7,8,9,10,11};
  found = search(vec,size,7);
  found1 = search2(vec,size,8);

  printf("search: %d\n", search2(vec,size,5));
  printf("search: %d\n", search2(vec,size,2));
  printf("search: %d\n", search2(vec,size,3));
  printf("search: %d\n", search2(vec,size,4));
  printf("search: %d\n", search2(vec,size,6));
  printf("search: %d\n", search2(vec,size,7));
  printf("search: %d\n", search2(vec,size,8));
  printf("search: %d\n", search2(vec,size,9));
  printf("search: %d\n", search2(vec,size,10));
  printf("search: %d\n", search2(vec,size,11));
  printf("search: %d\n", search2(vec,size,12));
  printf("search: %d\n", search2(vec,size,0));


  return 0;
}
