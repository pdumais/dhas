all: libs
OUTDIR=../bin
MODULES=$(sort $(patsubst ./%/,%,$(dir $(wildcard ./*/*.cpp))))
MODULES+=$(sort $(patsubst ./%/,%,$(dir $(wildcard ./*/*.c))))
LIBS=$(addprefix $(OUTDIR)/,$(MODULES))
LIBS:=$(addsuffix .a,$(LIBS))

clean-%:
	-cd $* && make clean

clean: $(addprefix clean-,$(MODULES))

$(OUTDIR)/%.a:%
	cd $< && make
	mv $</$<.a $@

libs: $(LIBS)
	echo $(LIBS)
