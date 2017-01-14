
OBJECTS = ../src/util.o ../src/solver.o ../src/dwsolv.o

#solver_reduce: solver_reduce.c $(OBJECTS)
#	gcc solver_reduce.c $(OBJECTS) -o solver_reduce -g

ALL: solver_reduce util_malloc

solver_reduce: solver_reduce.cpp $(OBJECTS)
	g++ solver_reduce.cpp $(OBJECTS) -o solver_reduce -g -lgtest

util_malloc: util_malloc.cpp $(OBJECTS)
	g++ util_malloc.cpp $(OBJECTS) -o util_malloc -g -lgtest

.PHONY: $(OBJECTS)

$(OBJECTS):
	cd ../src && make $(notdir $@)
