all: application
clean: cleanapp

SOURCESCPP = InsteonDevice.cpp SwitchDevice.cpp EZFloraDevice.cpp InsteonModem.cpp IInsteonMessage.cpp StandardOrExtendedMessage.cpp AllLinkDatabaseMessage.cpp IMConfigurationMessage.cpp IOLinc.cpp GroupCleanupMessage.cpp StartAllLinkingMessage.cpp InsteonLogging.cpp
OBJECTS=$(SOURCESCPP:.cpp=.o)
CFLAGS=-g -rdynamic -I ./ -I../dumaislib/ -I../../common/ -std=c++11

cleanapp:
	rm *.o
.cpp.o:
	g++ -c $(CFLAGS) $< -o $@

application: $(OBJECTS) 
	ar rcs insteon.a $(OBJECTS)

