CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g -lm -ldl

TARGET = unit_test
OBJS = ../src/log/*.cpp ../src/pool/*.cpp ../src/timerheap/*.cpp \
       ../src/http/*.cpp ../src/server/*.cpp ../src/buffer/*.cpp \
       ../src/config/*.cpp ../test/unit_test.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET) -lpthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)