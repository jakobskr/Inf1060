/** Server Program which recieves job requests from a client, then gets a job from the jobFile
 *  and then sends the job to the client
 *  Author: 15144
 */

#include "common.h"

FILE *jobFile = NULL;
int readTotal = 0;
int sd;

/**
 * attempts to convert an string to an integer
 * @param
 * string: the string we want to convert
 * port:   the pointer to where the converted int will be stored
 * @return
 * returns 1 on failure
 * returns 0 on success
 *
 * Method taken from 1060-lecture 30.10.2017
 */
int stringToInt(char* string, unsigned short* port) {
  char* ptr;
  int ret = strtol(string,&ptr, 10);
  if (ret == 0 || string == ptr) {
    printf("[port] arguement has to be an integer\n");
    return 1;
  }
  *port = (unsigned short) ret;
  return 0;
}

/**
 * checks if the debug flag is turned on or off
 * @return:
 * returns 1 if the flag is on
 * returns 0 if the flag is off
 */
int checkDebug() {
  #ifdef DEBUG
    return 1;
  #endif
  return 0;
}

/**
 * Calculates the value of the sent in string, then uses the modula
 * operator on the accumalated value. And then returns the checksum
 * @param
 * text: the string to be counted. NB: has to be '\0' terminated
 * @return
 * returns the value after count and %32
 **/
int calcCheckSum(char* text) {
  int count = 0;
  for (unsigned int i = 0; i < strlen(text); i++) {
    count += text[i];
  }
  count = count % 32;
  return count;
}

/**
 * Catches the Ctrl-C input from user
 * @param sig [description]
 **/
 void intHandler(int sig) {
   fclose(jobFile);
   if (sd == 0) {
     printf(CYN "Exiting program due to INTERRUPT signal caught\n" RESET);
     exit(sig);
   }
   //signal(sig, SIG_IGN);
   int clientChoice = 0;
   char info = 64;
   send(sd, &info, sizeof(char),0);
   while(1) {
     printf(GRN "Waiting For Clients Exit signal\n" RESET);
     recv(sd, &clientChoice, sizeof(int),0);
     if (clientChoice == -1 || clientChoice == -2) {
       printf(RED "%d\n",clientChoice );
       printf(GRN "Client Exit Signal Recieved\n" RESET);
       break;
     }
     else {
       send(sd, &info, sizeof(char),0);
     }
   }
   printf("exiting program due to INTERRUPT signal caught\n" );
   close(sd);
   exit(sig);
 }

/**
 * Checks if client has sent a terminating signal while we were doing something else
 * @return
 * returns 1 if it found a terminating Signal
 * returns 0 if it did not find a signal
 */
int pingClient() {
  int failCheck = -3;
  recv(sd,&failCheck, sizeof(int), MSG_PEEK | MSG_DONTWAIT );
  if (failCheck == -1 || failCheck == -2) {
    return 1;
  }
  else return 0;
}

/**
 * Attempts to read the data from the jobfile and send it to the client
 * @return
 * returns 1 if it read correctly and sent correctly
 * returns 0 if it reaches EOF
 * returns -1 if something went wrong
 * returns -2 if it recived a terminating signal from client during the method
 */
int getJob() {
  char jobType;
  unsigned char info;
  unsigned int jobLength;
  char* jobText;
  jobType =  fgetc(jobFile);
  if (!(jobType == 'O' || jobType == 'E') || feof(jobFile)) {
    info = 224;
    if (pingClient()) {
      printf(RED "ERROR: client sent a termintating message at an unexpected time\n" RESET);
      return - 2;
    }
    jobLength = 0;
    char buf[5];
    buf[0] = info;
    memcpy(buf + 1, &jobLength,sizeof(int));
    send(sd,buf,sizeof(char) * 5,0);
    return 0;
  }

  if(checkDebug()) printf(CYN "==%d== read jobType %c from jobfile\n" RESET,getpid(), jobType);
  ssize_t read_ret;
  fread(&jobLength, sizeof(int), 1, jobFile);
  if(checkDebug()) printf(CYN "==%d== read joblength %d from the jobfile\n",getpid(), jobLength);
  int msgLength = jobLength + 6;
  char msg [msgLength];
  jobText = malloc(sizeof(char) * jobLength + 1);
  read_ret = fread(jobText,1,jobLength,jobFile);
  if(checkDebug()) printf(CYN "==%d== read %zd bytes from jobfile. Expected bytes read: %d\n" RESET,getpid(),read_ret, jobLength );
  if (read_ret < jobLength) {
    printf(RED "ERROR: Read less bytes than jobLength\n"
               "Terminating program due to corrputed jobfile" RESET);
    info = 64;
    if (pingClient()) {
      printf(RED "ERROR: client sent a termintating message at an unexpected time\n" RESET);
      free(jobText);
      return - 2;
    }
    send(sd,&info,sizeof(unsigned char), 0);
    return -1;
  }

  //
  jobText[jobLength] = '\0';
  info = calcCheckSum(jobText);
  //Since Job types with O, has has bit pattern "000", we only need to edit
  //the jobinfo if jobtype is E
  if (jobType == 'E') {
    info |= 1 << 5;
  }
  msg[msgLength] = '\0';
  msg[0] = info;
  memcpy(msg + 1, &jobLength,sizeof(int));
  memcpy(msg + 5, jobText, sizeof(char) * jobLength + 1);
  //printf("%zd jobtext length\n", strlen(jobText));
  //printf("%d after bitwise\n",(info));
  //we have to check if the client have sent an termintaing message while we werent looking!
  if (pingClient()) {
    printf(RED "ERROR: client sent a termintating message at an unexpected time\n" RESET);
    free(jobText);
    return - 2;
  }
  ssize_t write_ret = send(sd, msg, sizeof(char) * (jobLength + 6), 0);
  if(checkDebug()) printf(CYN "==%d== sent %zd bytes to client. Expected bytes sent: %d\n" RESET, getpid(), write_ret, msgLength);
  free(jobText);
  return 1;
}


