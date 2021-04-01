## Redis源码解析

2021-03-17

任何程序都是从main开始

###   Unix编程

#### [FD_CLOEXEC](https://my.oschina.net/u/3725204/blog/1588593)

close on exec, not on-fork 意思是如果对描述符设置了FD_CLOEXEC,使用execl执行的程序里,此描述符被关闭，此描述符不能再被使用. 但是在使用fork产生的子进程中此描述符不被关闭，仍然可以使用。

#### epoll | select|kqueue

不同平台使用不同的事件机制

```c
int epoll_create(int size); //创建epoll实例
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event); //add mod del 监听的fd
/*
	timeout = 0 立即返回
	timeout = -1 阻塞到事件发生为止
	timeout > 0 等待的事件到达或者timeout超时返回
 */
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

#### socket

```c
static int _anetTcpServer(char *err, int port, char *bindaddr, int af, int backlog)
{
    int s = -1, rv;
    char _port[6];  /* strlen("65535") */
    struct addrinfo hints, *servinfo, *p;

    snprintf(_port,6,"%d",port);
    memset(&hints,0,sizeof(hints));
    hints.ai_family = af;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    /* No effect if bindaddr != NULL */
    if (bindaddr && !strcmp("*", bindaddr))
        bindaddr = NULL;
    if (af == AF_INET6 && bindaddr && !strcmp("::*", bindaddr))
        bindaddr = NULL;

    if ((rv = getaddrinfo(bindaddr,_port,&hints,&servinfo)) != 0) {
        anetSetError(err, "%s", gai_strerror(rv));
        return ANET_ERR;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
            continue;

        if (af == AF_INET6 && anetV6Only(err,s) == ANET_ERR) goto error;
        if (anetSetReuseAddr(err,s) == ANET_ERR) goto error;
        if (anetListen(err,s,p->ai_addr,p->ai_addrlen,backlog) == ANET_ERR) s = ANET_ERR;
        goto end;
    }
    if (p == NULL) {
        anetSetError(err, "unable to bind socket, errno: %d", errno);
        goto error;
    }

error:
    if (s != -1) close(s);
    s = ANET_ERR;
end:
    freeaddrinfo(servinfo);
    return s;
}
```

标准的socket server编程

![](http://media.geeksforgeeks.org/wp-content/uploads/Socket-Programming-in-C-C-.jpg)

