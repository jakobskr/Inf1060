/** Client Program which requests jobs from a server
 *  then executes the recieved job
 *  Author: 15144
 */

#include "common.h"
#define BUF_SIZE 253

int pipefd1[2];
int pipefd2[2];
int sd;
pid_t retv2 = -1, retv = -1;
int status = -2;
int exit_command = -1;
int kill_command = 0;
//had to set this as an global variable due to some mem leaks because of int handler
char* msg;

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
 * attempts to convert an string to an integer
 * @param  string the string we want to convert
 * @param  port   the pointer to where the converted int will be stored
 * @return        returns 1 on failure and 0 on success
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
 * Calculates the value of the sent in string by summarizing each char in the string, then uses the modula
 * operator on the accumalated value. And then returns the count
 * @param
 * text: the string to be counted
 * @return:
 * returns the count %32
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
 * Checks if the user provided the correct amount of arguments expected.
 * prints out correct usage of program to user.
 * NB: does not assure that the arguments are valid
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

/**
 * Starts the terminating process, during the processes the parent process
 * waits for the children to die before terminating itself after cleaning up
 * @param
 * sig: the signal that was caught
 * sig = 1 is SIGNAL HANGUP
 * sig = 2 is SIGNAL INTERRUPT
 */
void terminate(int sig) {
  if (retv == 0 || retv2 == 0) {
    return;
  }
  write(pipefd1[1], &kill_command, sizeof(int));
  write(pipefd2[1], &kill_command, sizeof(int));
  wait(NULL);
  send(sd, &exit_command, sizeof(int),0);
  close(pipefd1[0]);
  close(pipefd1[1]);
  close(pipefd2[0]);
  close(pipefd2[1]);
  close(sd);
  if (msg != NULL) {
    free(msg);
  }
  fflush(stderr);
  fflush(stdout);
  printf("exiting program due to caught SIGNAL %d from user\n",sig );
  printf(CYN "HAVE A NICE DAY\n" RESET );
  printf(RESET);
  exit(0);
}

/**
 * Catches the HANGUP signal and calls on terminate()
 */
void hang_up_handler(int sig) {
  terminate(sig);
}

/**
 * Catches the INTERRUPT signal and calls on terminate()
 */
void intHandler(int sig) {
  terminate(sig);
}

/**
 * Attempts to connect to a server on the given adress and port.
 * @param
 * argv: the array containing the i.p adress and port
 * @return
 * returns 1 on failure to connect after printing the error
 * returns 0 on success
 */
int connectToServer( char *argv[]) {
  int ret;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints,0,sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = IPPROTO_TCP;

  ret = getaddrinfo(argv[1], argv[2], &hints, &result);
  if (ret) {
    const char* err = gai_strerror(ret);
    perror(RED "GETADDRINFO ERROR");
    fprintf(stderr, RED "getaddrinfo: %s\n" RESET, err);
    freeaddrinfo(result);
    return 1;
  }

  sd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
  if (sd == -1) {
    perror(RED "SOCKET ERROR");
    printf(RESET);
  }
  char ipadress[INET_ADDRSTRLEN];
  void *addr;
  for(rp = result; rp != NULL; rp = rp->ai_next) {
    ret = connect(sd, rp->ai_addr, rp->ai_addrlen);
    if(!ret) {
      if (checkDebug()) {
        if (rp-> ai_family == AF_INET) {
          struct sockaddr_in *holder = (struct sockaddr_in *) rp->ai_addr;
          addr = &(holder->sin_addr);
        }
        inet_ntop(AF_INET, addr, ipadress,sizeof(ipadress));
        printf(CYN "==%d== connected to server on adress: %s with port: %s\n" RESET,getpid(), ipadress, argv[2] );
      }
      break;
    }
  }

  if (rp == NULL && ret == -1) {
    fprintf(stderr, RED "CONNECTION ERROR: NO ADRESS SUCCEDED\n" RESET);
    freeaddrinfo(result);
    return 1;
  }
  freeaddrinfo(result);
  return 0;
}

/**
 * Attempts to read jobs sent from the parent process, and will either print the recieved job to either stdout or stderr
 * depending on which child process we are in. if recieves a joblength of 0 it will then close the pipes and then terminate itself
 * @param
 * pipe: the pipe that the child uses to read from
 */
