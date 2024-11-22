SOURCE = csapp.c echo.c echoclient.c echoserveri.c
OBJECT_CLIENT = csapp.o echoclient.o
OBJECT_SERVER = csapp.o echo.o echoserveri.o

all:
		gcc -c $(SOURCE)
		gcc -o client $(OBJECT_CLIENT) -lpthread
		gcc -o server $(OBJECT_SERVER) -lpthread