
OBJECTS = ../src/util.o ../src/solver.o ../src/dwsolv.o

LD_FLAGS = -lgtest -pthread

ALL: solver_reduce util_malloc

solver_reduce: solver_reduce.cpp $(OBJECTS)
	g++ solver_reduce.cpp $(OBJECTS) -o solver_reduce -g $(LD_FLAGS)

util_malloc: util_malloc.cpp $(OBJECTS)
	g++ util_malloc.cpp $(OBJECTS) -o util_malloc -g $(LD_FLAGS)

.PHONY: $(OBJECTS)

$(OBJECTS):
	cd ../src && make $(notdir $@)