void waitForParent(int pipe[2]) {
  int read_end = pipe[0];
  unsigned int jobLength = 0;
  char* jobText = NULL;
  ssize_t read_ret = 0;
  while (1) {
    read_ret = 0;
    if(retv == 0) {
      if(checkDebug()) printf(YEL "==%d== is waiting for %d\n" RESET, getpid(), getppid());
    }
    else if (retv2 == 0){
      if(checkDebug()) fprintf(stderr,RED "==%d== is waiting for %d\n" RESET, getpid(), getppid());
    }
    read_ret += read(read_end,&jobLength, sizeof(int));

    if (checkDebug()) {
      if (retv == 0) {
        printf(YEL "==%d== read jobLength %d from %d\n" RESET,getpid(), jobLength, getppid());
      }
      else if (retv2 == 0) {
        fprintf(stderr, RED "==%d== read jobLength %d from %d\n" RESET, getpid(), jobLength, getppid());
      }
    }

    if (jobLength) {
      jobText = malloc(sizeof(char) * jobLength + 1);
      read_ret += read(read_end, jobText, sizeof(char) * jobLength + 1);
      if (checkDebug()) {
        if (retv == 0) {
          printf(YEL "==%d== read %zd bytes from %d. Expected bytes read: %d\n",getpid(),read_ret, getppid(),jobLength + 5);
        }
        else if (retv2 == 0) {
          fprintf(stderr, RED "==%d== read %zd bytes from %d. Expected bytes read: %d\n",getpid(),read_ret, getppid(),jobLength + 5);
        }
      }

      if (retv == 0) {
        if(checkDebug()) printf(YEL "==%d== printing job:\n" RESET, getpid());
        printf(YEL "%s\n" RESET, jobText);
      }

      else if (retv2 == 0){
        if(checkDebug()) fprintf(stderr ,RED "==%d== printing job:\n" RESET, getpid());
        fprintf(stderr, RED "%s\n" RESET,jobText);
        }
      free(jobText);
      jobText = NULL;
    }

    else if(jobLength == 0) {
      if (retv == 0) {
        if (checkDebug()) printf(YEL "==%d== Terminating\n" RESET,getpid());
      }
      else if (retv2 == 0) {
        if (checkDebug())  fprintf(stderr, RED "==%d== Terminating\n" RESET,getpid());
      }
      if (jobText) {
        free(jobText);
        jobText = NULL;
      }

      if (checkDebug()) {
        if (retv == 0) {
          printf(YEL "==%d== closing pipes and exiting\n" RESET, getpid());
        }
        else if (retv2 == 0) {
          fprintf(stderr, RED "==%d== closing pipes and exiting\n" RESET, getpid());
        }
      }
      close(pipefd1[0]);
      close(pipefd1[1]);
      close(pipefd2[0]);
      close(pipefd2[1]);
      exit(0);
    }
  }
}

/**
 * Prints the commmand menu
 */
void printMenu()  {
  printf(CYN
  "Valid Commands:\n"
  "1: Get one job\n"
  "2: Get n number of jobs\n"
  "3: Get all of the jobs\n"
  "4: Print this menu again\n"
  "-1: Exit Program\n" RESET);
}

/**
 * Will try to recieve jobs from the server until it either gets an error message
 * or a NO_MORE_JOBS message from the server. The recived jobs are processed by the one of the children depending
 * on the jobType. if it recieves is 224 or 64 then the parent tells its child process to terminate.
 * @return:
 * returns 0 when there are no more jobs left.
 * returns -1 if it recieves an error message.
 */
int get_all_jobs() {
  int msgLength;
  unsigned char info;
  int command = 0;
  send(sd, &command, sizeof(int), 0);
  ssize_t write_ret;
  while (1) {
    write_ret = 0;
    recv(sd,&info,sizeof(unsigned char),0);
    if (info == 224) {
      write(pipefd1[1],&kill_command, sizeof(int));
      write(pipefd2[1],&kill_command, sizeof(int));
      printf(CYN "No more jobs to recieve\n" "starting termination process\n" RESET);
      send(sd,&exit_command,sizeof(int),0);
      return 0;
    }
    else if (info == 64) {
      write(pipefd1[1],&kill_command, sizeof(int));
      write(pipefd2[1],&kill_command, sizeof(int));
      printf(YEL "Server Terminating Due to Server Side ERROR\n" RESET);
      send(sd,&exit_command,sizeof(int),0);
      return -1;
    }

    if (checkDebug()) {
      if (info >> 5 == 0) {
        printf(CYN "==%d== recived INFO %d from server [jobType: E checkSum: %d]\n" RESET,getpid(), info, (info & 31));
      }
      else {
        printf(CYN "==%d== recived INFO %d from server [jobType: O checkSum: %d]\n" RESET,getpid(), info, (info & 31));
      }
    }

    recv(sd,&msgLength,sizeof(int),0);
    if(checkDebug()) printf(CYN "==%d== recieved msgLength %d from server\n" RESET,getpid(), msgLength );
    msg = (char*) malloc((sizeof(char)) * (msgLength + 1));
    ssize_t readret = recv(sd,msg,sizeof(char) * msgLength + 1, 0);

    if (checkDebug()) {
      printf(CYN "==%d== recived %zd bytes from server. Bytes Expected %d\n" RESET, getpid(),readret, msgLength + 1);
    }

    if (readret != msgLength + 1) {
      printf("%zd less than %d :thinking: \n", readret, msgLength + 1);
      free(msg);
      continue;
    }

    int checkSum = calcCheckSum(msg);
    if(checkDebug()) printf(CYN "==%d== checksum from server: %d checksum from client %d\n" RESET,getpid(), info & 31, checkSum );

    if ((info & 31) != checkSum) {
      printf("info checksum differs from calcCheckSum()\n");
      free(msg);
      continue;
    }

    if (info >> 5 == 0) {
      write_ret += write(pipefd1[1],&msgLength, sizeof(int));
      write_ret += write(pipefd1[1],msg, sizeof(char) * msgLength + 1);
      if (checkDebug()) {
        printf(CYN "==%d== wrote %zd bytes too %d. Excpected bytes written: %d \n" RESET,getpid(), write_ret, retv, (msgLength +5));
      }
    }

    else if (info >> 5 == 1) {
      write_ret += write(pipefd2[1],&msgLength, sizeof(int));
      write_ret += write(pipefd2[1],msg, sizeof(char) * msgLength + 1);
      if (checkDebug()) {
        printf(CYN "==%d== wrote %zd bytes too %d. Excpected bytes written: %d \n" RESET,getpid(), write_ret, retv2, (msgLength +5));
      }
    }
    free(msg);
    msg = NULL;
  }
}

