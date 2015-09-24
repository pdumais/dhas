all: libs
OUTDIR=../bin
MODULES=$(sort $(patsubst ./%/,%,$(dir $(wildcard ./*/Makefile))))

clean-%:
	-cd $* && make clean

clean: $(addprefix clean-,$(MODULES))

.PHONY: $(MODULES)

$(MODULES): 
	cd $@ && make

libs: $(MODULES)
	cp duktape/duktape.a $(OUTDIR)/
	cp sound/sound.a $(OUTDIR)/
	cp insteon/insteon.a $(OUTDIR)/
	cp dumaislib/sdk/lib/rest.a $(OUTDIR)/
	cp dumaislib/sdk/lib/json.a $(OUTDIR)/
	cp dumaislib/sdk/lib/webserver.a $(OUTDIR)/
