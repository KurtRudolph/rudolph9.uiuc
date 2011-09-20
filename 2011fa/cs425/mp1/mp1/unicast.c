#include "mp1.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>

int *mcast_members;
int mcast_num_members;
int my_id;
static int mcast_mem_alloc;
pthread_mutex_t member_lock = PTHREAD_MUTEX_INITIALIZER;

static int sock;

pthread_t receive_thread;


/* Add a potential new member to the list */
void new_member(int member) {
    int i;
    pthread_mutex_lock(&member_lock);
    for (i = 0; i < mcast_num_members; i++) {
        if (member == mcast_members[i])
            break;
    }
    if (i == mcast_num_members) { /* really is a new member */
        fprintf(stderr, "New group member: %d\n", member);
        if (mcast_num_members == mcast_mem_alloc) { /* make sure there's enough space */
            mcast_mem_alloc *= 2;
            mcast_members = realloc(mcast_members, mcast_mem_alloc * sizeof(mcast_members[0]));
            if (mcast_members == NULL) {
                perror("realloc");
                exit(1);
            }
        }
        mcast_members[mcast_num_members++] = member;
    }
    pthread_mutex_unlock(&member_lock);
}

void *receive_thread_main(void *discard) {
    struct sockaddr_in fromaddr;
    socklen_t len;
    int nbytes;
    char buf[1000];
    
    for (;;) {
        int source;
        len = sizeof(fromaddr);
        nbytes = recvfrom(sock,buf,1000,0,(struct sockaddr *)&fromaddr,&len);
        if (nbytes < 0) {
            perror("recvfrom");
            exit(1);
        }
        
        source = ntohs(fromaddr.sin_port);
        
        fprintf(stderr, "Received %d bytes from %d\n", nbytes, source);
        
        if (nbytes == 0) {
            /* A first message from someone */
            new_member(source);
        } else {            
            /* message must be NUL-terminated */
            assert(buf[nbytes-1] == 0);
            receive(source, buf);
        }
    }
}


void unicast_init(void) {
    struct sockaddr_in servaddr;
    socklen_t len;
    int fd;
    char buf[128];
    int i,n;
    
    /* set up UDP listening socket */
    sock = socket(AF_INET,SOCK_DGRAM,0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }
    
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port=htons(0);     /* let the operating system choose a port for us */
    
    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    /* obtain a port number */
    len = sizeof(servaddr);
    if (getsockname(sock, (struct sockaddr *) &servaddr, &len) < 0) {
        perror("getsockname");
        exit(1);
    }
    
    my_id = ntohs(servaddr.sin_port);
    fprintf(stderr, "Our port number: %d\n", my_id);
    
    /* allocate initial member arrary */
    mcast_mem_alloc = 16;
    mcast_members = malloc(sizeof(mcast_members[0]) * mcast_mem_alloc);
    if (mcast_members == NULL) {
        perror("malloc");
        exit(1);
    }
    mcast_num_members = 0;
    
    /* start receiver thread to make sure we obtain announcements from anyone who tries to contact us */
    if (pthread_create(&receive_thread, NULL, &receive_thread_main, NULL) != 0) {
        fprintf(stderr, "Error in pthread_create\n");
        exit(1);
    }
    
    /* add self to group file */
    
    fd = open(GROUP_FILE, O_WRONLY | O_APPEND | O_CREAT, 0600); /* read-write by the user */
    if (fd < 0) {
        perror("open(group file, 'a')");
        exit(1);
    }
    
    if (write(fd, &my_id, sizeof(my_id)) != sizeof(my_id)) {
        perror("write");
        exit(1);
    }
    close(fd);
    
    
    /* now read in the group file */
    fd = open(GROUP_FILE, O_RDONLY);
    if (fd < 0) {
        perror("open(group file, 'r')");
        exit(1);
    }
    
     do {
         int *member;
         n = read(fd, buf, sizeof(buf));
         if (n < 0) {
             break;
         }
         
         for (member = (int *) buf; ((char *) member) - buf < n; member++) {
             new_member(*member);
         }
     } while (n == sizeof(buf));
     
     close(fd);
     
     /* announce ourselves to everyone */
     
     pthread_mutex_lock(&member_lock);
     for (i = 0; i < mcast_num_members; i++) {
         struct sockaddr_in destaddr;
         
         if (mcast_members[i] == my_id) 
             continue;  /* don't announce to myself */
         
         memset(&destaddr, 0, sizeof(destaddr));
         destaddr.sin_family = AF_INET;
         destaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
         destaddr.sin_port = htons(mcast_members[i]);
         
         sendto(sock, buf, 0, 0, (struct sockaddr *) &destaddr, sizeof(destaddr));
     }
     pthread_mutex_unlock(&member_lock);
}

void usend(int dest, const char *message) {
    struct sockaddr_in destaddr;
    
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    destaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    destaddr.sin_port = htons(dest);
    
    /* include the trailing NUL in the sent message */
    sendto(sock, message, strlen(message)+1, 0, (struct sockaddr *) &destaddr, sizeof(destaddr));
}
