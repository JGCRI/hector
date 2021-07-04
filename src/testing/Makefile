## This Makefile is meant to be invoked recursively from the top level directory

## This (SRCS) will include testing_main.cpp
SRCS	= $(wildcard *.cpp)

## temporary
## the following files aren't working yet
UNDONE = test_core.cpp test_csv_file_reader.cpp test_dependency_finder.cpp test_ini_to_core_reader.cpp
SRCS := $(filter-out $(UNDONE), $(SRCS))
## temporary

CXXSRCS := $(filter-out $(MAINS), $(CXXSRCS))

OBJS	= $(SRCS:.cpp=.o)
DEPS	= $(SRCS:.cpp=.d)

-include $(DEPS)

LDFLAGS += -L$(GTLIB) -Wl,-L$(GTLIB),-L../

## ----------------------------------------------------

hector-unit-tests: $(OBJS) ../libhector.a
	$(CXX) $(LDFLAGS) -o hector-unit-tests $(OBJS) ../*.o -lhector -lgtest -lpthread -lm -lboost_system -lboost_filesystem
	
clean:
	-rm *.o
	-rm hector-unit-tests

chkvar:
	@echo "------- Testing"
	@echo "OBJS:"
	@echo $(OBJS)
