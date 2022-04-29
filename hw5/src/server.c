/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "debug.h"
#include "pbx.h"
#include "server.h"

/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */
#if 1
void *pbx_client_service(void *arg)
{
    int connfd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());
    TU *tu = tu_init(connfd);
    pbx_register(pbx, tu, connfd);

    char buf[1024];
    int bytes;
    while (1)
    {
        bytes = read(connfd, buf, 1024);
        if (bytes <= 0)
        {
            return NULL;
        }

        if (strncmp(buf, "pickup", 6) == 0)
        {
            tu_pickup(tu);
        }
        else if (strncmp(buf, "hangup", 6) == 0)
        {
            tu_hangup(tu);
        }
        else if (strncmp(buf, "dial ", 5) == 0)
        {
            long num;
            num = strtol(buf + 5, NULL, 10);
            if (num <= 0)
                continue;
            pbx_dial(pbx, tu, num);
        }
        else if (strncmp(buf, "chat ", 5) == 0)
        {
            tu_chat(tu, buf + 5);
        }
    }

    return NULL;
}
#endif
