#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/time.h>

#include "mp1.h"
#include "list.h"



/*
 Machine Problem 1
 * Md Tanvir Al Amin 
 * Kurt Rudolph
 * maamin2@illinois.edu, rudolph9@illinois.edu
 * University of Illinois at Urbana-Champaign
 */
 
/* Protocol 

 Message acknowledgments about reception is included in ping
 
 Message Types:
 1 : Periodic / Heartbit
 - 1 byte for message type
 - mcast_num_members * sizeof(int) bytes for ack



 2 : Data message
 - 1 byte for message type
 - sizeof(int) bytes for message source_index
 - mcast_num_members * sizeof(int) bytes for timestamp
 - null terminated string as payload (message)

 3 : Transmission Request
 - 1 byte for message type
 - sizeof(int) bytes for what number
 - sizeof(int) bytes for whose message is missing



 Only data messages do increase the timestamp, other messages do not increase timestamp
 One thread per participant to handle message from that participant
 */


#define MSG_HEARTBEAT 1
#define MSG_DATA 2
#define MSG_RETRANSMIT 3

/* variables initialized once and never changed later */
/* to access these variable we do not need locks once chatting has started*/
int mcast_num_members_local = 0; /* as we do not have the number of members correctly at the start of the program, the program counts at each mcast_join*/
int my_id_index = -1;
int size_of_timestamp = -1; /* size of the timestamp vector in bytes */
int initialization_complete = 0; /* Race condition may arrive at the very first. Let us be safe*/

pthread_mutex_t timestamp_lock = PTHREAD_MUTEX_INITIALIZER;
int ** timestamp = NULL; /*Vector Timestamp*/
int sum_timestamp = 0;

pthread_mutex_t ack_lock = PTHREAD_MUTEX_INITIALIZER;
int *ack = NULL; /* ack[i] means Upto what sequence number have we recieved the messages from participant i (in consecutive order) */
int *news_ack = NULL; /* news_ack[i] means upto what sequence number have we received the messages from participant i */


pthread_mutex_t max_ack_lock = PTHREAD_MUTEX_INITIALIZER;
int *max_ack = NULL; /* max_ack[i] = upto what seq no someone has received anything from process i*/

pthread_mutex_t receive_ack_lock = PTHREAD_MUTEX_INITIALIZER;
int *receive_ack = NULL; /* receive_ack[i] means upto what sequence process i has got my messages */

#define HEARTBEAT_MISS_THRESHOLD 6

pthread_mutex_t heartbeat_lock = PTHREAD_MUTEX_INITIALIZER;
long int *heartbeat = NULL;
int *missed_heartbeat = NULL; /* missed_heartbeat [i] = 0 means process i is alive. missed_heartbeat [i] > HEARTBEAT_MISS_THRESHOLD means not alive (didn't heard from a long time)*/

void B_multicast(const char * message, int len);
void *participant_handler_thread(void *my_participant);
void *heartbeat_sender_thread(void *my_index);

/* We save the incoming DATA messages in this structures*/

/* Each element does belong to two linked lists */
struct message_store {
    int source_index; /* Which participant sent this message? */
    int *timestamp; /* What are the timestamp values that came with this message? (helps account for acks)*/
    int sumstamp; /* Sum of the timestamp vector */
    char *payload; /* The actual thing to display */
    struct list_head hold_back; /* Linked list of the messages in the hold back queue */
    struct list_head fifo_order; /* Linked list of the messages from same source as this. Stored in fifo order */
};

/* Linux Kernel linked list description http://isis.poly.edu/kulesh/stuff/src/klist/ */

/* There is one thread for handling message from EACH participants*/
pthread_t *participant_thread = NULL;

/* There is one thread for heartbeating */
pthread_t heartbeating_thread;

/* TODO: Decide who is going to cleanup dynamically allocated **timestamp, *participant_thread */

/* The hold back queue */
struct message_store *hold_back_queue = NULL;
/* Lock for hold back queue */
pthread_mutex_t hold_back_lock = PTHREAD_MUTEX_INITIALIZER;

/* The buckets (linked list) associated with each participant */
/* participant_bucket[i] contains the messages from participant i */
struct message_store **participant_bucket = NULL;
struct message_store **participant_jump = NULL;

/* Lock for participant bucket */
pthread_mutex_t *participant_bucket_lock = NULL;

/* Constructor function */
struct message_store *new_message_store(int si, int *ts, char *msg) {
    struct message_store *ms;
    int i, nmem = size_of_timestamp / sizeof (int);

    ms = (struct message_store *) malloc(sizeof (struct message_store));

    ms->source_index = si;

    /* preserve the message timestamp in the message_store*/
    ms->timestamp = (int *) malloc(size_of_timestamp);
    memcpy(ms->timestamp, ts, size_of_timestamp);

    /* preserve the message itself */
    ms->payload = strdup(msg); /* msg is a null terminated string */

