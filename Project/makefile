.RECIPEPREFIX +=
CC = g++
CFLAGS = -Wall -lm -lrt -lwiringPi -lpthread

PROG = bin/*
OBJS = obj/*

default:
    mkdir -p bin obj
    $(CC) $(CFLAGS) -c src/project.cpp -o obj/project
    $(CC) $(CFLAGS) obj/project -o bin/project
    
run:
    sudo hwclock -w
    timedatect1
    sudo ./bin/project

clean:
    rm $(PROG) $(OBJS)
