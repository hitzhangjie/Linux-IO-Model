void io_handler(int); /*This is the main I/O procedure */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/* #include <netdb.h> */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/time.h>

#define MYPORT 3490    /* the port users will be sending to */
#define MAXBUFLEN 100

int sock;
char buf[MAXBUFLEN];

int main()
{
    // udp socket config
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("opening datagram socket");
        exit(1);
    }

    struct sockaddr_in addr_svr;
    addr_svr.sin_family = AF_INET;
    addr_svr.sin_addr.s_addr = INADDR_ANY;
    addr_svr.sin_port = htons(MYPORT);

    if (bind(sock, (struct sockaddr *)&addr_svr, sizeof addr_svr) <0 ) {
        perror("binding datagram socket");
        exit(1);
    }

    /*
    int length = sizeof(addr_svr);
    if (getsockname(sock, (struct sockaddr *)&addr_svr, &length) < 0){
        perror("getting socket name");
        exit(1);
    }
    printf("Socket port #%d\n", ntohs(addr_svr.sin_port));
    */

    // first: set up a SIGIO signal handler by use of the signal call()
    signal(SIGIO, io_handler);

    // second: set the process id or process group id that is to receive
    // notification of pending input to its own process id or process 
    // group id
    if (fcntl(sock, F_SETOWN, getpid()) < 0){
        perror("fcntl F_SETOWN");
        exit(1);
    }

    // third: allow receipt of asynchronous I/O signals
    // - in glibc, O_ASYNC is synonyms of FASYNC.
    if (fcntl(sock, F_SETFL, FASYNC) < 0){
        perror("fcntl F_SETFL, FASYNC");
        exit(1);
    }


    for(;;)
        ;
    /* Actually here was suppose to come the rest of the code (not dealing with comm) */
}

void io_handler(int signal)
{
    struct sockaddr_in  addr_peer; /* connector's address information  */
    int addr_len;
    
    printf("The recvfrom proc. !\n");

    int numbytes = recvfrom(sock, buf, MAXBUFLEN, 0, (struct sockaddr *)&addr_peer, &addr_len);
    if(numbytes < 0) {
        perror("recvfrom");
        exit(1);
    }
    
    buf[numbytes]='\0';

    printf("got from %s --->%s \n", inet_ntoa(addr_peer.sin_addr), buf);
    return;
}
