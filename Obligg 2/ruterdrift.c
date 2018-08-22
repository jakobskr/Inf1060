#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ruter {
  unsigned char ruterId;
  unsigned char FLAGG;
  unsigned char infoLengde;
  char* modell;
}ruter;

void init_ruter (ruter* rt, unsigned char id, unsigned char FLAGG, unsigned char lengde, char* info) {
  rt-> ruterId = id;
  rt-> FLAGG = FLAGG;
  rt-> infoLengde = lengde;
  rt-> modell = malloc(lengde + 1);
  strcpy(rt->modell,info);
}

void printInfo(ruter* rt) {
  printf("%s\n",rt->modell );
}

void printFLAGG(ruter* rt) {
  printf("RuterId: %d"
  "\nAktiv   [%d]"
  "\nTrådløs [%d]"
  "\n5Ghz    [%d]"
  "\nEndret  [%d]\n"
  ,rt->ruterId,(rt->FLAGG >> 0) & 1,(rt->FLAGG >> 1) & 1,(rt->FLAGG >> 2) & 1,rt->FLAGG >> 4);
}

void printRuter(ruter* rt) {
  printInfo(rt);
  printFLAGG(rt);
}

void endreFlagg (ruter * rt) {
  int mask = 240;
  int valg;
  int a = (mask & rt->FLAGG) >> 4;
  if (a == 15) {
    printf("kan ikke endres mer\n");
    return;
  }
  else {
    printFLAGG(rt);
    printf("Hva vil du endre?\n1: Aktiv \n2: Trådløs \n3: 5GHz\n");
    scanf ("%d",&valg);
     if (valg == 1) {
       rt->FLAGG ^= 1 << 0;
       a++;
     }
     else if (valg == 2) {
       rt->FLAGG ^= 1 << 1;
       a++;
     }
     else if (valg == 3) {
       rt->FLAGG ^= 1 << 2;
       a++;
     }
     else {
       printf("Ugyldig valg\n");
     }
     rt->FLAGG = (rt->FLAGG & 15) | a << 4;
     }
}

void printID (ruter * rt) {
  printf("%d\n", rt->ruterId);
}

#define ARRAY_SIZE 256
ruter* routers[ARRAY_SIZE] = {NULL};
int ruterTeller = 0;

void filinnlesning(char* filnavn) {
  unsigned char id;
  unsigned char FLAGG;
  unsigned char lengde = 0;
  char* info = 0;
  FILE *file = fopen(filnavn, "r");
  ruterTeller = (int) fgetc(file);
  if (ruterTeller > ARRAY_SIZE) {
    ruterTeller = ARRAY_SIZE - 1;//sletter ekstra rutere :)
  }
  fseek(file,5,SEEK_SET);//hopper fram til id-byten
  for (int i = 0; i < ruterTeller; i++) {
  id = fgetc(file);
  FLAGG = fgetc(file);
  lengde = fgetc(file);
  info = malloc((lengde));
  fread(info,lengde,1,file);
  info[lengde - 1] = '\0';
  //printf("%u, %d, %u, %s\n",id,FLAGG,lengde,info);
  routers[id] = malloc(sizeof(ruter));
  init_ruter(routers[id],id,FLAGG,lengde,info);
  free(info);
  }
  fclose(file);
}

void skrivTilFil(char* filnavn) {
  FILE *file = fopen(filnavn, "w");
  fseek(file, 0, SEEK_SET);
  char a = 0xa;
  fwrite(&ruterTeller,sizeof(ruterTeller),1,file);
  fwrite(&a,sizeof(a),1,file);
  for (int i = 0; i < ARRAY_SIZE; i++) {
    if (routers[i] == NULL) {}
    else {
      fwrite(&routers[i]->ruterId,sizeof(routers[i]->ruterId),1,file);
      fwrite(&routers[i]->FLAGG,sizeof(routers[i]->FLAGG),1,file);
      fwrite(&routers[i]->infoLengde,sizeof(routers[i]->infoLengde),1,file);
      for(int j = 0; j < routers[i]->infoLengde-1; j++){
        fwrite(&routers[i]->modell[j],sizeof(routers[i]->modell[j]),1,file);
      }
      fwrite(&a,sizeof(a),1,file);
    }
  }
  fclose(file);
}

void delete(int id) {
  free(routers[id]->modell);
  free(routers[id]);
  routers[id]= NULL;
  ruterTeller = ruterTeller - 1;
  printf("Fjernet ruter nr %d\n", id);
}

