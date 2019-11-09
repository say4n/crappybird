.PHONY: all

all: clean main

main: main.cpp
	mkdir -p build
	$(CXX) -std=c++11 -lncurses -o build/crappybird main.cpp

clean:
	rm -f *.log
	rm -rf build