    /* Save the sum of the timestamp vector */
    ms->sumstamp = 0;
    for (i = 0; i < nmem; i++) {
        ms->sumstamp += ts[i];
    }

    /* Initialize the lists*/
    /* TODO: These initialize might be unnecessary. Scrutinize program later. If subsequenct calls are list_add etc, then initialize is not necessary*/
    //INIT_LIST_HEAD(& (ms -> hold_back)  );
    //INIT_LIST_HEAD(& (ms -> fifo_order) );
    return ms;
}

/* Destructor function */
void delete_message_store(struct message_store *ms) {
    /* Delete the timestamp*/
    free(ms->timestamp);

    /* Delete payload */
    free(ms->payload);

    /* Delete list elements*/
    list_del(&(ms->hold_back));
    list_del(&(ms->fifo_order));

    /* free the message store*/
    free(ms);
}

int get_index_from_id(int id) {
    int i;
    pthread_mutex_lock(&member_lock);
    for (i = 0; i < mcast_num_members; i++) {
        if (mcast_members[i] == id) {
            break;
        }
    }
    pthread_mutex_unlock(&member_lock);
    assert(i < mcast_num_members);
    return i;
}

void retransmit(int whom, int num, int dest_index) {

    struct message_store *elem;
    char * protocol_message;
    int len;

    pthread_mutex_lock(&participant_bucket_lock[whom]);

    list_for_each_entry(elem, & (participant_bucket[whom] -> fifo_order), fifo_order) {

        if (elem->timestamp[whom] >= num)
            break;
    }
    // do you really have this message ? if not, simply return
    if ( elem->payload == NULL || elem->timestamp[whom] != num)
    {
        pthread_mutex_unlock(&participant_bucket_lock[whom]);
        return; 
    }
    
    len = 1 + sizeof(int) + size_of_timestamp + sizeof (char) * (strlen(elem->payload) + 1);

    /*Cook the timestamp and the message in a contiguous array*/
    protocol_message = (char *) malloc(len);
    protocol_message[0] = MSG_DATA;
    memcpy (protocol_message +1 , & (elem->source_index), sizeof(int) );
    memcpy(protocol_message + 1 + sizeof(int), elem->timestamp, size_of_timestamp);

    memcpy(protocol_message + 1 + sizeof(int) + size_of_timestamp, elem->payload,
            sizeof (char) * (strlen(elem->payload) + 1));

    pthread_mutex_unlock(&participant_bucket_lock[whom]);
    usend(mcast_members[dest_index], protocol_message, len);

    /* (Done) TODO: An important question. Should we keep the messages saved? For how long*/
    free(protocol_message);

}

/* Basic multicast implementation */
void B_multicast(const char *message, int len) {
    int i;

    /* Send it to myself directly. When I am sending it to myself, I want to make sure that I am getting it */
    /* This way I do not need to wait for acknowledgement of my own message */

    /* TODO: Caution this is very important. Here the sender thread is actually accessing the data structures
     which are meant to be handled by the receiver thread.
     */

    receive(my_id, message, len);

    pthread_mutex_lock(&member_lock);

    for (i = 0; i < mcast_num_members; i++) {
        if (i != my_id_index) {
            /* When I am sending it to others*/
            usend(mcast_members[i], message, len);
        }
    }
    pthread_mutex_unlock(&member_lock);

}

void multicast(const char *message) {
    //struct protocol_message pmessage;
    char * protocol_message;
    int len = 1 + sizeof(int) + size_of_timestamp + sizeof (char) * (strlen(message) + 1);

    /*Cook the timestamp and the message in a contiguous array*/
    protocol_message = (char *) malloc(len);
    protocol_message[0] = MSG_DATA;

    memcpy(protocol_message + 1, &(my_id_index), sizeof(int)) ;
    
    pthread_mutex_lock(&timestamp_lock);
    {
        /*Update timestamp as described in CO Multicast protocol*/

        timestamp[my_id_index][my_id_index] =
                timestamp[my_id_index][my_id_index] + 1;
        sum_timestamp++;

        memcpy(protocol_message +1 +sizeof(int), timestamp[my_id_index], size_of_timestamp);
    }
    pthread_mutex_unlock(&timestamp_lock);

    memcpy(protocol_message + 1 + sizeof(int) + size_of_timestamp, message,
            sizeof (char) * (strlen(message) + 1));

    B_multicast(protocol_message, len);

    /* (Done) TODO: An important question. Should we keep the messages saved? For how long*/
    free(protocol_message);

}

