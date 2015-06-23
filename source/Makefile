CXX	= g++
CXXFLAGS = -g  $(INCLUDES) $(OPTFLAGS) $(CXXEXTRA) $(CXXPROF) $(WFLAGS) -MMD
INCLUDES = -I$(BOOSTROOT) -I$(HDRDIR) -I$(GSLINC)
WFLAGS   = -Wall -Wno-unused-local-typedefs # Turn on warnings, turn off one particularly annoying one that infests Boost libs
OPTFLAGS = -O3
LDFLAGS	 = $(CXXPROF) -L$(GSLLIB) -L. -Wl,-rpath=$(GSLLIB)

export CXXFLAGS OPTFLAGS

## project root
HROOT	= $(CURDIR)/..
HDRDIR	= $(HROOT)/headers
export HROOT

## Boost root
ifeq ($(strip $(BOOSTVERSION)),)
BOOSTVERSION	= 1_52_0
endif
ifeq ($(strip $(BOOSTROOT)),)
BOOSTROOT	= $(HOME)/src/boost_$(BOOSTVERSION)
endif

## Gtest root
GTVERS	= 1.5.0
GTROOT	= $(HROOT)/libs/gtest-$(GTVERS)
GTINC	= $(GTROOT)/include
GTLIB	= $(GTROOT)/lib/.libs

## Location of GSL (default is /usr/local -- can be overridden by environment)
ifeq ($(strip $(GSLROOT)),)
GSLROOT	= /usr/local
endif

## Set the GSL include and library locations.  There is a special case for Evergreen users.
ifeq ($(strip $(GSLROOT)),EVERGREEN)
GSLINC  = $(GSL_INC)
GSLLIB  = $(GSL_LIB)
# remove warning flags on Evergreen too:  old version of g++
CXXFLAGS = -g  $(INCLUDES) $(OPTFLAGS) $(CXXEXTRA) $(CXXPROF) -MMD
else
GSLINC	= $(GSLROOT)/include
GSLLIB	= $(GSLROOT)/lib
endif

export GTLIB GTINC GSLINC GSLLIB


## component subdirectories
DIRS	= core input input/inih models components visitors data

## sources in the top level directory
SRCS	= $(wildcard *.cpp)
OBJS	= $(SRCS:.cpp=.o)
DEPS	= $(SRCS:.cpp=.d)

## default target
hector: libhector.a main.o
	$(CXX) $(LDFLAGS) -o hector main.o -lhector -lgsl -lgslcblas -lm

## alternate version that uses the capabilities needed for driving
## hector from an external source (e.g., an IAM) 
## DO NOT BUILD THIS TARGET UNLESS YOU ARE TESTING HECTOR'S INTERNAL
## PLUMBING.  It uses some hard-coded emissions for testing and likely
## will not do what you want it to under all circumstances.
# hector-ext: libhector.a main-ext.o
# 	$(CXX) $(LDFLAGS) -o hector-ext main-ext.o -lhector -lgsl -lgslcblas -lm

## Targets that do not literally name files
.PHONY: subdirs $(DIRS) clean test gtest

test: testing
	cd testing && ./hector-unit-tests

testing: INCLUDES += -I$(GTINC)
testing: gtest components topdir
	$(MAKE) -C testing hector-unit-tests

lib: libhector.a
libhector.a: subdirs
	ar ru libhector.a build/*.o

subdirs: build $(DIRS)

## When building a directory, include its corresponding subdirectory
## in the headers directory.  Note that the headers base directory is
## included by default, so, e.g., a source file in source/core could
## include a header from input by saying #include<input/file.hh>
$(DIRS): INCLUDES += -I$(HDRDIR)/$@
## input directory needs an additional include
input: INCLUDES += -I$(HDRDIR)/input/inih

$(DIRS):
	$(MAKE) -C $@ all
	ln -sf $(CURDIR)/$@/*.o build/

build:
	mkdir -p build

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

