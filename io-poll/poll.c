#include "../common/linux.h"

#define MAX_NUM_FDS 10

#define IPADDR "127.0.0.1"
#define PORT 21000 
#define BACKLOG 10

int main()
{
    // 新建并初始化文件描述符集
    struct pollfd fds[MAX_NUM_FDS];
    int nfds = 0;

    // tcp socket配置和监听
    int sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_listen < 0) {
        perror("failed to create sock_listen");
        return 1;
    }
    printf("create sock_listen success, sock_listen = %d\n", sock_listen);

    struct sockaddr_in addr_listen;
    addr_listen.sin_family = AF_INET;
    addr_listen.sin_port = htons((short)PORT);
    if(!inet_aton(IPADDR, &addr_listen.sin_addr)) {
        fprintf(stderr, "invalid ipaddr, ipaddr = %s\n", IPADDR);
        return 1;
    }
    printf("initialize addr_listen success, ip:port=%s:%d\n", IPADDR, PORT);

    if(bind(sock_listen, (struct sockaddr *)&addr_listen, sizeof(addr_listen)) < 0) {
        perror("bind socket & addr failed");
        return 1;
    }
    printf("bind socket & addr success\n");

    if(listen(sock_listen, BACKLOG) < 0) {
        fprintf(stderr, "failed to listen on ip:port = %s:%d", IPADDR, PORT);
        return 1;
    }
    printf("listen on ip:port success, ip:port= %s:%d\n", IPADDR, PORT);
    
    // 对socket描述符上关心的事件进行注册
    fds[0].fd = sock_listen;
    fds[0].events = POLLIN;
    ++ nfds;
    
    while(1) {
    
        int i;
    
        // 调用poll阻塞等待，等待时间为永远等待直到事件发生
        poll(fds, nfds, -1);
    
        // 测试是否有客户端发起连接请求，如果有则接受并把新建的描述符加入监控
        if(fds[0].revents & POLLIN) {
            int sock_conn = accept(sock_listen, NULL, NULL); 
    
            fds[nfds].fd = sock_conn;
            fds[nfds].events = POLLIN | POLLOUT;

            ++ nfds;
        }
        
        // 对其他描述符发生的事件进行适当处理
        for(i=1; i<nfds+1; ++i) {
            if( (fds[i].revents & POLLIN) && (fds[i].revents & POLLOUT) ) {
                doEchoActionInPoll(fds, MAX_NUM_FDS, i);
            }
            /*
            if(fds[i].revents & POLLIN) {
                doReadAction(i);
            }
            if(fds[i].revents & POLLOUT) {
                doWriteAction(i);
            }
            */
        }
    }
}
