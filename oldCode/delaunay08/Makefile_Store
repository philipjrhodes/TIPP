CXX = g++

EXE = delaunay
BUILD_DIR = build

SRC = $(wildcard *.cpp) $(wildcard **/*.cpp)
OBJ = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC))

#CXXFLAGS = -std=c++11 -Wall -Wextra -Wfatal-errors -pedantic -w -Winline -fno-rtti -ggdb -D_GLIBCXX_DEBUG -lsfml-graphics -lsfml-window -lsfml-system
CXXFLAGS = -std=c++11 -Wall 
all: $(EXE)

$(EXE): dir delaunayMain.o delaunay.o triangle.o edge.o point2.o
	$(CXX) $(CXXFLAGS) -o $(EXE) $(OBJ) -lgraph

delaunayMain.o: delaunayMain.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

delaunay.o: delaunay.cpp	
	$(CXX) $(CXXFLAGS) -fopenmp  -o $(BUILD_DIR)/$@ -c $<

triangle.o: triangle.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

edge.o: edge.cpp 
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

point2.o: point2.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

dir:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(EXE) $(BUILD_DIR)
