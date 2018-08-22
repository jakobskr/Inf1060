#include <string.h>

int distance_between(char* s, char c) {
  int i = 0;
  int forste = -1;
    for(i = 0; i < strlen(s); i++) {
    if (s[i] == c) {
      if (forste == -1) {
        forste = i;
      }
      else {
        return i - forste;
      }
    }
  }
  return -1;
}
