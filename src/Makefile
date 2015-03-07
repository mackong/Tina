# Makefile for Tina

CC = gcc
CFLAGS = -g -Wall
PREFIX=/usr/sbin
LIBS = -lxml2 -lpthread
INCLUDES = -I/usr/include/libxml2

SOURCES = evbuffer.c header.c logger.c mime.c \
		request.c response.c tina.c util.c

OBJS = $(SOURCES:.c=.o)

all: mime_types.h tina

mime_types.h: mime_types.txt
	rm -f mime_types.h
	sort mime_types.txt > sorted_mime_types.txt
	sed < sorted_mime_types.txt > mime_types.h \
		-e 's/#.*//' -e 's/[ ]*$$//' -e '/^$$/d' \
		-e 's/[		][	]*/", "/' -e 's/^/{"/' -e 's/$$/"},/'
	rm sorted_mime_types.txt
	
tina: $(OBJS) server.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

server.o: server.c
	$(CC) -c $^ $(CFLAGS) $(INCLUDES)

install:
	cp tina $(PREFIX)

clean:
	rm -f $(OBJS) server.o tina mime_types.h

