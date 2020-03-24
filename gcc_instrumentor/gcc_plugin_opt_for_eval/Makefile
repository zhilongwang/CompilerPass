CXX = g++
CXXFLAGS = -std=c++11 -Wall -fno-rtti  -Wno-literal-suffix
 
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include -fPIC

SRCS :=  kernel_instrumentor.cc 
OBJS := $(SRCS:.cc=.o)

all: kernel_instrumentor.so
 
kernel_instrumentor.so: ${OBJS}
	$(CXX) $(LDFLAGS) -shared -o $@ $(OBJS)
 
%.o: %.c 
	$(CXX) $(CXXFLAGS) -fPIC -o $@ -c $<
 
clean:
	rm -f kernel_instrumentor.o handler.o

check:
	gcc -fplugin=./kernel_instrumentor.so -o test test.c runtime
 
.PHONY: all clean check
