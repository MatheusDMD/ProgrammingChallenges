#include <stdio.h>
#include <string.h>

void del_extension(char str[]){
  int len_str, pos;
  char dest[50];
  len_str = strlen(str);
  pos = len_str;
  for (int i = 0; i < len_str; i++) {
    if(str[i]=='.'){
      pos = i;
      printf("%d\n", pos);
    }
  }
  for (int i = 0; i < pos; i++) {
    dest[i] = str[i];
  }
  printf("%s\n", dest);
}

void main(){
  char file[50];
  printf("%s\n", "Escreva o nome do arquivo do qual quer tirar a extensão");
  scanf("%s", file);

  del_extension(file);

}
