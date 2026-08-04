CC ?= gcc
CFLAGS ?= -O2 -Wall
GTK_CFLAGS := $(shell pkg-config --cflags gtk4)
GTK_LIBS := $(shell pkg-config --libs gtk4)

all: shobika-installer

shobika-installer: main.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) main.c -o shobika-installer $(GTK_LIBS)

clean:
	rm -f shobika-installer

.PHONY: all clean
