all: lib
OBJECTS = duktape.o
SOURCES=$(OBJECTS:.o=.c)

clean:
	-rm *.o
	-rm *.a

.c.o:
	$(CXX) -g -std=c++11 -c $< -o $@

lib: $(OBJECTS)
	$(AR) rcs duktape.a $(OBJECTS) 

