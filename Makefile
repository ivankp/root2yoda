CFLAGS := -std=c++11 -Wall -g

ROOT_CFLAGS := $(shell root-config --cflags)
ROOT_LIBS   := $(shell root-config --libs)

YODA_CFLAGS := $(shell yoda-config --cppflags)
YODA_LIBS   := $(shell yoda-config --libs)

.PHONY: all clean

EXE := root2yoda

all: $(EXE)

$(EXE): %: %.cc
	@echo CXX $@
	@$(CXX) $(CFLAGS) $(ROOT_CFLAGS) $(YODA_CFLAGS) $^ -o $@ $(ROOT_LIBS) $(YODA_LIBS)

clean:
	@rm -fv $(EXE)

