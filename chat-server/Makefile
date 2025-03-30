#
#   FILE          : Makefile
#   PROJECT       : chat-system - A4
#   PROGRAMMER    : Valentyn, Juan Jose, Warren, Ahmed
#   FIRST VERSION : 03/30/2025
#   DESCRIPTION   :
#      This is the Makefile which puts all the source codes together
#		and compile it into the executable file
#

# FINAL BINARY Target
./bin/chat-server : ./obj/chat-server.o
	cc ./obj/chat-server.o -o ./bin/chat-server

# =======================================================
#                     Dependencies
# =======================================================
./obj/chat-server.o : ./src/chat-server.c ./inc/chat-server.h
	cc -c ./src/chat-server.c -o ./obj/chat-server.o 

# =======================================================
# Other targets
# =======================================================
all : ./bin/chat-server

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o