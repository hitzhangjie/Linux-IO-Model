#define _GNU_SOURCE

#include "../common/linux.h"

#define IPADDR "127.0.0.1"
#define PORT 24000
#define BACKLOG 10

void sighandler(int signo)
{
    if(signo == SIGRTMIN+1) {
        printf("recv realtime signal SIGRTMIN+1, signal = %d\n", signo);
    }
    else if(signo == SIGIO) {
        printf("recv realtime signal SIGIO, signal = %d\n", signo);
    }
    else {
        printf("recv unknown signal, signal = %d\n", signo);
    }
}

void verbose(siginfo_t *siginfo)
{
    printf("siginfo: fd = %d, signo = %d\n", siginfo->si_fd, siginfo->si_signo);
}

int main()
{
    signal(SIGRTMIN+1, sighandler);
    signal(SIGIO, sighandler);

    // block all signals
    sigset_t all;
    sigfillset(&all);
    sigdelset(&all, SIGINT);
    sigprocmask(SIG_SETMASK, &all, NULL);

    // 新建并初始化关心的信号
    sigset_t sigset;
    siginfo_t siginfo;
    
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGRTMIN + 1);
    //sigaddset(&sigset, SIGIO);
    
    // socket配置和监听
    int sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_listen < 0) {
        perror("failed to create sock_listen");
        return 1;
    }
    printf("create sock_listen success\n");

    struct sockaddr_in addr_listen;
    addr_listen.sin_family = AF_INET;
    addr_listen.sin_port = htons(PORT);
    if(!inet_aton(IPADDR, &addr_listen.sin_addr)) {
        fprintf(stderr, "invalid ipv4 addr, addr = %s\n", IPADDR);
        return 1;
    }
    printf("intialize addr_listen success, addr = %s\n", IPADDR);

    if(bind(sock_listen, (struct sockaddr *)&addr_listen, sizeof(addr_listen)) < 0) {
        fprintf(stderr, "failed to bind sock_listen & addr_listen, ip:port = %s:%d\n", IPADDR, PORT);
        return 1;
    }
    printf("bind sock_listen & addr_listen success, ip:port = %s:%d\n", IPADDR, PORT);

    if(listen(sock_listen, BACKLOG) < 0) {
        fprintf(stderr, "failed to listen on ip:port = %s:%d\n", IPADDR, PORT);
        return 1;
    }
    printf("listen on ip:port success, ip:port = %s:%d\n", IPADDR, PORT);
    
    // 重新设置描述符可读写时要发送的信号值
    fcntl(sock_listen, F_SETSIG, SIGRTMIN + 1);
    fcntl(sock_listen, F_SETOWN, getpid());
    int flags = fcntl(sock_listen, F_GETFL);
    fcntl(sock_listen, F_SETFL, flags|O_ASYNC|O_NONBLOCK);
    
    while(1) {
        struct timespec ts;
        ts.tv_sec = 1;
        ts.tv_nsec = 0;
    
        // 调用sigtimedwait阻塞等待，等待事件1s
        //sigtimedwait(&sigset, &siginfo, &ts);
        //printf("sigtimedwait wake up\n");
        sigwaitinfo(&sigset, &siginfo); 

        verbose(&siginfo);

        // 测试是否有客户端发起连接请求
        if(siginfo.si_fd == sock_listen) {
            int sock_conn = accept(sock_listen, NULL, NULL); 
            fcntl(sock_conn, F_SETSIG, SIGRTMIN + 1);
            fcntl(sock_conn, F_SETOWN, getpid());
            int flags = fcntl(sock_listen, F_GETFL);
            fcntl(sock_conn, F_SETFL, flags|O_ASYNC|O_NONBLOCK);

            printf("accept new connection & enable rtsig for sock_conn %d\n", sock_conn);
        }
        // 对其他描述符上发生的读写事件进行处理
        else {
            doEchoActionInRtsig(&siginfo);
            //printf("do echo action done\n");

            /*
            doReadAction(i);
            doWriteAction(i);
            */
        }
    }
}
