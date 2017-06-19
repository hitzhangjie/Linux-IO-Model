#include "../common/linux.h"

#define MAX_EVENTS 12
#define IPADDR "127.0.0.1"
#define PORT 22000
#define BACKLOG 10

int main()
{
    // 向epfd注册fd及该fd上关心的事件，需通过epoll_data来指明
    struct epoll_event ev;
    // epoll_wait返回的就绪事件
    struct epoll_event events[MAX_EVENTS];
    
    // 创建epoll句柄epfd
    int epfd = epoll_create(MAX_EVENTS);
    
    // 监听socket配置
    int sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_listen < 0) {
        perror("failed to create sock_listen");
        return 1;
    }
    printf("create socket_listen success\n");

    struct sockaddr_in addr_listen;
    addr_listen.sin_family = AF_INET;
    addr_listen.sin_port = htons(PORT);
    if(!inet_aton(IPADDR, &addr_listen.sin_addr)) {
        fprintf(stderr, "invalid ipv4 address, ip = %s\n", IPADDR);
        return 1;
    }
    printf("initialize addr_listen success\n");

    if(bind(sock_listen, (struct sockaddr *)&addr_listen, sizeof(addr_listen)) < 0) {
        perror("failed to bind sock_listen & addr_listen");
        return 1;
    }
    printf("bind sock_listen & addr_listen success\n");

    if(listen(sock_listen, BACKLOG) < 0) {
        fprintf(stderr, "failed to listen on ip:port = %s:%d", IPADDR, PORT);
        return 1;
    }
    printf("listen on ip:port success, ip:port = %s:%d\n", IPADDR, PORT);
    
    // 对socket描述符上关心的事件进行注册
    ev.events = EPOLLIN;
    ev.data.fd = sock_listen;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock_listen, &ev);
    
    while(1) {
        // 调用epoll_wait阻塞等待，等待事件未永远等待直到发生事件
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

        int i;
        for(i=0; i<n; ++i) {
            // 测试是否有客户端发起连接请求，如果有则接受并把新建的描述符加入监控
            if(events[i].data.fd == sock_listen) {
                if(events[i].events & EPOLLIN) {
                    int sock_conn = accept(sock_listen, NULL, NULL); 
    
                    ev.events = EPOLLIN | EPOLLOUT;
                    ev.data.fd = sock_conn;
    
                    epoll_ctl(epfd, EPOLL_CTL_ADD, sock_conn, &ev);
                }
            }
            // 对于其他描述符上发生的事件进行适当处理
            else {
                if( (events[i].events & EPOLLIN) && (events[i].events & EPOLLOUT) ) {
                    doEchoActionInEpoll(epfd, events[i].data.fd);
                }
                /*
                if(events[i].events & EPOLLIN) {
                    doReadAction(i);
                }
                if(events[i].events & EPOLLOUT) {
                    doWriteAction(i);
                }
                */
            }
        }
    }
}
