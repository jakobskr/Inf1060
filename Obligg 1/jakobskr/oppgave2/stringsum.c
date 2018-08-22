#include <stdio.h>

int stringsum(char* s) {
  int i = 0;
  int out = 0;
  while(s[i] != '\0') {
    if(96 < s[i] && s[i] < 123) {
      out = out + (int) s[i] - 96;
    }
    else if(64 < s[i] && s[i] < 91) {
      out = out + (int) s[i] - 64;
    }
    else {
      return -1;
    }
    i++;
  }
  return out;
}