/**
 * the main loop in the server program. It attempts to get a request from client, and then executes
 * what the client sent over. the server reads 4 bytes from the client and stores them into an int.
 * for 0+ it will attempt to read a job from the jobfile. and if it reads -1 or -2 it terminates.
 * if it fails to read a job from the jobfile, it will signal to the client that the server wishes to
 * terminate and will wait until it recieves the terminating signal from user.
 * see PROTOKOLL.txt for a more detailed breakdown of my protocoll
*/
void clientInteraction() {
  int clientChoice = -1;
  unsigned char noJobsChar = 224;
  int status = 99;

  while (1) {
    printf("waiting for user input\n" );
    read(sd,&clientChoice, sizeof(int));
    printf("user input %d\n",clientChoice);

    if (clientChoice == -1 || clientChoice == -2) {
      printf("Exiting due to client input of %d\n",clientChoice);
      return;
    }

    if (clientChoice > 1) {
      for (int i = 0; i < clientChoice; i++) {
        if (!status) {
          break;
        }
        else {
          status = getJob();
        }
      }
    }

    else if (clientChoice == 0) {
      while (status >= 1) {
        status = getJob();
      }
    }

    else if (clientChoice == 1){
      status = getJob();
    }

    if (status == 0 || status == -1) {
      while(1) {
        printf(GRN "Waiting For Clients Exit signal\n" RESET);
        recv(sd, &clientChoice, sizeof(int),0);
        if (clientChoice == -1 || clientChoice == -2) {
          printf(RED "%d\n",clientChoice );
          printf(GRN "Client Exit Signal Recieved\n" RESET);
          return;
        }
        else {
          send(sd, &noJobsChar, sizeof(char),0);
        }
      }
    }
    else if(status == -3) {
      printf("nu var vi her ja\n");
      return;
    }
    clientChoice = -1;
    }
  }

  /**
   * Checks if the user provided the correct amount of parameters expected.
   * prints out correct usage of program to user.
   * NB: does not assure that the parameters are valid
   * @param
   * argc: the number of parameters provider
   * argv: the parameters themself
   * @return
   * returns -1 on failure
   * returns 0 on success
   */
  int usage(int argc, char* argv[]) {
    if (argc != 3) {
      printf(RED "Usage: %s <Server Adress> <Port>\n" RESET,argv[0] );
      return -1;
    }
    return 0;
  }

  int main(int argc, char* argv[] ){
    signal(SIGINT, intHandler);
    unsigned short port;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    unsigned int clientaddrlen;
    int request_sd;
    if (usage(argc,argv) == -1) {
      exit(1);
    }
    jobFile = fopen(argv[1], "r");
    if (jobFile == NULL) {
      printf(RED "<%s> Could Not Be Opened\n" RESET, argv[1]);
      exit(0);
    }
    int ret = stringToInt(argv[2],&port);
    if (ret == 1) {
      fclose(jobFile);
      exit(0);
    }
    request_sd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (request_sd == -1) {
      perror(RED "SOCKET ERROR");
      printf("RESET");
      fclose(jobFile);
      exit(1);
    }
    memset(&serveraddr, 0,sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);

    if (bind(request_sd, (struct  sockaddr*) &serveraddr, sizeof(struct sockaddr_in)) == -1) {
      perror(RED "BIND ERROR" );
      printf(RESET);
      fclose(jobFile);
      exit(1);
    }

    printf(CYN "Waiting for a client to connect\n" RESET);
    if (listen(request_sd, SOMAXCONN) == -1) {
      perror(RED "LISTEN ERROR");
      printf(RESET);
      fclose(jobFile);
      exit(1);
    }

    clientaddrlen = sizeof(struct sockaddr_in);
    sd = accept(request_sd,
        (struct sockaddr*)&clientaddr,
        &clientaddrlen);
    if (sd == -1) {
      perror(RED "ACCEPTATION ERROR");
      printf(RESET);
      close(request_sd);
      fclose(jobFile);
      exit(1);
    }
    char ipadress[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, ipadress,sizeof(ipadress));
      printf(CYN "==%d== accepted client on adress: %s with port: %s\n" RESET, getpid(), ipadress, argv[2]);

    close(request_sd);
    clientInteraction();
    fclose(jobFile);
    close(sd);
  }
