#include <stdio.h>

int is_fibonacci(int vec[],unsigned int size){
  if(size < 3){
    return 1;
  }
  for (int i = 0; i < size-2; i++) {
    if(vec[i] + vec[i+1] != vec[i+2]){
      return 0;
    }
  }
  return 1;
}

void main(){
  int vec1[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
  int size1 = 10;
  int vec2[] = {3, -5, -2, -7, -9, -16, -25};
  int size2 = 7;
  int vec3[] = {1, 2, 3, 4, 5, 6};
  int size3 = 5;
  printf("%s\n", "{0, 1, 1, 2, 3, 5, 8, 13, 21, 34}");
  printf("%d\n", is_fibonacci(vec1,size1));
  printf("%s\n", "{3, -5, -2, -7, -9, -16, -25}");
  printf("%d\n", is_fibonacci(vec2,size2));
  printf("%s\n", "{1, 2, 3, 4, 5, 6}");
  printf("%d\n", is_fibonacci(vec3,size3));
}
