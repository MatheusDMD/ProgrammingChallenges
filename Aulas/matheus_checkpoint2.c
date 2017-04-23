void tres_n(int n){
  if(n == 0){
    return;
  }
  tres_n(n-1);
  tres_n(n-1);
  tres_n(n-1);
}
