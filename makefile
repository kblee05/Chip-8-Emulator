CC = gcc
CFLAGS = -Wall -Wextra -std=c11 $(shell sdl2-config --cflags)
LIBS = $(shell sdl2-config --libs)

TARGET = chip8
SRCS = main.c chip8.c sdl.c emulator.c 
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)