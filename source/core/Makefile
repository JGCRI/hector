## This Makefile is meant to be invoked recursively from the top level directory

SRCS	= $(wildcard *.cpp)
OBJS	= $(SRCS:.cpp=.o)
DEPS	= $(SRCS:.cpp=.d)


-include $(DEPS)

all: $(OBJS)

clean:
	-rm *.o