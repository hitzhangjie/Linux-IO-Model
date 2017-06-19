#ifndef _LINUX_H
#define _LINUX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/epoll.h>

#define MAX(a,b) (a>b? a:b)

#define BUF_SIZE 16
char buf[BUF_SIZE];

ssize_t doReadAction(int fd)
{
    ssize_t cnt = read(fd, buf, BUF_SIZE);
    if(cnt == 0) {
        printf("read eof from sock_conn %d, peer socket is closed\n", fd);
    }
    else if(cnt > 0) {
        buf[cnt] = '\0';
        printf("read %d characters from fd %d: ", (int)cnt, fd);
    
        int i = 0;
        for(; i<cnt; i++) {
            printf("%c", buf[i]);
        }
        printf("\n");
    }
    else {
        fprintf(stderr, "error occurred when reading from sock_conn %d\n", fd);
        fprintf(stderr, strerror(errno));
    }
    return cnt;
}

ssize_t doWriteAction(int fd)
{
    ssize_t cnt = write(fd, buf, strlen(buf));
    if(cnt < 0) {
        if(errno == EPIPE) {
            perror("peer socket is closed\n");
            return -1;
        } 
        else {
            perror("other error\n");
            fprintf(stderr, strerror(errno));
            return -1;
        }
    }
    printf("write %d characters to fd %d: %s\n", (int)cnt, fd, buf);

    return cnt;
}

void doEchoAction(int fd, fd_set *readfds, fd_set *writefds)
{
    ssize_t rd_cnt = doReadAction(fd);
    // peer socket closed, including read/write stream
    if(rd_cnt <= 0) {
        FD_CLR(fd, readfds);
        FD_CLR(fd, writefds);
        close(fd);
        return;
    }

    // echo
    ssize_t wr_cnt = doWriteAction(fd);    
    if(wr_cnt < 0) {
        fprintf(stderr, "echo to client sock_conn %d failed\n", fd);
        if(errno == EPIPE) {
            fprintf(stderr, "peer socket sock_conn %d closed\n", fd);
            FD_CLR(fd, readfds);
            FD_CLR(fd, writefds);
            close(fd);
        }
    }
}

void doEchoActionInPoll(struct pollfd fds[], int max_num_fds, int i)
{
    if(i<=0 || i>=max_num_fds) {
        fprintf(stderr, "invalid param i, i = %d\n", i);
        return;
    }

    int fd = fds[i].fd;
    ssize_t rd_cnt = doReadAction(fd);

    // peer socket closed, including read/write stream
    if(rd_cnt <= 0) {
        fds[i].events &= ~POLLIN;
        fds[i].events &= ~POLLOUT;
        close(fd);
        return;
    }

    // echo
    ssize_t wr_cnt = doWriteAction(fd);    
    if(wr_cnt < 0) {
        fprintf(stderr, "echo to client sock_conn %d failed\n", fd);
        if(errno == EPIPE) {
            fprintf(stderr, "peer socket sock_conn %d closed\n", fd);
            fds[i].events &= ~POLLIN;
            fds[i].events &= ~POLLOUT;
            close(fd);
        }
    }
}

void doEchoActionInEpoll(int epfd, int fd)
{
    ssize_t rd_cnt = doReadAction(fd);

    // peer socket closed, including r/w stream
    if(rd_cnt <= 0) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        return;
    }

    // echo
    ssize_t wr_cnt = doWriteAction(fd);
    if(wr_cnt < 0) {
        fprintf(stderr, "echo to client sock_conn %d failed\n", fd);
        if(errno == EPIPE) {
            fprintf(stderr, "peer socket sock_conn %d closed\n", fd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
        }
    }
}

void doEchoActionInRtsig(siginfo_t *siginfo)
{
    if(!siginfo) return;

    int fd = siginfo->si_fd;
    int signo = siginfo->si_signo;

    while(1) {
        ssize_t rd_cnt = read(fd, buf, BUF_SIZE);
        if(rd_cnt > 0) {
            buf[rd_cnt] = '\0';
            
            int offset = 0;
            while((offset += write(fd, buf+offset, strlen(buf+offset))) > 0) {
                ;
            }
            offset = 0;
            while((offset += write(0, buf+offset, strlen(buf+offset))) > 0) {
                ;
            }
        }
        else {
            close(fd);
            break;
        }
    }
}



#endif
