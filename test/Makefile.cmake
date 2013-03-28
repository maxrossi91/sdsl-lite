CC=g++
CC_FLAGS=-Wall -Werror -Wunused-parameter -g -O3 -I@CMAKE_INSTALL_PREFIX@/include -L@CMAKE_INSTALL_PREFIX@/lib 
CCLIB=-lsdsl -ldivsufsort -ldivsufsort64 -lgtest 
SOURCES=$(wildcard *Test.cpp)
EXECS=$(SOURCES:.cpp=.x)
EXEC_LIST=$(patsubst %,./%;,$(EXECS))                # list of executables
EXEC_LIST_VAL=$(patsubst %,valgrind ./%;,$(EXECS))   # list of executables preceded by valgrind command

all: $(EXECS)
	        
%.x:%.cpp @CMAKE_INSTALL_PREFIX@/lib/libsdsl.a
	$(CC) $(CC_FLAGS) -o $@ $< $(CCLIB) 

clean:
	rm -f $(EXECS)
	rm -rf *.dSYM

# TODO special test case for BitMagicTestSSE

test:
	./test_cases/small/get_corpus.sh
	$(EXEC_LIST)


vtest:
	./test_cases/small/get_corpus.sh
	$(EXEC_LIST_VAL)