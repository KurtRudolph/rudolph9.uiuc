#ifndef __MP1_H_
#define __MP1_H_

#include <pthread.h>

/*** unicast interface ***/

/* Should be called by multicast_init at start of program */
void unicast_init(void);

/* Send message (NUL-terminate) to destination */
void usend(int destination, const char *message);

/* Called *by* unicast interface when an incoming unicast message arrives from source */
void receive(int source, const char *message);

/*** multicast interface ***/

/* Call at start of program. */
void multicast_init(void);

/* Multicast message to multicast group (implicit) */
void multicast(const char *message);

/* Called *by* multicast protocol when an incoming multicast message is ready to be delivered */
void deliver(int source, const char *message);

/*** group maintenance ***/

extern int *mcast_members;
extern int mcast_num_members;
extern int my_id;
extern pthread_mutex_t member_lock;

/*** internals ***/

/* File where the list of current group members is stored */
#define GROUP_FILE  "GROUPLIST"

#endif