void join_initialize() {
    int i, rc;
    size_of_timestamp = sizeof (int) * mcast_num_members_local;
    struct message_store *temp;

    pthread_mutex_lock(&timestamp_lock);
    {
        timestamp = (int**) realloc(timestamp,
                sizeof (int*) * mcast_num_members_local);
        timestamp[mcast_num_members_local - 1] = NULL;

        for (i = 0; i < mcast_num_members_local; i++) {
            timestamp[i] = (int*) realloc(timestamp[i],
                    sizeof (int) * mcast_num_members_local);
            /* Initialization of the vector timestamp */

            memset(timestamp[i], 0, sizeof (int) * mcast_num_members_local);
        }
    }
    pthread_mutex_unlock(&timestamp_lock);

    /* Create the acknowledgment and receive acknowledgments */

    pthread_mutex_lock(&ack_lock);
    ack = (int *) realloc(ack, sizeof (int) * mcast_num_members_local);
    ack[mcast_num_members_local - 1] = 0;
    
    news_ack = (int *)realloc(news_ack, sizeof(int) * mcast_num_members_local);
    news_ack[mcast_num_members_local - 1 ] = 0;
    pthread_mutex_unlock(&ack_lock);
    
    
    pthread_mutex_lock(&max_ack_lock);
    max_ack = (int *)realloc(max_ack, sizeof(int) * mcast_num_members_local);
    max_ack[mcast_num_members_local - 1] = 0;
    pthread_mutex_unlock(&max_ack_lock);

    pthread_mutex_lock(&receive_ack_lock);
    receive_ack = (int *) realloc(receive_ack,
            sizeof (int) * mcast_num_members_local);
    receive_ack[mcast_num_members_local - 1] = 0;
    pthread_mutex_unlock(&receive_ack_lock);

    pthread_mutex_lock(&heartbeat_lock);
    heartbeat = (long int *) realloc(heartbeat,
            sizeof (long int) * mcast_num_members_local);
    heartbeat[mcast_num_members_local - 1] = 0;

    missed_heartbeat = (int *) realloc(missed_heartbeat,
            sizeof (long int) * mcast_num_members_local);
    missed_heartbeat[mcast_num_members_local - 1] = 0;
    pthread_mutex_unlock(&heartbeat_lock);

    /* Create a new lock for the new participant bucket*/
    participant_bucket_lock = (pthread_mutex_t*) realloc(
            participant_bucket_lock,
            sizeof (pthread_mutex_t) * mcast_num_members_local);
    /* Initialize that lock*/
    //participant_bucket_lock[ mcast_num_members_local - 1] = PTHREAD_MUTEX_INITIALIZER;
    //
    pthread_mutex_init(
            &participant_bucket_lock[mcast_num_members_local - 1],
            NULL /*TODO: Check what to put here to initialize dynamically allocated mutexes */);

    /* Resize the participant_bucket*/
    participant_bucket = (struct message_store **) realloc(participant_bucket,
            sizeof (struct message_store *) * mcast_num_members_local);
    participant_jump = (struct message_store **) realloc(participant_jump,
            sizeof (struct message_store *) * mcast_num_members_local);

    /* Create the bucket required for this thread*/

    pthread_mutex_lock(&participant_bucket_lock[mcast_num_members_local - 1]);
    participant_bucket[mcast_num_members_local - 1] =
            (struct message_store *) malloc(sizeof (struct message_store)); /* Dummy Element */
    participant_bucket[mcast_num_members_local - 1]->timestamp = NULL;
    participant_bucket[mcast_num_members_local - 1]->payload = NULL;
    INIT_LIST_HEAD(
            & (participant_bucket[mcast_num_members_local - 1]->fifo_order));

    participant_jump[mcast_num_members_local - 1] =
            participant_bucket[mcast_num_members_local - 1];
    pthread_mutex_unlock(&participant_bucket_lock[mcast_num_members_local - 1]);

    /* The thread can now be created */
    /* Create a new thread for the new participant */
    participant_thread = (pthread_t *) realloc(participant_thread,
            sizeof (pthread_t) * mcast_num_members_local);
    rc = pthread_create(&participant_thread[mcast_num_members_local - 1], NULL,
            participant_handler_thread, (void *) (mcast_num_members_local - 1));

}

void multicast_init(void) {
    int i;
    unicast_init();

    /* In almost all cases, it will succeed in the first chance*/
    pthread_mutex_lock(&member_lock);
    // O(1) actually
    for (i = mcast_num_members - 1; i >= 0; i--) {
        if (my_id == mcast_members[i]) {
            my_id_index = i;
            break;
        }
    }

    pthread_mutex_unlock(&member_lock);

    /*Create the hold back queue */
    pthread_mutex_lock(&hold_back_lock);
    hold_back_queue = (struct message_store *) malloc(
            sizeof (struct message_store)); /* Dummy Element */
    hold_back_queue->timestamp = NULL;
    hold_back_queue->payload = NULL;
    INIT_LIST_HEAD(& (hold_back_queue -> hold_back));
    pthread_mutex_unlock(&hold_back_lock);
    /* hold_back_queue is the dummy node for hold_back_queue. */

    /* Create the heartbeating thread */
    pthread_create(&heartbeating_thread, NULL, heartbeat_sender_thread,
            (void *) (my_id_index));

    initialization_complete = 1;

}

