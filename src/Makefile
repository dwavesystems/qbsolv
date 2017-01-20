src = $(wildcard *.c)
obj = $(src:.c=.o)
#CC = gcc -O3 -Ofast #Don't do this, the -O3 is redundant here -Ofast replaces it

# this should have -std=c99 at the very least, that was 18 years ago
CC = gcc -Ofast -Wall


qbsolv: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(obj) qbsolv
