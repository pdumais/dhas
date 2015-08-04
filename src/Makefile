all: application
install: installapp
install_sounds: installsounds
clean: cleanapp

#configuration
OUTDIR=./bin
APPPATH=/opt/ha
TARGETFOLDER=$(APPPATH)
DATAFOLDER=../data
TESTSDIR=tests
TESTSCPP=$(wildcard $(TESTSDIR)/*.cpp)
TESTSOBJ=$(TESTSCPP:.cpp=.o)
EXCEPTIONS=$(wildcard ./main.cpp)
ROOTLEVELSRC=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
MODULESSRC=$(wildcard ./modules/*.cpp ./modules/**/*.cpp)
INTERFACESRC=$(wildcard ./WebInterface/*.cpp)
COMMONSRC=$(wildcard ./common/*.cpp)
SOURCESCPP:=$(ROOTLEVELSRC) $(COMMONSRC) $(MODULESSRC) $(INTERFACESRC)

OBJECTS=$(SOURCESCPP:.cpp=.o)
OBJECTS:=$(addprefix $(OUTDIR)/,$(notdir $(OBJECTS)))

INCLUDES=$(dir $(wildcard ./modules/*/)) ./WebInterface ./common $(dir $(wildcard ./libs/*/)) ./
INCLUDES:=$(realpath $(sort $(INCLUDES)))
VPATH:=$(INCLUDES)
INCLUDES:=$(patsubst %,-I%,$(INCLUDES)) -I$(DATAFOLDER)

CFLAGS=-g -rdynamic $(INCLUDES) -fpermissive -std=c++11 -DSYSLOG
LDFLAGS=-lrt -lpthread -ldl -ldum -lresip -lrutil -lortp -ldb -llua -lasound -ludev
STATICLIBS=$(wildcard ./bin/*.a)

.PHONY: dumaislib
.PHONY: couchdb
.PHONY: tst

tst:
	echo $(ROOTLEVELSRC)

chkdata:
	@if [ ! -d "$(DATAFOLDER)" ]; then echo -e "\r\nThe folder $(DATAFOLDER) does not exist. Please look at the example folder"; exit -1; fi

cleantests:
	-rm $(TESTSDIR)/*.o
	-rm $(TESTSDIR)/*.test

cleanapp: cleantests
	-rm $(OUTDIR)/*
	-rmdir $(OUTDIR)
	-cd libs && make clean


$(OUTDIR)/%.o:%.cpp
	g++ -c $(CFLAGS) $< -o $@


$(TESTSDIR)/%.o:$(TESTSDIR)/%.cpp 
	g++ -c $(CFLAGS) $(LDFLAGS) $< -o $@
	g++ $@ $(OBJECTS) $(STATICLIBS) $(LDFLAGS) -o $(subst .o,.test,$@)

tests: cleantests outdir libraries $(OBJECTS) $(TESTSOBJ)

outdir:
	mkdir -p $(OUTDIR)

libraries:
	cd libs && make

application: chkdata outdir libraries $(OUTDIR)/main.o $(OBJECTS)
	g++ -g -rdynamic $(OUTDIR)/main.o $(OBJECTS) $(STATICLIBS) $(LDFLAGS) -o $(OUTDIR)/homeautomation
	$(OUTDIR)/homeautomation -g > ../documentation/api.json

installapp: application
	-killall homeautomation -w
	mkdir -p $(TARGETFOLDER)
	cp $(OUTDIR)/homeautomation $(TARGETFOLDER)
	-cp $(DATAFOLDER)/JSON.lua $(TARGETFOLDER)
	-cp $(DATAFOLDER)/passwd $(TARGETFOLDER)
	$(APPPATH)/homeautomation -d < /dev/null >/dev/null

updatescript: chkdata
	cp $(DATAFOLDER)/script.lua $(TARGETFOLDER)
	-killall -HUP homeautomation

installsounds: chkdata
	mkdir -p $(TARGETFOLDER)/sounds
	./convert.sh $(DATAFOLDER)/sounds /tmp/dhassounds
	mv tmp/dhassounds/* $(TARGETFOLDER)/sounds
	rmdir tmp/dhassounds
	
couchdb:
	./couchdb
