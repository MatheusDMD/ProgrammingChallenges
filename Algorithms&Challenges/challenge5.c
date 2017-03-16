#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAX_NUMBER 100

void insertion_sort(int vec[], int size){
  int temp,i;
  if(size > 1){
    insertion_sort(vec,size-1);//chama a função até que o valor -> size = 1.
    /*Assim, quando o valor de size cresce cada retorno da recursão*/
    temp = vec[size-1];
    i = size - 2;
    /*Define a variável temporária (temp) como a seguinte à parte "ordenada"
      e o iterador (i) como o último indice ordenado.*/
    while (i >= 0 && vec[i]>temp) {
      /* Se o valor analisado é >= 0, ou seja, ainda pode ser que o valor temp seja maior do que próximo/
        ou se o valor do índice analisado é maior que o valor da temp.*/
      vec[i+1] = vec[i];
      /* Caso o valor seja verdadeiro,copia o valor no índice i para o elemento a sua direita.*/
      i = i - 1;
      /*Reduz o valor do índice para a próxima análise comparativa*/
    }
    vec[i+1] = temp;
    /*preenche a posição encontrada para o valor temp com seu valor.*/
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

void generate_vector(int vec[], int size){
  /*Função que recebe um vetor e seu comprimento.
    E preenchem seu conteúdo com inteiros aleatórios*/
  for (int i = 0; i < size; i++) {
    //Preenche o vetor com ints aleatórios.
    vec[i] = rand() % MAX_NUMBER;
  }
}

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

int main(int argc, char const *argv[]) {
  int size = 1000;
  int times = 100;
  int vec[size], wrong_n = 0;
  srand(time(NULL));

  printf("%s\n", "Bubble Sort:");
  for (int i = 0; i < times; i++) {
    generate_vector(vec,size);
    optimized_bubble_sort(vec,size);
    //bubble_sort(vec,size);
    if(!is_sequence(vec,size)){
      wrong_n++;
    }
  }
  printf("O Número de erros é: %d\n", wrong_n);
  wrong_n = 0;
// ---------------------------------------------
  printf("%s\n", "Selection Sort:");
  for (int i = 0; i < times; i++) {
    generate_vector(vec,size);
    selection_sort(vec,size);
    if(!is_sequence(vec,size)){
      wrong_n++;
    }
  }
  printf("O Número de erros é: %d\n", wrong_n);
  wrong_n = 0;
// ---------------------------------------------
  printf("%s\n", "Insertion Sort:");
  for (int i = 0; i < times; i++) {
    generate_vector(vec,size);
    insertion_sort(vec,size);
    if(!is_sequence(vec,size)){
      wrong_n++;
    }
  }
  printf("O Número de erros é: %d\n", wrong_n);
  wrong_n = 0;

  return 0;
}