int try_to_deliver() {
    /* sum of my current timestamp is sum_timestamp */
    /* We need to check the hold_back_queue upto the point */

    /*	pthread_mutex_lock(&timestamp_lock);

     pthread_mutex_unlock(&timestamp_lock);
     */
    int k, cond1, cond2, i;
    struct message_store *elem, *dummy;

    pthread_mutex_lock(&hold_back_lock);

    list_for_each_entry_safe(elem, dummy, &(hold_back_queue->hold_back), hold_back)
            /* We are using list_for_each_entry_safe because elements can get deleted */ {
        if (elem->sumstamp > sum_timestamp + 1) /* if sum_of_message_timestamp exceeds sum_of_my_local_timestamp + 1, I can't display it anyway */
            break; /* The list is sorted in the order of sum_of_timestamp */

        /* now check if I can display elem */

        /* Diagnostic: Print the timestamps*/
        /*printf("from: %d [%d]\n=> ", elem->source_index,
                mcast_members[elem->source_index]);
        for (i = 0; i < mcast_num_members; i++) {
            printf("%d ", elem->timestamp[i]);
        }
        printf("\n=> ");
         */ 

        pthread_mutex_lock(&timestamp_lock);
        /*
        for (i = 0; i < mcast_num_members; i++) {
            printf("%d ", timestamp[my_id_index][i]);
        }
        printf("\n");
         */
        /* Compare local timestamp with the timestamp of the message*/

        /* Condition 1 */
        if (elem->source_index == my_id_index)
            cond1 = 1;
        else
            cond1 = (elem->timestamp[elem->source_index]
                == timestamp[my_id_index][elem->source_index] + 1);

        /* Condition 2 */

        /*Theoretically we should ask for member_lock here, but we do not do it not to accidentally put into deadlock */
        /*TODO: Do more research */
        /* Actually member_lock is not necessary here, because the objects the member_lock is protecting is not going to change because
         WE DO NOT HAVE DYNAMIC GROUP MANAGEMENT
         */
        /* TODO: Remember, for test purpose we disabled the message dropping 'functionality' */

        cond2 = 1;
        for (k = 0; k < mcast_num_members; k++) {
            if (k == elem->source_index)
                continue;

            if (elem->timestamp[k] > timestamp[my_id_index][k]) {
                cond2 = 0;
                break;
            }
        }
        if (cond1 && cond2) {
            /* Congratulations: You can display this finally */
            /* Deliver the message */
            deliver(mcast_members[elem->source_index], elem->payload);

            /* Now, we do not need this message in the hold_back queue. Delete it from there */
            list_del_init(&(elem->hold_back));

            /* Update your own timestamp if you are not the source*/
            if (elem->source_index != my_id_index) {
                timestamp[my_id_index][elem->source_index]++;
                sum_timestamp++;
            }

        }

        pthread_mutex_unlock(&timestamp_lock);
    }

    pthread_mutex_unlock(&hold_back_lock);
}

