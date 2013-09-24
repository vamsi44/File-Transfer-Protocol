# This is a sample Makefile which compiles source files named:
# - tcpechotimeserv.c
# - tcpechotimecliv.c
# - time_cli.c
# - echo_cli.c
# and creating executables: "server6", "client", "time_cli"
# and "echo_cli", respectively.
#
# It uses various standard libraries, and the copy of Stevens'
# library "libunp.a" in ~cse433/Stevens/unpv13e_solaris2.10 .
#
# It also picks up the thread-safe version of "readline.c"
# from Stevens' directory "threads" and uses it when building
# the executable "server6".
#
# It is set up, for illustrative purposes, to enable you to use
# the Stevens code in the ~cse433/Stevens/unpv13e_solaris2.10/lib
# subdirectory (where, for example, the file "unp.h" is located)
# without your needing to maintain your own, local copies of that
# code, and without your needing to include such code in the
# submissions of your assignments.
#
# Modify it as needed, and include it with your submission.

CC = gcc 

LIBS = -lm -lpthread\
	/home/vamsi/workspace/npassign2/unpv13e/libunp.a\
	
FLAGS = -g -O2 -D_REENTRANT

CFLAGS = ${FLAGS} -I/home/vamsi/workspace/npassign2/unpv13e/lib

all: client server6



# server6 uses the thread-safe version of readline.c

server6: server6.o myrtt.o
	${CC} ${FLAGS} -o server6 server6.o get_ifi_info_plus.o myrtt.o ${LIBS}
server6.o: server6.c
	${CC} ${CFLAGS} -c server6.c


client: client.o get_ifi_info_plus.o myrtt.o
	${CC} ${FLAGS} -o client client.o get_ifi_info_plus.o myrtt.o ${LIBS}
client.o: client.c
	${CC} ${CFLAGS} -c client.c


# pick up the thread-safe version of readline.c from directory "threads"

#readline.o: /Home/nwprg/unpv13e/threads/readline.c
#	${CC} ${CFLAGS} -c /Home/nwprg/unpv13e/threads/readline.c

get_ifi_info_plus.o: get_ifi_info_plus.c
	${CC} ${CFLAGS} -c get_ifi_info_plus.c
#prifinfo_plus.o: /Home/nwprg/Asgn2_code/prifinfo_plus.c
#	${CC} ${CFLAGS} -c /Home/nwprg/Asgn2_code/prifinfo_plus.c

myrtt.o: myrtt.c
	${CC} ${CFLAGS} -c myrtt.c


clean:
	rm   client client.o server6 server6.o readline.o  get_ifi_info_plus.o


