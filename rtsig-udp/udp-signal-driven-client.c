#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define MYPORT 3490
#define MAXBUFLEN 100

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr,"usage: ./client hostname message\n");
        exit(1);
    }

    // dns resolve
    struct hostent *he;
    if ((he=gethostbyname(argv[1])) == NULL) {
        herror("gethostbyname");
        exit(1);
    }

    // udp socket config
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr_svr;
    addr_svr.sin_family = AF_INET;      /* host byte order */
    addr_svr.sin_port = htons(MYPORT);  /* short, network byte order */
    addr_svr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(addr_svr.sin_zero), 8);     /* zero the rest of the struct */
    while(1) {
        int numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0, (struct sockaddr *)&addr_svr, sizeof(struct sockaddr));
        if (numbytes < 0) {
            perror("sendto");
            exit(1);
        }
        printf("sent %d bytes to %s\n", numbytes, inet_ntoa(addr_svr.sin_addr));
    }
    close(sockfd);

    return 0;
}
