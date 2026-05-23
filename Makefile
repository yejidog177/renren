CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
TARGET = talent_show
OBJS = main.o user.o datamanager.o menu.o

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

main.o: main.cpp menu.h
	$(CXX) $(CXXFLAGS) -c main.cpp

user.o: user.cpp user.h
	$(CXX) $(CXXFLAGS) -c user.cpp

datamanager.o: datamanager.cpp datamanager.h user.h
	$(CXX) $(CXXFLAGS) -c datamanager.cpp

menu.o: menu.cpp menu.h datamanager.h user.h
	$(CXX) $(CXXFLAGS) -c menu.cpp

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