/* TRY TO MAKE THIS RECEIVE FUNCTION RE-ENTRANT*/
void receive(int source, const char *message, int len) {

    int source_index, message_type;
    //assert(message[len-1] == 0);

    /* What is the type of this message*/
    message_type = message[0];
    source_index = get_index_from_id(source);

    switch (message_type) {
        case MSG_HEARTBEAT:
        {

            pthread_mutex_lock(&heartbeat_lock);
            struct timeval nowtime;
            gettimeofday(&nowtime, NULL);
                heartbeat[source_index] = nowtime.tv_usec;
                missed_heartbeat[source_index] = 0; /* I received a heartbeat. So things are fresh */
            pthread_mutex_unlock(&heartbeat_lock);

            /* get upto what extent process source_index has received my messages */
            pthread_mutex_lock(&receive_ack_lock);
            memcpy(receive_ack + source_index,
                    message + 1 + source_index * sizeof (int), sizeof (int));
            pthread_mutex_unlock(&receive_ack_lock);
            
            /* */
            int dd, kk;
            pthread_mutex_lock(&max_ack_lock);
            for(kk = 0; kk < mcast_num_members; kk++)
            {
                memcpy( &dd, message + sizeof(int) * kk, sizeof(int) );
                
                if( dd > max_ack [kk])
                {
                    max_ack [kk] = dd;
                }
            }
            
            pthread_mutex_unlock(&max_ack_lock);
            

            break;

        }

        case MSG_DATA:
        {

            struct message_store *elem, *new_msg;
            // get source index from message
            memcpy( &source_index, message + 1, sizeof(int));

            /* Create a new message store item */
            new_msg =
                    new_message_store(
                    source_index,
                    (int *) (message + 1 + sizeof(int)), (char *) (message + 1 + sizeof(int) + size_of_timestamp));

            /* Update acknowledgments */
            pthread_mutex_lock(&ack_lock);
            if (new_msg->timestamp[new_msg->source_index] == ack[source_index] + 1)
                ack[source_index]++;
                
                //update news_ack
                if ( new_msg->timestamp[source_index] > news_ack[source_index] )
                {
                        news_ack[source_index] = new_msg->timestamp[source_index];
                }
            
            pthread_mutex_unlock(&ack_lock);

            /* Vector merge operation of the timestamps*/

            pthread_mutex_lock(&timestamp_lock);
            int i;
            for (i = 0; i < mcast_num_members; i++) {
                if (new_msg->timestamp[i] > timestamp[source_index][i])
                    timestamp[source_index][i] = new_msg->timestamp[i];
            }
            pthread_mutex_unlock(&timestamp_lock);

            /* First check for repeated message. Did I already received it and displayed it? */

            pthread_mutex_lock(&timestamp_lock);
            if (new_msg->source_index == my_id_index) {
                /* If I am the sender ?*/
                if (new_msg->timestamp[source_index]
                        < timestamp[my_id_index][source_index]) {
                    pthread_mutex_unlock(&timestamp_lock);
                    return;
                }
            } else if (new_msg->timestamp[source_index]
                    <= timestamp[my_id_index][source_index]) {
                // already received it 
                pthread_mutex_unlock(&timestamp_lock);
                try_to_deliver(); //////////////////// just for efficiency
                return;
            }
            pthread_mutex_unlock(&timestamp_lock);

            /* Is it a repeated message? Did I receive this message already somehow? In that case just discard it and we are done
             Otherwise, Put it in the queue for the correct process */

            /* Acquire the lock*/

            pthread_mutex_lock(&participant_bucket_lock[source_index]);

            list_for_each_entry(elem, &(participant_bucket[source_index]->fifo_order), fifo_order) {

                pthread_mutex_unlock(&participant_bucket_lock[source_index]);
                pthread_mutex_lock(&timestamp_lock);
                if (elem->timestamp[source_index]
                        == new_msg->timestamp[source_index]) {
                    /* Handle duplication. This works only if this message is found in the participant_bucket

                     which means that I may or may not have displayed it, but it haven't received acknowledgement for it from everyone
                     and I haven't deleted this message yet.
                     */
                    //pthread_mutex_unlock(&participant_bucket_lock[source_index]);
                    pthread_mutex_unlock(&timestamp_lock);
                    
                    try_to_deliver(); //////////////////// just for efficiency
                    return;
                } else if (elem->timestamp[source_index]
                        > new_msg->timestamp[source_index]) {
                    pthread_mutex_unlock(&timestamp_lock);
                    break;
                }
                pthread_mutex_unlock(&timestamp_lock);
                pthread_mutex_lock(&participant_bucket_lock[source_index]);
            }
            /* Insert new_msg to the link list, before elem*/
            list_add_tail(&(new_msg->fifo_order), &(elem->fifo_order));

            /* Release the lock*/
            pthread_mutex_unlock(&participant_bucket_lock[source_index]);

            /* Put it in the hold_back queue
             TODO: But we need to find the appropriate place where to insert
             Ans: Use sumstamp to insert it into the proper place */

            pthread_mutex_lock(&hold_back_lock);

            list_for_each_entry(elem, &(hold_back_queue->hold_back), hold_back) {
                /* now elem is the pointer to correct item */
                if (elem->sumstamp > new_msg->sumstamp)
                    break;
            }
            /* Now we should insert new_msg to the link list, before elem */
            list_add_tail(&(new_msg->hold_back), &(elem->hold_back));
            pthread_mutex_unlock(&hold_back_lock);

            /* Upon receiving a data message the sender thread will check if it has any message to display */

            /* The receiver thread now checks if it can display something*/

            try_to_deliver();

            break;
        }
        case MSG_RETRANSMIT:
        {

            int num = *(int *) (message + 1);
            int whom = *(int *) (message + 1 + sizeof (int));
           // printf("Retransmission request received  from %d for me message %d\n", source_index, num);

            /* retransmitting message*/
            retransmit(whom, num, source_index);
            //printf("Retransmission done to %d\n", source_index);

            break;
        }
    }

    /*TODO: Extract the timestamp out of this message and update your timestamp accordingly (vector timestamp merge)
     Plainly extracting the timestamp as each message arrives doesn't work. Take a judicial decision of what to do
     This update is for the acknowledgement purpose.
     Also keep in mind that there is no merging / acknowledgment operation when the sender and the receiver are the same
     */
    /* TODO: This code will change*/

    /*pthread_mutex_lock(&timestamp_lock);
     memcpy(timestamp[source_index], message+1,size_of_timestamp);
     pthread_mutex_unlock(&timestamp_lock);
     */

}