void createRuter(char id, ruter* rt) {
  char buf[64];
  rt->ruterId = id;
  rt->modell = malloc(254);
  while ( getchar() != '\n' ){}
  printf("Skriv inn modellnavn\n" );
  fgets(rt->modell,253,stdin);
  rt->infoLengde = (unsigned char) strlen(rt->modell);
  printf("%d\n", rt->infoLengde);
  rt->FLAGG = 0;
  printf("Er ruteren aktiv? [Y/N]\n");
  fgets(buf, sizeof(buf), stdin);
  if (buf[0] == 'y' || buf[0] == 'Y') {
    rt->FLAGG ^= 1 << 0;
  }

  printf("Er ruteren trådløs? [Y/N]\n");
  fgets(buf, sizeof(buf), stdin);
  if (buf[0] == 'y' || buf[0] == 'Y') {
    rt->FLAGG ^= 1 << 1;
  }

  printf("Støtter ruteren 5GHz? [Y/N]\n");
  fgets(buf, sizeof(buf), stdin);
  if (buf[0] == 'y' || buf[0] == 'Y') {
    rt->FLAGG ^= 1 << 2;
  }
  ruterTeller++;
}

void printAlleRutere() {
  for (int i = 0; i < ARRAY_SIZE; i++) {
    if (routers[i] != NULL) {
      printf("ID : %d Navn: %s\n", routers[i]->ruterId, routers[i]->modell);
    }
  }
}

void endreModell(ruter* rt) {
  free(rt->modell);
  rt->modell = malloc(254);
  while ( getchar() != '\n' ){}
  printf("Skriv inn det nye modellnavn\n" );
  fgets(rt->modell,253,stdin);
  rt->infoLengde = (unsigned char) strlen(rt->modell);
}

//command loop
int menu() {
  int user_int = 0;
  printf("\nVelg operasjon\n"
  "1: Vis Ruter info\n"
  "2: Endre Flagg\n"
  "3: Legg til ny Ruter\n"
  "4: Fjern en ruter\n"
  "5: Endre ruter modell\n"
  "6: Print alle ruterene\n"
  "-1: Avslutt Programmet\n"
  );
  scanf("%d", &user_int);
    if (user_int == -1 || user_int == 113) {
      return 0;
    }

    else if (user_int == 1) {
      printf("Velg hvilken ruter som skal vises\n" );
      scanf("%d", &user_int);
      if (user_int == -1 || user_int >= ARRAY_SIZE ) {
        printf("Ugyldig ID\n");
      }
      else {
        if (routers[user_int] != NULL) {
          printRuter(routers[user_int]);
        }
        else{
          printf("Ingen ruter med den id'en\n");
        }
      }
    }

    else if (user_int == 2) {
      printf("Velg hvilken ruter som skal endres\n" );
      scanf(" %d", &user_int);
      if (user_int == -1 || user_int >= ARRAY_SIZE ) {
        printf("Ugyldig ID\n");
      }
      else {
        if (routers[user_int] != NULL) {
          endreFlagg(routers[user_int]);
        }
        else{
          printf("Ingen ruter med den id'en\n");
        }
      }
    }

    else if (user_int == 3) {
      printf("Gi ruteren en ID\n" );
      scanf(" %d", &user_int);
      if (user_int == -1 || user_int >= ARRAY_SIZE ) {
        printf("Ugyldig ID\n");
      }
      else {
        if (routers[user_int] == NULL) {
          routers[user_int] = malloc(sizeof(ruter));
          createRuter(user_int, routers[user_int]);
        }
        else{
          printf("Det er alt en ruter med den ID'en\n");
        }
      }
    }

    else if (user_int == 4) {
      printf("Hvilken ruter skal endres?\n");
      scanf(" %d", &user_int);
      if (user_int == -1 || user_int >= ARRAY_SIZE) {
        printf("Ugyldig ID\n");
      }
      else {
        if (routers[user_int] != NULL) {
          delete(user_int);
        }
        else{
          printf("Ingen ruter med den id'en\n" );
        }
      }
    }

    else if (user_int == 5) {
      printf("Hvilken ruter skal fjernes?\n");
      scanf(" %d", &user_int);
      if (user_int == -1 || user_int >= ARRAY_SIZE) {
        printf("Ugyldig ID\n");
      }
      else {
        if (routers[user_int] != NULL) {
          endreModell(routers[user_int]);
        }
        else{
          printf("Ingen ruter med den id'en\n" );
        }
      }
    }

    else if (user_int == 6) {
      printAlleRutere();
    }

    else {
      printf("ugyldig input\n");
      while ( getchar() != '\n' ){}//flusher user input, så programmet ikke krasjer ved feil input av bruker.
      return 1;
    }
    return 1;
  }

int main(int argX, char* argY[]) {
  if (argX != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argY[0]);
		return 0;
  }
  filinnlesning(argY[1]);
  int loop = 1;
  while (loop != 0) {
    loop = menu();
  }
  skrivTilFil(argY[1]);
  for (int i = 0; i < ARRAY_SIZE; i++) {
    if (routers[i] != NULL) {
      free((routers[i])->modell);
      free(routers[i]);
    }
  }
  printf("Programmet avsluttes nå!\n");
  //printf("%u\n", '\ns');
  return 0;
}
