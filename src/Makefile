src = $(wildcard *.c)
obj = $(src:.c=.o)

CC = clang 
#CC = gcc 

CPPFLAGS = -Wall -Ofast -Wextra -std=gnu99
LDFLAGS = -lm

# For local builds or builds where you know the execution environment will
# be identical to the build environment turn this on
# CC := $(CC) -march=native

qbsolv: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(obj) qbsolv
