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
./bin/chat-client : ./obj/chat-client.o
	cc ./obj/chat-client.o -o ./bin/chat-client -lncurses

# =======================================================
#                     Dependencies
# =======================================================
./obj/chat-client.o : ./src/chat-client.c
	cc -c ./src/chat-client.c -o ./obj/chat-client.o 

# =======================================================
# Other targets
# =======================================================
all : ./bin/chat-client

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
