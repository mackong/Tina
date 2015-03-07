# a simple cgi program
# add two number,and return the result
CFLAGS = -g -Wall
CC = gcc
TARGET_DIR=../cgi-bin

SOURCES = adder.c

OBJS = $(SOURCES:.c=.o)

adder:$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)
	mv $@ $(TARGET_DIR)
	
clean:
	rm -f $(OBJS) $(TARGET_DIR)/adder
