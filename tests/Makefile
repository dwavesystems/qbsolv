
OBJECTS = ../src/util.o ../src/solver.o ../src/dwsolv.o

solver_reduce: solver_reduce.c $(OBJECTS)
	gcc solver_reduce.c $(OBJECTS) -o solver_reduce -g

.PHONY: $(OBJECTS)

$(OBJECTS):
	cd ../src && make $(notdir $@)
