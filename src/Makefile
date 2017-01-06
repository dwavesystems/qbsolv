src = $(wildcard *.c)
obj = $(src:.c=.o)
CC = gcc -O3 -Ofast


qbsolv: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(obj) qbsolv
