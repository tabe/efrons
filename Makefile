.PHONY: all clean

all: efrons

clean:
	-rm -rf efrons

efrons: efrons.cc
	g++ -std=c++11 -o $@ $^
