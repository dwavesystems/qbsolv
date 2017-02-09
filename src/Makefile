src = $(wildcard *.c)
obj = $(src:.c=.o)

CC = gcc -Ofast -Wall -Wextra -std=gnu99

# For local builds or builds where you know the execution environment will
# be identical to the build environment turn this on
# CC := $(CC) -march=native

qbsolv: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(obj) qbsolv
