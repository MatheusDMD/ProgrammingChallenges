

#include <time.h>
#include <sys/times.h>
#define SIZE 10000
#define MAX_NUMBER 100


int main(int argc, char **argv) {

  struct tms t;
  clock_t begin, end;
  double elapsed;
  int vec1[SIZE],vec2[SIZE],vec3[SIZE],vec4[SIZE],vec5[SIZE],vec6[SIZE],vec7[SIZE],vec8[SIZE],vec9[SIZE],vec10[SIZE];

  void generate_vector(int vec[], int size){
    /*Função que recebe um vetor e seu comprimento.
      E preenchem seu conteúdo com inteiros aleatórios*/
    for (int i = 0; i < size; i++) {
      //Preenche o vetor com ints aleatórios.
      vec[i] = rand() % MAX_NUMBER;
    }
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

  void selection_sort(int vec[], int size){
    int temp_pos;
    for(int j = 0; j < size; j++) {
      /*O for externo indica a posição em que os valores desordenados estão.*/
      int temp = vec[j];
      int temp_pos = j;
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
  void bubble_sort_recursive (int v[],int size){
    if(size > 1){
      for(int i = 1; i < size; i++) {
        if(v[i - 1] > v[i]) {
          int temp = v[i - 1];
          v[i - 1] = v[i];
          v[i] = temp;
        }
      }
      bubble_sort_recursive(v,size-1);
    }
  }
  void selection_sort_recursive(int vec[], int size){
    int temp = vec[0];
    int temp_pos = 0;
    if(size > 1){
      for(int i = size-1; i >= 0; i--) {
        /*O indice iterador do for interno começa no primeiro desordenado*/
        if(vec[i] > temp) {
          /*E busca pelo menor valor no vetor*/
          temp = vec[i];
          temp_pos = i;
          /*Redefine o menor valor na temp e sua posição*/
        }
      }
      vec[temp_pos] = vec[size-1];
      vec[size-1] = temp;
      selection_sort_recursive(vec,size-1);
    }
    /*Troca os valores de posição, mantendo assim o menor dos desordenados à esquerda. Portanto, ordenado.*/
  }
  int elapsed1 = 0;
  int elapsed2 = 0;
  int elapsed3 = 0;
  int elapsed4 = 0;
  int elapsed5 = 0;

  for (int i = 0; i < 10; i++) {
    /* code */
    generate_vector(vec1,SIZE);
    generate_vector(vec2,SIZE);
    generate_vector(vec3,SIZE);
    generate_vector(vec4,SIZE);
    generate_vector(vec5,SIZE);
    generate_vector(vec6,SIZE);
    generate_vector(vec7,SIZE);
    generate_vector(vec8,SIZE);
    generate_vector(vec9,SIZE);
    generate_vector(vec10,SIZE);

    times(&t);
    begin = t.tms_utime + t.tms_stime;
    //bubble sort
    bubble_sort(vec1,SIZE);

    times(&t);
    end = t.tms_utime + t.tms_stime;

    printf("bubble sort: %lf\n", elapsed = (double) (end - begin) / CLOCKS_PER_SEC);
    elapsed1 += elapsed;

    times(&t);
    begin = t.tms_utime + t.tms_stime;
    //selection sort
    selection_sort(vec2,SIZE);

    times(&t);
    end = t.tms_utime + t.tms_stime;

    printf("selection sort: %lf\n", elapsed = (double) (end - begin) / CLOCKS_PER_SEC);
    elapsed2 += elapsed;

    times(&t);
    begin = t.tms_utime + t.tms_stime;
    //inserction sort recursive
    insertion_sort(vec3,SIZE);

    times(&t);
    end = t.tms_utime + t.tms_stime;

    printf("inserction sort recursive: %lf\n", elapsed = (double) (end - begin) / CLOCKS_PER_SEC);
    elapsed3 += elapsed;

    times(&t);
    begin = t.tms_utime + t.tms_stime;
    //bubble sort recursive
    bubble_sort_recursive(vec4,SIZE);

    times(&t);
    end = t.tms_utime + t.tms_stime;

    printf("bubble sort recursive: %lf\n", elapsed = (double) (end - begin) / CLOCKS_PER_SEC);
    elapsed4 += elapsed;

    times(&t);
    begin = t.tms_utime + t.tms_stime;
    //selection sort recursive
    selection_sort_recursive(vec5,SIZE);

    times(&t);
    end = t.tms_utime + t.tms_stime;

    printf("selection sort recursive: %lf\n", elapsed = (double) (end - begin) / CLOCKS_PER_SEC);
    elapsed5 += elapsed;
  }
  printf("\nselection sort recursive: %lf\n bubble sort recursive: %lf\ninserction sort recursive: %lf\nselection sort: %lf\nbubble sort: %lf\n", elapsed5,elapsed4,elapsed3,elapsed2,elapsed1);
}
