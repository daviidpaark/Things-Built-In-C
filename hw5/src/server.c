/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>
#include <pthread.h>

#include "debug.h"
#include "pbx.h"
#include "server.h"

/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */
#if 0
void *pbx_client_service(void *arg)
{
    int connfd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());
    TU *tu = tu_init(connfd);
    pbx_register(pbx, tu, connfd);

    return NULL;
}
#endif