/**
 * Attemps to read @jobsRequested number of jobs from the server,
 * The recived jobs are processed by the one of the children depending
 * on the jobType. if it recieves 224 or 64 then the parent tells its child process to terminate.
 * @param
 * jobsRequested: number of jobs requested
 * @return:
 * returns 0 if it did not succeed on recieving all of the jobs
 * returns 1 if it succeeded on recieving all of the requested jobsRequested
 * returns -1 if it recieved an error message
 */
int get_job_from_server(int jobsRequested) {
  unsigned int msgLength;
  unsigned char info;
  ssize_t write_ret;
  printf(CYN "Getting %d jobs from server\n" RESET, jobsRequested);
  send(sd, &jobsRequested, sizeof(unsigned int),0);
  for (int i = 0; i < jobsRequested; i++) {
    write_ret = 0;
    recv(sd,&info,sizeof(unsigned char),0);
    if (info == 224) {
      write(pipefd1[1],&kill_command, sizeof(int));
      write(pipefd2[1],&kill_command, sizeof(int));
      printf(CYN "No more jobs to recieve\n" "starting termination process\n" RESET);
      send(sd,&exit_command,sizeof(int),0);
      return 0;
    }
    else if (info == 64) {
      write(pipefd1[1],&kill_command, sizeof(int));
      write(pipefd2[1],&kill_command, sizeof(int));
      printf(YEL "Server Terminating Due to Server Side ERROR\n" RESET);
      send(sd,&exit_command,sizeof(int),0);
      return -1;
    }

    if (checkDebug()) {
      if (info >> 5 == 0) {
        printf(CYN "==%d== recived INFO %d from server [jobType: E checkSum: %d]\n" RESET,getpid(), info, (info & 31));
      }
      else {
        printf(CYN "==%d== recived INFO %d from server [jobType: O checkSum: %d]\n" RESET,getpid(), info, (info & 31));
      }
    }

    recv(sd,&msgLength,sizeof(int),0);
    if(checkDebug()) printf(CYN "==%d== recieved msgLength %d from server\n" RESET,getpid(), msgLength );
    msg = (char*) malloc((sizeof(char)) * (msgLength + 1));
    ssize_t readret = recv(sd,msg,sizeof(char) * msgLength + 1, 0);

    if (checkDebug()) {
      printf(CYN "==%d== recived %zd bytes from server. Bytes Expected %d\n" RESET, getpid(),readret, msgLength + 1);
    }

    if (readret != msgLength + 1) {
      printf("%zd less than %d :thinking: \n", readret, msgLength + 1);
      free(msg);
      continue;
    }

    int checkSum = calcCheckSum(msg);
    if(checkDebug()) printf(CYN "==%d== checksum from server: %d checksum from client %d\n" RESET,getpid(), info & 31, checkSum );

    if ((info & 31) != checkSum) {
      printf("info checksum differs from calcCheckSum()\n");
      free(msg);
      continue;
    }

    if (info >> 5 == 0) {
      write_ret += write(pipefd1[1],&msgLength, sizeof(int));
      write_ret += write(pipefd1[1],msg, sizeof(char) * msgLength + 1);
      if (checkDebug()) {
        printf(CYN "==%d== wrote %zd bytes too %d. Excpected bytes written: %d \n" RESET,getpid(), write_ret, retv, (msgLength +5));
      }
    }

    else if (info >> 5 == 1) {
      write_ret += write(pipefd2[1],&msgLength, sizeof(int));
      write_ret += write(pipefd2[1],msg, sizeof(char) * msgLength + 1);
      if (checkDebug()) {
        printf(CYN "==%d== wrote %zd bytes too %d. Excpected bytes written: %d \n" RESET,getpid(), write_ret, retv2, (msgLength +5));
      }
    }
    free(msg);
    msg = NULL;
  }
  return 1;
}

