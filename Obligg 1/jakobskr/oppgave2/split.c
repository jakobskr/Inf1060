#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char** split(char *s) {
    printf("hell" );
    int counter,i;
    int start = 0;
    for (i = 0; i < strlen(s); i++) {
      if (s[i] == ' ') {
        counter++;
      }
    }
    char **ret;

    ret = malloc(sizeof(char*) * counter + 2);
    ret[0] =(char*) malloc(sizeof(char) * strlen(s));
    int k,j,count = 0;
    for (k = 0; k < strlen(s); k++) {
      if (s[k] == ' ' || s[k] == '\0') {
        ret[count][j] = '\0';
        count++;
        ret[count] = malloc(strlen(s));
        j = 0;
      }
      else {
        ret[count][j] = s[k];
        j++;
      }
    }
    ret[count + 1] = NULL;
    printf("%s\n", ret[0]);
    return ret;
  }
