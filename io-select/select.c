#include "../common/linux.h"

#define PORT 20000
#define IPADDR "127.0.0.1"
#define BACKLOG 10

int main(void) 
{
    // 可读、可写、异常3种文件描述符集的声明和初始化
    fd_set readfds, writefds, exceptfds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    int max_fd;

    // tcp socket配置和监听
    int sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_listen < 0) {
        perror("create listening socket failed\n");
        return 1;
    }
    printf("create listening socket success\n");

    struct sockaddr_in addr_listen;
    addr_listen.sin_family = AF_INET;
    addr_listen.sin_port = htons(PORT);
    if(!inet_aton(IPADDR, &addr_listen.sin_addr)) {
        fprintf(stderr, "initialize ipv4 address failed, ipaddr = %s", IPADDR);
        perror(strerror(errno));
        return 1;
    }
    printf("initialize ipv4 address success, ipaddr = %s\n", IPADDR);

    if(bind(sock_listen, (struct sockaddr *)&addr_listen, sizeof(addr_listen)) < 0) {
        perror("bind socket & addr failed\n");
        perror(strerror(errno));
        return 1;
    } 
    printf("bind socket & addr success\n");

    if(listen(sock_listen, BACKLOG) < 0) {
        fprintf(stderr, "listen failed, ip:port = %s:%d", IPADDR, PORT);
        perror(strerror(errno));
        return 1;
    }
    printf("listen success, ip:port = %s:%d\n", IPADDR, PORT);
     
    // 对socket描述符上关心的事件进行注册，select不要求fd非阻塞
    FD_SET(sock_listen, &readfds);
    max_fd = sock_listen;

    int loop = 0;
    while (1)
    {
        printf("loop times: %d\n", loop++);

        int i;
        fd_set r, w, e;
         
        // 为了重复使用readfds、writefds、exceptionfds，将他们复制到临时变量内
        memcpy(&r, &readfds, sizeof(fd_set));
        memcpy(&w, &writefds, sizeof(fd_set));
        memcpy(&e, &exceptfds, sizeof(fd_set));
         
        // 利用临时变量调用select阻塞等待，等待时间为永远等待直到事件发生
        select(max_fd + 1, &r, &w, &e, NULL);
         
        // 测试是否有客户端发起连接请求，如果有则接受并把新建的描述符加入监控
        if (FD_ISSET(sock_listen, &r))
        {
            int sock_conn = accept(sock_listen, NULL, NULL); 
            printf("create new sock_conn %d\n", sock_conn);

            FD_SET(sock_conn, &readfds);
            FD_SET(sock_conn, &writefds);
            max_fd = MAX(max_fd, sock_conn);
        }
         
        // 对其他描述符上发生的事件进行适当处理
        // 描述符依次递增，各系统的最大值可能有所不同，一般可以通过ulimit -n进行设置
        for (i = sock_listen + 1; i < max_fd + 1; ++i)
        {
            if(FD_ISSET(i, &r) && FD_ISSET(i, &w)) {
                doEchoAction(i, &r, &w);
            }

            /*
            if (FD_ISSET(i, &r)) {
                ssize_t cnt = doReadAction(i);
                // peer socket closed
                if(cnt == 0) FD_CLR(i, &r);
                //if(cnt < 0) FD_CLR(i, &r);
            }

            if (FD_ISSET(i, &w)) {
                ssize_t cnt = doWriteAction(i);
                // peer socket closed
                if(cnt < 0 && errno == EPIPE)
                    FD_CLR(i, &w);
            }
            */
        }
    }

    return 0;
}
