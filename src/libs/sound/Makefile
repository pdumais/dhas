all: application
clean: cleanapp

SOURCESCPP=$(wildcard ./*.cpp)
OBJECTS=$(SOURCESCPP:.cpp=.o)
CFLAGS=-std=c++11 -g -rdynamic -I ./ -I../../common/ -I../dumaislib

cleanapp:
	rm *.o
.cpp.o:
	$(CXX) -c $(CFLAGS) $< -o $@

application: $(OBJECTS) 
	ar rcs sound.a $(OBJECTS)

