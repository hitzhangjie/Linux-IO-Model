module list:
.
├── client      : tcp client for testing purposes
├── io-epoll    : tcp server based on epoll lt/et modes
├── io-poll     : tcp server based on poll
├── io-select   : tcp server based on select
├── rtsig-tcp   : tcp server based on rtsig driven io, in which sigwaitinfo &
├                 siginfo_t are used to identify signal relevant file descriptor.
├── rtsig-tcp2  : tcp server based on rtsig driven io, in which sighandler and
├                 other techniques are used to process different connections,
├                 i think it's a bad way.
└── rtsig-udp   : udp server based on rtsig driven io, rtsig is raised only
                  when data arrives or error occurs, so it's easier than tcp.
                  this also contains an udp client for testing purposes.


