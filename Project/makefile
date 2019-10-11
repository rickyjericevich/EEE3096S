.RECIPEPREFIX +=
CC = g++
CFLAGS = -Wall -lm -lrt -lwiringPi -lpthread

PROG = bin/*
OBJS = obj/*

default:
    mkdir -p bin obj
    $(CC) $(CFLAGS) -c src/project.cpp -o obj/project
    $(CC) $(CFLAGS) -c src/CurrentTime.cpp -o obj/CurrentTime
    $(CC) $(CFLAGS) obj/project obj/CurrentTime -o bin/proj
    
run:
    sudo hwclock -w
    timedatectl
    sudo ./bin/proj

clean:
    rm $(PROG) $(OBJS)
