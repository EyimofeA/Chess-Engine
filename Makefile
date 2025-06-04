CXX=g++
CXXFLAGS=-std=c++17 -O2
SOURCES=src/cli_main.cpp src/board.cpp src/eval.cpp src/moveGenerator.cpp src/search.cpp src/utils.cpp

engine: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o engine

clean:
	rm -f engine
