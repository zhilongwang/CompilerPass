#include<stdio.h>

int v;
int main()
{
  int i = 3;
  if (i>4) {
    v += 3;
  }else{
    v -=3;
  }

  printf("%d\n", v);

  return 0;
}
