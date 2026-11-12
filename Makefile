CC = gcc
CFLAGS = -Wall -g
TARGET = newshell
OBJS = main.o builtin.o execute.o path.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c major2.h
	$(CC) $(CFLAGS) -c main.c

builtin.o: builtin.c major2.h
	$(CC) $(CFLAGS) -c builtin.c

execute.o: execute.c major2.h
	$(CC) $(CFLAGS) -c execute.c

path.o: path.c major2.h
	$(CC) $(CFLAGS) -c path.c

clean:
	rm -f $(OBJS) $(TARGET)
