CC=gcc
CFLAGS=-Wall -Werror

SRCS=shellify.c commands.c utils.c
OBJS=$(SRCS:.c=.o)
TARGET=shellify

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)