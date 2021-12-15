CFLAGS=-Wall -Wextra -g
LDLIBS=

SRCS=top_proc.c example.c
OBJS=$(subst .c,.o,$(SRCS))

EXAMPLE=top_proc_example

all: $(EXAMPLE)

$(EXAMPLE): $(OBJS)
	$(CC) $(CFLAGS) $(LDLIBS) $(OBJS) -o $(EXAMPLE)

clean:
	rm -f $(OBJS) $(EXAMPLE)
