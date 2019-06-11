#Compiler and Flags
CC = gcc
CFLAGS = -std=c89 -g3 -pedantic -Wall -O2 -lpthread

#Sources
SRCS = *.c

#Headers
HEADERS = 
ifdef *.h
	*.h
endif

#Objects
OBJS = *.o

#Documentation
DOCS = *.pdf

#Programs
PROG = prog

#Compressed File
TAR = cs.tar.bz2

#####################################################
# BUILD and TAR
# ###################################################

brad: br ad

br: mcdadem.buildrooms.c
	${CC} ${CFLAGS} mcdadem.buildrooms.c -o mcdadem.buildrooms

ad: mcdadem.adventure.c
	${CC} ${CFLAGS} mcdadem.adventure.c -o mcdadem.adventure

###################
#CLEAN
###################

clean:
	rm -f mcdadem.adventure mcdadem.buildrooms *.o *~
	rm -rf mcdadem.rooms.*
	rm -f currentTime.txt
