## This Makefile is meant to be invoked recursively from the top level directory

## This (SRCS) will include testing_main.cpp
SRCS	= $(wildcard *.cpp)

## ** TEMPORARY **
## the following files aren't working yet; remove them
UNDONE = test_core.cpp test_csv_file_reader.cpp test_dependency_finder.cpp
SRCS := $(filter-out $(UNDONE), $(SRCS))

OBJS	= $(SRCS:.cpp=.o)
LDFLAGS += -L$(GTLIB) -Wl,-L../
INCLUDES += -I$(GTINC)

## ----------------------------------------------------
## Default target
hector-unit-tests: $(OBJS) ../libhector.a
	$(CXX) $(LDFLAGS) -o hector-unit-tests $(OBJS) -lhector -lgtest -lpthread -lm -lboost_system -lboost_filesystem

.PHONY: clean chkvar
	
clean:
	-rm *.o
	-rm hector-unit-tests

chkvar:
	@echo "------- Testing"
	@echo "CXX:"
	@echo $(CXX)
	@echo "OBJS:"
	@echo $(OBJS)
	@echo "LDFLAGS:"
	@echo $(LDFLAGS)
	@echo "INCLUDES:"
	@echo $(INCLUDES)
	@echo "GTROOT:"
	@echo $(GTROOT)
