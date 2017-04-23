#include <stdio.h>
#include <stdlib.h>

int search(int v[], int n, int q);
int gangnam_search(int v[], int r, int l, int q);

int search(int v[], int n, int q) {
  return gangnam_search(v, n, 0, q);
}

int gangnam_search(int v[], int r, int l, int q){
  int m1 = l + ((r - l) / 3);
  int m2 = l + 2 * ((r - l) / 3) + 1;
  if(l >= rs) {
  return -1;
  }
  if(v[m1] == q) {
    return m1;
  }
  if(v[m2] == q){
    return m2;
  }
  if(v[m1] < q && v[m2] < q) {
    return gangnam_search(v, r, m2, q);
  }
  else if(v[m1] > q && v[m2] > q){
    return gangnam_search(v, m1, l, q);
  }
  else if(v[m1] < q && v[m2] > q){
    return gangnam_search(v, m2, m1, q);
  }
  else{
    return -1;
  }
}
