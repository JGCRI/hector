ifeq ($(strip $(CXX)),)
CXX      = g++
endif
CXXFLAGS = -g $(INCLUDES) $(OPTFLAGS) $(CXXEXTRA) $(CXXPROF) $(WFLAGS) -MMD
CFLAGS   = -g $(INCLUDES) $(OPTFLAGS) -MMD
INCLUDES = -I$(BOOSTROOT) -I$(HDRDIR)
WFLAGS   = -Wall -Wno-unused-local-typedefs -Wno-c++11-extensions # Turn on warnings, turn off one particularly annoying one that infests Boost libs, also turn off warning about c++-11 extensions.
OPTFLAGS = -O3
LDFLAGS	 = $(CXXPROF) -L$(BOOSTLIB) -L. -Wl,-rpath $(BOOSTLIB)

export CXXFLAGS OPTFLAGS

## project root
HROOT	= $(CURDIR)/..
HDRDIR	= $(HROOT)/include
export HROOT

## Boost root
ifeq ($(strip $(BOOSTVERSION)),)
BOOSTVERSION	= 1_63_0
endif
ifeq ($(strip $(BOOSTROOT)),)
BOOSTROOT	= $(HOME)/src/boost_$(BOOSTVERSION)
endif
ifeq ($(strip $(BOOSTLIB)),)
## Default is the location used by the build scripts included with Boost, but
## this can be overridden by setting BOOSTROOT explicitly in the enviornment.
BOOSTLIB = $(BOOSTROOT)/stage/lib
endif

## Gtest root
GTVERS	= 1.5.0
GTROOT	= $(HROOT)/libs/gtest-$(GTVERS)
GTINC	= $(GTROOT)/include
GTLIB	= $(GTROOT)/lib/.libs

export GTLIB GTINC


## sources in the top level directory
CXXSRCS	= $(wildcard *.cpp)
MAINS   = main.cpp main-api.cpp
CXXSRCS := $(filter-out $(MAINS), $(CXXSRCS))
CSRCS   = $(wildcard *.c)
OBJS	= $(CXXSRCS:.cpp=.o) $(CSRCS:.c=.o) 
DEPS	= $(CXXSRCS:.cpp=.d) $(CSRCS:.c=.d)

## default target
hector: libhector.a main.o
	$(CXX) $(LDFLAGS) -o hector main.o -lhector -lm -lboost_system -lboost_filesystem

## alternate version that uses the capabilities needed for driving
## hector from an external source (e.g., an IAM) 
## DO NOT BUILD THIS TARGET UNLESS YOU ARE TESTING HECTOR'S API
## FUNCTIONALITY.  It uses some hard-coded emissions for testing and likely
## will not do what you want it to under all circumstances.
# hector-api: libhector.a main-api.o
# 	$(CXX) $(LDFLAGS) -o hector-api main-api.o -lhector -lgsl -lgslcblas -lm

## Targets that do not literally name files
.PHONY: clean test gtest

test: testing
	cd testing && ./hector-unit-tests

testing: INCLUDES += -I$(GTINC)
testing: gtest components topdir
	$(MAKE) -C testing hector-unit-tests

lib: libhector.a
libhector.a: $(OBJS)
	ar ru libhector.a *.o

## top level directory objects
-include $(DEPS)

gtest:
	if [ ! -f $(GTROOT)/config.status ] ; then cd $(GTROOT) && ./configure; fi
	$(MAKE) -C $(GTROOT)

clean:
	for dir in $(DIRS); do \
	    $(MAKE) -C $$dir clean; \
	done
	$(MAKE) -C testing clean
	-rm hector *.o
	-rm -rf build

chkvar:
	echo $(CXXSRCS)
	echo $(CSRCS)
	echo $(OBJS)
	echo $(INCLUDES)