/**
 * The main command loop in the program, it prints the menu, then ask the user for a input
 * then executes the representing command. the loop will terminate when
 * either of the getJobs functions returns 0 or -1 since that means that either the server terminated
 * or there are no more jobs left
 */
void shell() {
  int choice = -2;
  int result = 0;
  printMenu();
  while (1) {
    if (!status || status == -1) {

      return;
    }
    printf(CYN "input command\n" RESET);
    result = scanf("%d", &choice);
    if (result == EOF) {

    }
    else if(choice == 1){
      status = get_job_from_server(choice);
    }

    else if (choice == 2) {
      while ( getchar() != '\n' ){}
      choice = 0;
      printf("How many jobs do you want?\n");
      while (1) {
        scanf("%d", &choice);
        if (choice <= 0 || choice > INT_MAX) {
          printf(CYN "not a valid number\n" );
          printf("input a number between 0 and %d \n" RESET, INT_MAX);
          while ( getchar() != '\n' ){}
        }
        else break;
      }

      status = get_job_from_server(choice);
    }

    else if (choice == 3) {
      printf("Getting all the jobs from the server\n");
      status = get_all_jobs();
    }

    else if (choice == 4) {
      printMenu();
    }

    else if (choice == -1) {
      send(sd,&exit_command,sizeof(int),0);
      write(pipefd1[1],&kill_command,sizeof(int));
      write(pipefd2[1],&kill_command,sizeof(int));
      break;
    }
    else  {
      printf(CYN "Not a valid input\n" RESET);
      printf(CYN "input 4 to view menu again\n" RESET);
    }
    choice = -2;
    while ( getchar() != '\n' ){}
  }
}

/**
 * sets up the pipes and child proccesses, then sends the child processes to the waitForParent(),
 * while the first calls on connectToServer() and then shell().
 * when the parent returns from shell() it cleans up the parent process.
 */
int main(int argc, char* argv[]) {
  signal(SIGINT, intHandler);
  signal(SIGHUP,hang_up_handler);
  if (usage(argc,argv) == -1) {
    exit(1);
  }
  unsigned short port;
  int ret = stringToInt(argv[2],&port);
  if (ret == 1) {
    exit(0);
  }
  if (checkDebug()) printf(CYN "==%d== started the client program\n",getpid() );

  if (pipe(pipefd1) == -1) {
    perror(RED "Pipe Error" RESET);
    exit(1);
  }

  if (pipe(pipefd2) == -1) {
    perror(RED "Pipe Error" RESET);
    close(pipefd1[0]);
    close(pipefd1[1]);
    exit(1);
  }
  if (checkDebug()) {
    printf(CYN "==%d== Successfully opened pipes: pipe 1: [%d %d], pipe 2: [%d %d]\n" RESET,getpid(), pipefd1[0], pipefd1[1], pipefd2[0], pipefd2[1] );
  }
  retv = fork();
  if (retv == 0) { //First Child
    if (checkDebug()) printf(YEL "==%d== created by %d\n" RESET,getpid(),getppid());
    waitForParent(pipefd1);
  }
  //
  else if (retv > 0)  { //Parent
    retv2 = fork();
    if (retv2 == 0) {//Second Child
      if (checkDebug()) printf(RED "==%d== created by %d\n" RESET,getpid(),getppid());
      waitForParent(pipefd2);
    }
    else if (retv2 > 0) {
      if(connectToServer(argv) == 1) {
        //client failed to connect to server
        write(pipefd1[1],&kill_command,sizeof(int));
        write(pipefd2[1],&kill_command,sizeof(int));
        close(pipefd1[1]);
        close(pipefd2[1]);
        close(sd);
        exit(1);
      }
      shell();
      if(checkDebug()) {
        printf(CYN "==%d== cleaning up before terminating\n" RESET, getpid());
      }
      wait(NULL);
      //parent sleep for one second to asure that the exit messages are the last thing printed
      sleep(1);
      close(pipefd1[1]);
      close(pipefd2[1]);
      close(sd);
      fflush(stderr);
      fflush(stdout);
      printf(CYN "TERMINATING PROCESS FINISHED\n" "HAVE A NICE DAY\n" RESET );
    }
  }
  return 0;
}
