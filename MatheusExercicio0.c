#include <stdio.h>

void main(){
  int number_of_numbers;
  int sum;
  int media;
  int n;

  printf("%s\n", "Digite o numero de numeros que deseja obter a media:");
  scanf("%d", &number_of_numbers);
  printf("\n" );
  for (size_t i = 1; i <= number_of_numbers; i++) {
    printf("\n%s\n","Digite o próximo numero");
    scanf("%d", &n);
    sum += n;
  }
  printf("\nA média inteira dos números %d inseridos é: %d\n", number_of_numbers,sum/number_of_numbers);

}
