#include <stdio.h>

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

int main(int argc, char const *argv[]) {
  //Código de teste
  int vec1[] = {0,1,2,3,4,5,6,7};
  int vec2[] = {0,1,2,3,4,5,6,1};
  printf("VEC1 = %d\n",is_sequence(vec1,8));
  printf("VEC2 = %d\n",is_sequence(vec2,8));
  return 0;
}
