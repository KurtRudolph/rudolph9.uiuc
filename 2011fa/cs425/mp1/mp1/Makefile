all: mp1

SOURCEFILES = unicast.c mcast.c chat.c
HEADERS = mp1.h

mp1: $(SOURCEFILES) $(HEADERS)
	gcc -pthread -o $@ $(SOURCEFILES)

clean:	restart
	-rm mp1 *.o

restart:
	-rm GROUPFILE