void *heartbeat_sender_thread(void *my_index) {

    int heartbeat_interval = 10 * MAXDELAY;
    //printf("Hi, I am heartbeating thread\n");

    while (initialization_complete <= 0) {
        /* Spin-lock before initial cooking is finished */
       // printf("hungry %d", my_index);
        sleep(1);
    }

    while (1) {
        int i;
        pthread_mutex_lock(&member_lock);

        pthread_mutex_lock(&heartbeat_lock);

        for (i = 0; i < mcast_num_members; i++) {
            if (i == my_id_index)
                continue; /* Do not need my own heartbeats */
            missed_heartbeat[i]++; /* As long as missed_heartbeat <= HEARTBEAT_MISS_THRESHOLD, the process is taken to be alive */
            if (missed_heartbeat[i] > HEARTBEAT_MISS_THRESHOLD) {
               // printf("!!!! %d Dead Process !!!!\n", i);
            }
        }
        pthread_mutex_unlock(&heartbeat_lock);
        
       // pthread_mutex_unlock(&member_lock);

        

        int len = 1 + mcast_num_members * sizeof (int);
        /* Send a heartbeat to everyone */
        char *hb = (char *) malloc(len);

        hb[0] = MSG_HEARTBEAT;

        pthread_mutex_lock(&ack_lock);
        //heartbeat generally contain what I have received at max from all processes (news_ack)
        // but prior to sending it to process i, it contains what I have received from process i in consecutive order (ack)
        memcpy(hb + 1, news_ack, sizeof (int) * mcast_num_members);
        pthread_mutex_unlock(&ack_lock);

        for (i = 0; i < mcast_num_members; i++) {
            if (i == my_id_index)
                continue;
            //printf("......");
            memcpy ( hb + 1 + i * sizeof(int), ack + i, sizeof(int));
            usend(mcast_members[i], hb, len);
        }
        pthread_mutex_unlock(&member_lock);

        usleep(heartbeat_interval);
    }

}

