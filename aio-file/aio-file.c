#define _GNU_SOURCE     /* syscall() is not POSIX */

#include <stdio.h>      /* for perror() */
#include <unistd.h>     /* for syscall() */
#include <sys/syscall.h>    /* for __NR_* definitions */
#include <linux/aio_abi.h>  /* for AIO types and constants */
#include <fcntl.h>      /* O_RDWR */
#include <string.h>     /* memset() */
#include <inttypes.h>       /* uint64_t */

inline int io_setup(unsigned nr, aio_context_t * ctxp)
{
    return syscall(__NR_io_setup, nr, ctxp);
}

inline int io_destroy(aio_context_t ctx)
{
    return syscall(__NR_io_destroy, ctx);
}

inline int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp)
{
    return syscall(__NR_io_submit, ctx, nr, iocbpp);
}

inline int io_getevents(aio_context_t ctx, 
                        long min_nr, long max_nr, 
                        struct io_event *events, 
                        struct timespec *timeout)
{
    return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}

int main()
{
    int fd = open("./testfile", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if (fd < 0) {
        perror("open error");
        return -1;
    }

    // init aio context
    aio_context_t ctx = 0;

    int ret = io_setup(128, &ctx);
    if (ret < 0) {
        perror("io_setup error");
        return -1;
    }

    // setup I/O control block
    struct iocb cb;
    memset(&cb, 0, sizeof(cb));
    cb.aio_fildes = fd;
    cb.aio_lio_opcode = IOCB_CMD_PWRITE;

    // command-specific options
    char data[4096] = "i love you, dad!\n";
    cb.aio_buf = (uint64_t) data;
    cb.aio_offset = 0;
    cb.aio_nbytes = strlen(data);

    struct iocb *cbs[1];
    cbs[0] = &cb;

    ret = io_submit(ctx, 1, cbs);
    if (ret != 1) {
        if (ret < 0)
            perror("io_submit error");
        else
            fprintf(stderr, "could not sumbit IOs");

        return -1;
    }

    // get the reply 
    struct io_event events[1];
    ret = io_getevents(ctx, 1, 1, events, NULL);
    printf("%d io ops completed\n", ret);

    ret = io_destroy(ctx);
    if (ret < 0) {
        perror("io_destroy error");
        return -1;
    }

    return 0;
}
