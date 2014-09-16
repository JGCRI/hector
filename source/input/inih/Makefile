## This Makefile is meant to be invoked recursively from the top level directory

CXXSRCS	= $(wildcard *.cpp)
CSRCS	= $(wildcard *.c)
OBJS	= $(CXXSRCS:.cpp=.o) $(CSRCS:.c=.o)
DEPS	= $(CXXSRCS:.cpp=.d) $(CSRCS:.c=.d)

CFLAGS	= $(CXXFLAGS)

-include $(DEPS)

all: $(OBJS)

clean:
	-rm *.o
