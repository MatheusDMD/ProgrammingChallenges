#include <stdio.h>

int quick_sort_rec(){

}

void init_order(int vec[], int size){
  int ref = vec[0];
  int i, j;
  i = 1;
  j = size - 1;
  while(i < j){
    while(vec[i] <= ref){
      i ++;
    }
    while(vec[j] > ref){
      j --;
    }
    int temp = vec[i];
    printf("%s,%d\n", "A",vec[i]);
    printf("%s,%d\n", "B",vec[j]);
    vec[i]   = vec[j];
    vec[j]   = temp;
    i ++;
    j --;
  }
}

int quick_sort(int vec[],int size){
  init_order(vec,size);
  for (int i = 0; i < size; i++) {
    printf("%d\n", vec[i]);
  }
}

int main(int argc, char const *argv[]) {
  int size = 10;
  int vec[] = {3,2,4,6,8,10,9,1,5,7};
  quick_sort(vec,size);
  return 0;
}
