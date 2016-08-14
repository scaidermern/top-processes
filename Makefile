CFLAGS=-Wall -Wextra
LDLIBS=-lprocps

SRCS=top_proc.c example.c
OBJS=$(subst .c,.o,$(SRCS))

all: example

example: $(OBJS)
	$(CC) $(CFLAGS) $(LDLIBS) $(OBJS) -o top_proc_example

clean:
	rm $(OBJS) top_proc_example
