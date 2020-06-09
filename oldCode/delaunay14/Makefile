CXX = g++

EXE = domainDelaunay
BUILD_DIR = build

SRC = $(wildcard *.cpp) $(wildcard **/*.cpp)
OBJ = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC))
#OBJ = $($(BUILD_DIR)/%.o)


#CXXFLAGS = -std=c++11 -Wall -Wextra -Wfatal-errors -pedantic -w -Winline -fno-rtti -ggdb -D_GLIBCXX_DEBUG -lsfml-graphics -lsfml-window -lsfml-system
CXXFLAGS = -std=c++11 -Wall -pg 
all: $(EXE)

$(EXE): $(BUILD_DIR)/domainMain.o $(BUILD_DIR)/domain.o $(BUILD_DIR)/partition.o $(BUILD_DIR)/triangle.o $(BUILD_DIR)/boundingBox.o $(BUILD_DIR)/gridBound.o $(BUILD_DIR)/gridElement.o $(BUILD_DIR)/linkList.o $(BUILD_DIR)/edge.o $(BUILD_DIR)/point.o $(BUILD_DIR)/common.o
	$(CXX) $(CXXFLAGS) -o $(EXE)  $< 

domainMain.o: domainMain.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

domain.o: domain.cpp	
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

partition.o: partition.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

triangle.o: triangle.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

boundingBox.o: boundingBox.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

gridBound.o: gridBound.cpp 
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

gridElement.o: gridElement.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

edge.o: edge.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

linkList.o: linkList.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

point.o: point.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

common.o: common.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ -c $<

dir:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(EXE) $(BUILD_DIR)
