all: application
install: installapp
install_sounds: installsounds
clean: cleanapp

# change the following to LUA if you want to compile with Lua support. Don't foget to modify config.h accordingly.
SCRIPTTYPE=JS

PASSWDFILE=`awk '/PASSWD_FILE/ {gsub(/"/, "", $$3); print $$3}' config.h`
SCRIPTFILE=`awk '/SCRIPT_FILE/ {gsub(/"/, "", $$3); print $$3}' config.h`
SOUNDSFOLDER=$(shell awk '/SOUNDS_FOLDER/ {gsub(/"/, "", $$3); print $$3}' config.h)

#configuration
OUTDIR=./bin
APPPATH=/opt/ha
TARGET=root@services
TMPDIR=./tmp
TESTSDIR=tests
TESTSCPP=$(wildcard $(TESTSDIR)/*.cpp)
TESTSOBJ=$(TESTSCPP:.cpp=.o)
EXCEPTIONS=$(wildcard ./main.cpp)
ROOTLEVELSRC=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
MODULESSRC=$(wildcard ./modules/*.cpp ./modules/**/*.cpp)
INTERFACESRC=$(wildcard ./WebInterface/*.cpp)
COMMONSRC=$(wildcard ./common/*.cpp)
SOURCESCPP:=$(ROOTLEVELSRC) $(COMMONSRC) $(MODULESSRC) $(INTERFACESRC)
#TODO: should resolve those automatically
DEPENDENCIES=/lib64/librt.so.1  /lib64/libpthread.so.0  /lib64/libdl.so.2  /usr/local/lib/libdum-1.10.so  /usr/local/lib/libresip-1.10.so  /usr/local/lib/librutil-1.10.so  /usr/local/lib/libortp.so.8  /lib64/libdb-4.8.so  /usr/lib64/libasound.so.2  /lib64/libssl.so.1  /lib64/libcrypto.so.1  /usr/lib64/libmysqlclient.so.18  /usr/lib64/libstdc++.so.6  /lib64/libm.so.6  /usr/lib64/libgcc_s.so.1  /lib64/libc.so.6  /lib64/ld-linux-x86-64.so.2  /usr/local/lib/libresipares-1.10.so  /lib64/libz.so.1
OBJECTS=$(SOURCESCPP:.cpp=.o)
OBJECTS:=$(addprefix $(OUTDIR)/,$(notdir $(OBJECTS)))

INCLUDES=$(dir $(wildcard ./modules/*/)) ./WebInterface ./common $(dir $(wildcard ./libs/*/)) ./
INCLUDES:=$(realpath $(sort $(INCLUDES)))
VPATH:=$(INCLUDES)
INCLUDES:=$(patsubst %,-I%,$(INCLUDES)) -I/usr/include/mysql

CFLAGS=-g -rdynamic $(INCLUDES) -fpermissive -std=c++11 -DSYSLOG -DSCRIPT$(SCRIPTTYPE)
LDFLAGS=-lrt -lpthread -ldl -ldum -lresip -lrutil -lortp -ldb -lasound -lssl -lcrypto -lmysqlclient
#LDFLAGS=-static -lrt -lpthread -ldl -ldum -lresip -lrutil -lortp -ldb -lasound -lssl -lcrypto -lmysqlclient
ifeq ($(SCRIPTTYPE),LUA)
LDFLAGS+= -llua
endif
STATICLIBS=$(wildcard ./bin/*.a)
SRCDIR=$(shell pwd)

.PHONY: dumaislib
.PHONY: couchdb
.PHONY: tst

tst:
	echo $(subst /,\/,$(SOUNDSFOLDER))

chkconfig:
	@if [ ! -f "$(SRCDIR)/config.h" ]; then echo -e "\r\nconfig.h was not found in $(SRCDIR). Please make a copy of the file from the example folder and customize it to your needs"; exit -1; fi


cleantests:
	-rm $(TESTSDIR)/*.o
	-rm $(TESTSDIR)/*.test

cleanapp: cleantests
	-rm $(TMPDIR)
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

application: chkconfig outdir libraries $(OUTDIR)/main.o $(OBJECTS)
	g++ -g -rdynamic $(OUTDIR)/main.o $(OBJECTS) $(STATICLIBS) $(LDFLAGS) -o $(OUTDIR)/homeautomation
	# We copy the dependencies in the libs folder so that we can execute on a system with an older libc
	mkdir -p $(OUTDIR)/libs
	cp $(DEPENDENCIES) $(OUTDIR)/libs && chmod 755 $(OUTDIR)/libs/*
	$(OUTDIR)/homeautomation -g > ../documentation/api.json

killha:
	-ssh $(TARGET) "killall homeautomation -w"

installapp: application
	mkdir -p $(TMPDIR)
	cp config.h $(TMPDIR)
	cp views.json $(TMPDIR)
	cp dhaslog $(TMPDIR)
	chmod 555 $(TMPDIR)/dhaslog
	sed "s/SOUNDSFOLDERPLACEHOLDER/$(subst /,\/,$(SOUNDSFOLDER))/g" haconvertsounds > $(TMPDIR)/haconvertsounds
	@if [ ! -f "$(SCRIPTFILE)" ]; then echo -e "\r\n**NOTICE** Event script $(SCRIPTFILE) does not exist. Please create it (see example folder) and run 'homeautomation -r'"; fi
	@if [ ! -f "$(PASSWDFILE)" ]; then echo "username:password" > $(PASSWDFILE); echo -e "\r\n**NOTICE** Default username/password created in $(PASSWDFILE). Please update them and reload homeautomation"; fi
	cp $(OUTDIR)/homeautomation $(TMPDIR)

upgrade: killha installapp
	rsync $(TMPDIR)/* $(TARGET):$(APPPATH)
	ssh $(TARGET) "$(APPPATH)/homeautomation -d < /dev/null >/dev/null"

	
