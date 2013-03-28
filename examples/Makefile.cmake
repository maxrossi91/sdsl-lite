CC=g++
#CC_FLAGS=-Wall -g -O3 -I@CMAKE_INSTALL_PREFIX@/include -L@CMAKE_INSTALL_PREFIX@/lib -DNDEBUG -funroll-loops -msse4.2
CC_FLAGS=-Wall -g -O0 -I@CMAKE_INSTALL_PREFIX@/include -L@CMAKE_INSTALL_PREFIX@/lib 
CCLIB=-lsdsl -ldivsufsort -ldivsufsort64 
SOURCES=$(wildcard *.cpp)
EXECS=$(SOURCES:.cpp=.x)

all: $(EXECS)
	        
%.x:%.cpp
	$(CC) $(CC_FLAGS) -o $@ $< $(CCLIB) 

clean:
	rm -f $(EXECS)
	rm -rf *.dSYM
