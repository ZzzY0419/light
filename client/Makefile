CPPFLAGS = -std=c++11
CC = g++ $(CPPFLAGS)
AR = ar
SRC := $(wildcard *.cpp)
OBJS := $(SRC:%.cpp=%.o)
LIB = libclient.a
LDFLAGS = -L./ \
		  -lpthread
INCLUDE = -I./
CFLAGS = $(INCLUDE) -g
TEST_EXEC = test/test_sync_client test/test_pressure_client

###### single make ########
SYNC_TEST = test/test_sync_client
PRESSURE_TEST = test/test_pressure_client
SYNC_OBJS = sync_client.o
PRESSURE_OBJS = pressure_client.o
SYNC_LIB = libsync.a
PRESSURE_LIB = libpressure.a

stest : $(SYNC_LIB) $(SYNC_TEST)

$(SYNC_LIB) : $(SYNC_OBJS)
	$(AR) rc $@ $^

ptest : $(PRESSURE_LIB) $(PRESSURE_TEST)

$(PRESSURE_LIB) : $(PRESSURE_OBJS)
	$(AR) rc $@ $^

###### all make #########

all : output

test : $(LIB) $(TEST_EXEC) output

clean :
	rm -rf ./output/*
	rm -rf $(LIB)
	rm -rf *.a
	rm -rf *.o
	rm -rf ${TEST_EXEC}

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

.o : %.cpp
	$(CC) -c -o $@ $<

# $(TEST_EXEC) : $(wildcard test/*.cpp)
# 	$(CC) $(CFLAGS) -o $@ $^ -lclient $(LDFLAGS)

$(SYNC_TEST) : test/test_sync_client.cpp
	$(CC) $(CFLAGS) -o $@ $^ -lsync $(LDFLAGS)

$(PRESSURE_TEST) : test/test_pressure_client.cpp
	$(CC) $(CFLAGS) -o $@ $^ -lpressure $(LDFLAGS)