void *participant_handler_thread(void *my_participant) {

    int handler_index = (int) my_participant;

    while (my_id_index == -1) {
        /* Spin-lock before initial cooking is finished */
       // printf("hungry\n");
        sleep(1);
    }

    int wait_time_before_nak = (MAXDELAY - MINDELAY) / 1000000;
    int wait_time_before_next_round = (2* MAXDELAY) / 1000000;
    int i, can_delete, jmjm, what_is_missing, what_is_missing_now;
    struct message_store *elem_del = participant_bucket[handler_index],
            *temp_del, *elem_jump, *elem_jump_next, *elem_jump_prev;

    int *tss = (int *) malloc(sizeof (int) * mcast_num_members);
    memset(tss, 0, mcast_num_members * sizeof (int));

    pthread_mutex_lock(&participant_bucket_lock[handler_index]);

    participant_bucket[handler_index]->timestamp = tss;
    pthread_mutex_unlock(&participant_bucket_lock[handler_index]);

   // printf("Hi, I am handler thread for %d\n", handler_index);

    /*
     Use a condition variable to check if there is item in its queue.
     */

    /* TODO: Check what the participant_handler_thread for my own messages do*/
    if (handler_index == my_id_index) {
        return (void *) 0;
    }
    /* jump node = partcipant_jump [handler_index] */

    while (1) {

        pthread_mutex_lock(&participant_bucket_lock[handler_index]);

        list_for_each_entry_safe(elem_del, temp_del, & (participant_bucket [handler_index] -> fifo_order), fifo_order) {
            //printf("inside deletion\n");

            if (elem_del
                    == participant_jump[handler_index]) /* Then the node we are considering is the jump node */ {
                //pthread_mutex_lock( &participant_bucket_lock[handler_index] );
                break;
            }
            pthread_mutex_unlock(&participant_bucket_lock[handler_index]);
            /* verify I can delete elem */
            /* check if everybody has received it*/
            /* Don't need to do the member lock */
            can_delete = 1;
            pthread_mutex_lock(&timestamp_lock);
            for (i = 0; i < mcast_num_members; i++) {

                //printf("accessing ts %d, %d\n",i, elem_del -> timestamp);

                if (timestamp[i][handler_index]
                        < elem_del->timestamp[handler_index]) {
                    can_delete = 0;

                    break;
                }

            }
            pthread_mutex_unlock(&timestamp_lock);

            if (can_delete == 1) {
                /* we can delete it */
                pthread_mutex_lock(&participant_bucket_lock[handler_index]);
                //printf("We can delete\n");
                list_del_init(&(elem_del->fifo_order));
                /* TODO: Add delete_message_store function */
                //pthread_mutex_unlock( &participant_bucket_lock[handler_index] );
            } else {
                pthread_mutex_lock(&participant_bucket_lock[handler_index]);
                break;
            }
            //pthread_mutex_lock( &participant_bucket_lock[handler_index] );

        }
        pthread_mutex_unlock(&participant_bucket_lock[handler_index]);

        /* participant_jump [handler_index] */

        pthread_mutex_lock(&participant_bucket_lock[handler_index]);

        // initialization
        elem_jump_prev = participant_bucket[handler_index];

        do
        {
            /* if it goes past the last element ?? */
            
            // if list has elements then iteration starts from first non-dummy
            // if list has no elements other than dummy than iteration starts from there
            elem_jump =
                    list_entry((elem_jump_prev->fifo_order).next, struct message_store, fifo_order);
            elem_jump_next =
                    list_entry((elem_jump->fifo_order).next, struct message_store, fifo_order);

            pthread_mutex_unlock(&participant_bucket_lock[handler_index]);

            pthread_mutex_lock(&timestamp_lock);

            // jump detector starts
            jmjm = 0;
            if (elem_jump->payload == NULL) // is it the last iteration ? (or the case where we have only the dummy ?
            {
                // check with global timestamp, what according to my knowledge anybody has received at max
                int max_others = 0;
                for (i = 0; i < mcast_num_members; i++) {
                    if (timestamp[i][handler_index] > max_others)
                        max_others = timestamp[i][handler_index];
                }
                
                
                pthread_mutex_lock(&max_ack_lock);
                // check with max_ack, upto what extent anybody has received 
                if ( max_ack[handler_index] > max_others)
                    max_others = max_ack[handler_index];
                pthread_mutex_unlock(&max_ack_lock);

                if (elem_jump_prev->payload == NULL
                        && elem_jump_next->payload == NULL) {
                    // only the dummy element exists in the list
                    // may be I received nothing so far, or I received and displayed and deleted
                    // so check what I have displayed so far, and what everybody has received so far
                    //printf("only dummy %d %d\n",my_id_index,handler_index);
                    if (max_others > timestamp[my_id_index][handler_index]) {
                        jmjm = 1;
                        what_is_missing = timestamp[my_id_index][handler_index] + 1; ////////// changed proactively 8 oct 1:29 PM ( added +1 )
                    }
                } else // the list does contain elements other than dummy, but I am at the last iteration
                {
                    if (max_others > elem_jump_prev->timestamp[handler_index]) {
                        jmjm = 1;
                        what_is_missing =
                                elem_jump_prev->timestamp[handler_index] + 1; ////////// changed proactively 8 oct 1:29 PM ( added +1 )
                    }
                }
            } else if (elem_jump_prev->payload == NULL) // first iteration, prev element is dummy
            {
                if (elem_jump->timestamp[handler_index]
                        > timestamp[my_id_index][handler_index] + 1) {
                    jmjm = 1; /* there is a jump */
                    // printf("diagonal1\n");
                    what_is_missing = timestamp[my_id_index][handler_index] + 1;
                }
            } else // for subsequent iterations other than the last
            {
                if ((elem_jump->timestamp[handler_index]
                        > elem_jump_prev->timestamp[handler_index] + 1)) {

                    //printf("normal1\n");
                    jmjm = 1;
                    what_is_missing = elem_jump_prev->timestamp[handler_index]
                            + 1;
                }
            } // jump detector ends

            if (jmjm == 1) {
               // printf("Detected a jump\n");

                /* Start proper retransmission procedure */
                pthread_mutex_unlock(&timestamp_lock);
                sleep(wait_time_before_nak);

                pthread_mutex_lock(&participant_bucket_lock[handler_index]);

                elem_jump = list_entry((elem_jump_prev->fifo_order).next, struct message_store, fifo_order);
                        elem_jump_next =
                        list_entry((elem_jump->fifo_order).next, struct message_store, fifo_order);
                
                pthread_mutex_unlock(&participant_bucket_lock[handler_index]);

                pthread_mutex_lock(&timestamp_lock);

                // jump detector starts
                jmjm = 0;
                what_is_missing_now = -1;
                if (elem_jump->payload == NULL) // is it the last iteration ? (or the case where we have only the dummy ?
                {
                    // check with global timestamp, what according to my knowledge anybody has received at max
                    int max_others = 0;
                    for (i = 0; i < mcast_num_members; i++) {
                        if (timestamp[i][handler_index] > max_others)
                            max_others = timestamp[i][handler_index];
                    }
                    pthread_mutex_lock(&max_ack_lock);
                    // check with max_ack, upto what extent anybody has received 
                    if ( max_ack[handler_index] > max_others)
                        max_others = max_ack[handler_index];
                    pthread_mutex_unlock(&max_ack_lock);

                    if (elem_jump_prev->payload == NULL
                            && elem_jump_next->payload == NULL) {
                        // only the dummy element exists in the list
                        // may be I received nothing so far, or I received and displayed and deleted
                        // so check what I have displayed so far, and what everybody has received so far
                        if (max_others
                                > timestamp[my_id_index][handler_index]) {
                            jmjm = 1;
                            what_is_missing_now =
                                    timestamp[my_id_index][handler_index] + 1; ////////// changed after bug 8 oct 1:29 PM ( added +1 )

                        }
                    } else // the list does contain elements other than dummy, but I am at the last iteration
                    {
                        if (max_others
                                > elem_jump_prev->timestamp[handler_index]) {
                            jmjm = 1;
                            what_is_missing_now =
                                    elem_jump_prev->timestamp[handler_index] + 1; ////////// changed proactively 8 oct 1:29 PM ( added +1 )

                        }
                    }
                } else if (elem_jump_prev->payload == NULL) // first iteration, prev element is dummy
                {
                    if (elem_jump->timestamp[handler_index]
                            > timestamp[my_id_index][handler_index] + 1) {
                        jmjm = 1; /* there is a jump */
                        // printf("diagonal1\n");
                        what_is_missing_now = timestamp[my_id_index][handler_index]
                                + 1;
                    }
                } else // for subsequent iterations other than the last
                {
                    if ((elem_jump->timestamp[handler_index]
                            > elem_jump_prev->timestamp[handler_index] + 1)) {

                        // printf("normal1\n");
                        jmjm = 1;
                        what_is_missing_now =
                                elem_jump_prev->timestamp[handler_index] + 1;
                    }
                } // jump detector ends



                if (jmjm == 1) {

                    if (what_is_missing_now == what_is_missing) {
                        /* TODO: Negative acknowledgment gets multiple times */

                        /* TODO: Select a random node whom to ask for the message */
                        /* TODO: Remember, we need to change the MSG_DATA format, and add a "sender" field to a message */
                        // int whom_to_ask = rand() % mcast_num_members ;
                        int whom_to_ask = -1, try = 0;
                        pthread_mutex_lock(&heartbeat_lock);
                        for (try=0; ; try++)
                        {
                            if(try > 3 * mcast_num_members) 
                            {
                                whom_to_ask = handler_index;
                                break;
                            }
                            whom_to_ask = rand() % mcast_num_members;
                            if (whom_to_ask == my_id_index) continue; /* should not ask myself for retransmission :p */
                            
                                if ( missed_heartbeat [ whom_to_ask ] > HEARTBEAT_MISS_THRESHOLD ) 
                                {
                                    continue; /*does he seem to be dead ? */
                                }
                                else
                                {
                                    break;
                                }
                        }
                         pthread_mutex_unlock(&heartbeat_lock);
                        //printf("Negative Acknowledgment for message #%d fromm %d\n", what_is_missing, whom_to_ask);

                        {
                            int size_of_nak = sizeof (char) * (2 * sizeof (int) + 1);
                            char *nak = (char *) malloc(size_of_nak);
                            int req = what_is_missing;
                            nak[0] = MSG_RETRANSMIT;

                            memcpy(nak + 1, &req, sizeof (int));
                            memcpy(nak + 1 + sizeof (int), &handler_index,
                                    sizeof (int));

                            /* May be the original sender had crashed !!*/
                            /* Currently we are asking for retransmission to the original sender only */
                            usend(mcast_members[whom_to_ask], nak, size_of_nak);
                            free(nak);
                        }

                        // the jump is still there. Nothing arrived when I as sleeping, or incorrect element arrived 
                        //so the loop should not proceed 
                        // but I have sent a request
                        // should we or should we not update elem_jump_prev as usual ?
                                
                                /* if we decide to update  */
                                
                                /* comment out if we decide not to update*/
                                //////// start commenting
                                pthread_mutex_lock(&participant_bucket_lock[handler_index]);
                                elem_jump_prev =
                                list_entry((elem_jump_prev->fifo_order).next, struct message_store, fifo_order);
                        pthread_mutex_unlock(&participant_bucket_lock[handler_index]);
                                /////////// end commenting 

                    } else {
                        // the jump is there, but for some other element
                        
                        // this should be unreachable 
                        //printf("This should be unreachable code\n");
                        
                        pthread_mutex_lock(&participant_bucket_lock[handler_index]);
                        elem_jump_prev =
                                list_entry((elem_jump_prev->fifo_order).next, struct message_store, fifo_order);
                        pthread_mutex_unlock(&participant_bucket_lock[handler_index]);
                    }

                } else {
                    // the correct element arrived, and there is no jump now !!
                    //updaet elem_jump_prev
                    pthread_mutex_lock(&participant_bucket_lock[handler_index]);
                    elem_jump_prev =
                            list_entry((elem_jump_prev->fifo_order).next, struct message_store, fifo_order);
                    pthread_mutex_unlock(&participant_bucket_lock[handler_index]);

                }

            } else // if there was no jump, update elem_jump_prev as normal
            {
                pthread_mutex_lock(&participant_bucket_lock[handler_index]);
                elem_jump_prev =
                        list_entry((elem_jump_prev->fifo_order).next, struct message_store, fifo_order);
                pthread_mutex_unlock(&participant_bucket_lock[handler_index]);

            }
            pthread_mutex_unlock(&timestamp_lock);
            pthread_mutex_lock(&participant_bucket_lock[handler_index]);
        } while (elem_jump->payload != NULL);

        //printf("one round %d\n",handler_index);

        pthread_mutex_unlock(&participant_bucket_lock[handler_index]);

        sleep(wait_time_before_next_round);
    }

    return (void *) 0;
}

void mcast_join(int member) {
    pthread_mutex_lock(&member_lock);
    mcast_num_members_local++;
    join_initialize();
    pthread_mutex_unlock(&member_lock);
}



