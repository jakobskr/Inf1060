#include <string.h>
#include <stdlib.h>
int distance_between(char* s, char c);

char* string_between(char* s, char c) {
  int i = 0;
  int forste = -1;
  for(i = 0; i < strlen(s); i++) {
    if (s[i] == c) {
      if (forste == -1) {
        forste = i;
      }
      else {
        int len = distance_between(s, c) - 1;
        int counter = 0;
        char temp[len];
        char *ret = malloc(len + 1);
        for (i; forste < i - 1; forste++) {
          temp[counter] = s[forste + 1];
          counter++;
        }
        temp[len] = '\0';
        return strcpy(ret,temp);
      }
    }
  }
  return NULL;
}
