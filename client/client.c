#include "../common/linux.h"

int main(int argc, char *argv[])
{
    if(argc<3) {
        perror("usage: ./client <ipaddr> <port> <msg> [repeat=1]");        
        return 1;
    }

    // sever addr inited
    struct sockaddr_in addr_svr;
    addr_svr.sin_family = AF_INET;
    addr_svr.sin_port = htons((short)atoi(argv[2]));
    if(!inet_aton(argv[1], &addr_svr.sin_addr)) {
        fprintf(stderr, "invalid param ipaddr, ipaddr = %s\n", argv[1]);
        return 1;
    }
    printf("server addr init success, ip:port = %s:%s\n", argv[1], argv[2]);

    // connect to server
    int sock_conn = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_conn < 0) {
        perror("failed to create conn socket");
        fprintf(stderr, strerror(errno));
        return 1;
    }

    if(connect(sock_conn, (struct sockaddr *)&addr_svr, sizeof(addr_svr)) < 0) {
        perror("connect to server failed");
        fprintf(stderr, strerror(errno));
        return 1;
    }
    printf("connect to server success\n");

    int repeat = 1;
    if(argc == 5) {
        repeat = atoi(argv[4]);
        if(repeat <= 0)
            repeat = 1;
    }

    printf("ready to send data to server, send %s %d times\n", argv[3], repeat);
    int i;
    for(i=0; i<repeat; i++) {
        ssize_t cnt = write(sock_conn, argv[3], strlen(argv[3]));
        printf("repeat times %d, write %d characters to server\n", i, (int)cnt);
    }
    
    sleep(3);
    return 0;
}
