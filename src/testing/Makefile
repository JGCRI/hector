## This Makefile is meant to be invoked recursively from the top level directory

SRCS	= $(wildcard *.cpp)
OBJS	= $(SRCS:.cpp=.o)
DEPS	= $(SRCS:.cpp=.d)


-include $(DEPS)

LDFLAGS += -L$(GTLIB) -Wl,-rpath=$(GTLIB)

hector-unit-tests: $(OBJS)
	$(CXX) $(LDFLAGS) -o hector-unit-tests *.o ../build/*.o ../logger.o -lgtest -lpthread -lm

clean:
	-rm *.o
	-rm hector-unit-tests
