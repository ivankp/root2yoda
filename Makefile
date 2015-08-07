CFLAGS := -std=c++11 -Wall -O3

ROOT_CFLAGS := $(shell root-config --cflags)
ROOT_LIBS   := $(shell root-config --libs)

YODA_CFLAGS := $(shell yoda-config --cppflags)
YODA_LIBS   := $(shell yoda-config --libs)

.PHONY: all install clean

EXE := root2yoda

all: $(EXE)

$(EXE): %: %.cc
	@echo CXX $@
	@$(CXX) $(CFLAGS) $(ROOT_CFLAGS) $(YODA_CFLAGS) $^ -o $@ $(ROOT_LIBS) $(YODA_LIBS)

install:
	cp $(EXE) $(prefix)/bin/

clean:
	@rm -fv $(EXE)

