#include "mp1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    multicast_init();
    
    while (1) {
        char str[256];
        int len;
        
        if (fgets(str, sizeof(str), stdin) == NULL) {
            break;
        }
        len = strlen(str);
        /* trim newline */
        if (str[len-1] == '\n') {
            str[len-1] = 0;
        }
        if (strncmp(str, "/quit", 5) == 0) {
            break;
        }
        
        multicast(str);
    }
    
    return 0;
}

void deliver(int source, const char *message) {
    printf("<%d> %s\n", source, message);
}