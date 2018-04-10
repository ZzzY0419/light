CPPFLAGS = -std=c++11
CC = g++ $(CPPFLAGS)
AR = ar
SRC := $(wildcard *.cpp)
OBJS := $(SRC:%.cpp=%.o)
LIB = libsimple_log.a
LDFLAGS = -L./ \
		  -lpthread
INCLUDE = -I./
CFLAGS = $(INCLUDE) -g
TEST_EXEC = test/test_simple_log

all : output

test : $(LIB) $(TEST_EXEC) output

clean :
	rm -rf ./output/*
	rm -rf $(LIB)
	rm -rf *.o
	rm -rf test/simple.log
	rm -rf ${TEST}

output : $(LIB)
	rm -rf *.o *~
	@if [ -d output ]; then rm -rf output; fi
	mkdir output
	mkdir output/include
	mkdir output/lib
	cp $(LIB) ./output/lib
	cp *.h ./output/include

$(LIB) : $(OBJS)
	$(AR) rc $@ $^

$(OBJS) : $(SRC)
	$(CC) -c -o $@ $^

$(TEST_EXEC) : test/test_simple_log.cpp
	$(CC) $(CFLAGS) -o $@ $^ -lsimple_log $(LDFLAGS)
