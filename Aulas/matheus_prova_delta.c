#include <stdio.h>

/*

O código a seguir tem complexidade O(1), portanto O(n), sempre possuindo 10 chamadas recursivas.
Isso ocorre devido ao fato de que a chamada é feita não trivialmente e o caso base
impede que o resultado estoure! Executando, sempre 10 chamadas recursivas.
Assim, mantendo a funcao com O(1).

*/

void rec_funcao(int n, int max){
  if(n == max){
    return;
  }
  printf("%d\n", n);
  n++;
  rec_funcao(n, max);
}

void funcao(int n){
  rec_funcao(n, n+10);
}

int main(int argc, char const *argv[]) {
  //Trecho para testes. Deve imprimir 10 inteiros independentemente da entrada.
  funcao(0);
  return 0;
}
