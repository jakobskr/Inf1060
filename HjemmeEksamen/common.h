#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>


// COLOURS :))))
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

// if you want to turn off the colours for some reason
// comment out the colour block over and uncomment the colour block below
/*
#define RED   "\x1B[0m"
#define GRN   "\x1B[0m"
#define YEL   "\x1B[0m"
#define BLU   "\x1B[0m"
#define MAG   "\x1B[0m"
#define CYN   "\x1B[0m"
#define WHT   "\x1B[0m"
#define RESET "\x1B[0m"
 */
