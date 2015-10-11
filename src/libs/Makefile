all: libs
OUTDIR=../bin
MODULES=$(sort $(patsubst ./%/,%,$(dir $(wildcard ./*/Makefile))))

clean-%:
	-cd $* && make clean

clean: $(addprefix clean-,$(MODULES))

libs: 
	cd duktape && make
	cd sound && make
	cd insteon && make
	cd dumaislib && make tls
	cp duktape/duktape.a $(OUTDIR)/
	cp sound/sound.a $(OUTDIR)/
	cp insteon/insteon.a $(OUTDIR)/
	cp dumaislib/sdk/lib/rest.a $(OUTDIR)/
	cp dumaislib/sdk/lib/json.a $(OUTDIR)/
	cp dumaislib/sdk/lib/webserver.a $(OUTDIR)/
	cp dumaislib/sdk/lib/utils.a $(OUTDIR)/
