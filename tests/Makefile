
OBJECTS = util.o solver.o dwsolv.o

COMPILE_FLAGS = -fprofile-arcs -ftest-coverage -Wall -Wextra
CFLAGS = $(COMPILE_FLAGS) -std=gnu99
CXXFLAGS = $(COMPILE_FLAGS) -std=c++11
LD_FLAGS = -lgtest -pthread

ALL: solver_reduce util_malloc

solver_reduce: solver_reduce.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) solver_reduce.cpp $(OBJECTS) -o solver_reduce -g $(LD_FLAGS)

util_malloc: util_malloc.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) util_malloc.cpp $(OBJECTS) -o util_malloc -g $(LD_FLAGS)

test: solver_reduce util_malloc
	@./solver_reduce
	@./util_malloc

%.o: ../src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o
	rm -f util_malloc solver_reduce util_malloc.exe solver_reduce.exe
	cd ../src && rm -f $(OBJECTS)
