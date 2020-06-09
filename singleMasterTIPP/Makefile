MPICXX = mpic++
CXX = g++
GCC = gcc
TIPP_EXE = ./bin/TIPP
SERIAL_TIPP_EXE = ./bin/SERIAL_TIPP
NON_UNIFORM_DISTRIBUTE_EXE = ./bin/nonUniformDistribute
UNIFORM_DISTRIBUTE_EXE = ./bin/uniformDistribute
DRAW_EXE = ./bin/drawDomain
RBOX_EXE = ./bin/rbox

BUILD_DIR = ./build
SRC_TIPP_DIR = ./src/TIPP
SRC_DISTRIBUTION_DIR = ./src/pointDistribution
SRC_QHULLLIB_DIR = ./src/libqhull/
SRC_DRAW_DIR = ./src/draw
BIN_DIR = ./bin

SRC = $(wildcard *.cpp) $(wildcard **/*.cpp)
#OBJ = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC))
OBJ = $($(BUILD_DIR)/%.o)
#CFLAGS = -c -std=c99 -lm -Wall -Wextra -pedantic -O3 -Wfloat-equal -g
#CFLAGS = -c -O3 -ansi -fpic -m32
CFLAGS = -c -O3
#CXXFLAGS = -std=gnu++11 -Wall -pg
CXXFLAGS = -std=gnu++11 -O3 -w
all: $(RBOX_EXE) $(TIPP_EXE) $(SERIAL_TIPP_EXE) $(NON_UNIFORM_DISTRIBUTE_EXE) $(UNIFORM_DISTRIBUTE_EXE) $(DRAW_EXE)
#all: $(RBOX_EXE)

#$(EXE): $(BUILD_DIR)/*.o
#	$(CXX) $(CXXFLAGS) -o $@ $^




$(DRAW_EXE): $(BUILD_DIR)/drawDomain.o $(BUILD_DIR)/drawMesh.o $(BUILD_DIR)/linkList.o $(BUILD_DIR)/boundingBox.o $(BUILD_DIR)/io.o $(BUILD_DIR)/common.o $(BUILD_DIR)/triangle.o $(BUILD_DIR)/gridBound.o $(BUILD_DIR)/gridElement.o $(BUILD_DIR)/linkList.o $(BUILD_DIR)/point.o $(BUILD_DIR)/edge.o $(BUILD_DIR)/triangle.o
	$(CXX) $(CXXFLAGS) -o $@ $^  -lgraph

#use -I to point out the folder containing the including files
$(BUILD_DIR)/drawDomain.o: $(SRC_DRAW_DIR)/drawDomain.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)  -I $(SRC_TIPP_DIR) -lgraph

$(BUILD_DIR)/drawMesh.o: $(SRC_DRAW_DIR)/drawMesh.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)  -I $(SRC_TIPP_DIR) -lgraph




$(UNIFORM_DISTRIBUTE_EXE): $(BUILD_DIR)/uniformDistributeMain.o $(BUILD_DIR)/uniformDistribute.o $(BUILD_DIR)/boundingBox.o $(BUILD_DIR)/io.o $(BUILD_DIR)/common.o $(BUILD_DIR)/point.o $(BUILD_DIR)/gridBound.o $(BUILD_DIR)/gridElement.o $(BUILD_DIR)/linkList.o $(BUILD_DIR)/edge.o $(BUILD_DIR)/triangle.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/uniformDistributeMain.o: $(SRC_DISTRIBUTION_DIR)/uniformDistributeMain.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)  -I $(SRC_TIPP_DIR) $(SRC_QHULLLIB_DIR)

$(BUILD_DIR)/uniformDistribute.o: $(SRC_DISTRIBUTION_DIR)/uniformDistribute.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)  -I $(SRC_TIPP_DIR)  $(SRC_QHULLLIB_DIR)



$(NON_UNIFORM_DISTRIBUTE_EXE): $(BUILD_DIR)/nonUniformDistributeMain.o $(BUILD_DIR)/nonUniformDistribute.o $(BUILD_DIR)/boundingBox.o $(BUILD_DIR)/io.o $(BUILD_DIR)/common.o $(BUILD_DIR)/point.o $(BUILD_DIR)/gridBound.o $(BUILD_DIR)/gridElement.o $(BUILD_DIR)/linkList.o $(BUILD_DIR)/edge.o $(BUILD_DIR)/triangle.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/nonUniformDistributeMain.o: $(SRC_DISTRIBUTION_DIR)/nonUniformDistributeMain.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) -I $(SRC_TIPP_DIR)

$(BUILD_DIR)/nonUniformDistribute.o: $(SRC_DISTRIBUTION_DIR)/nonUniformDistribute.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) -I $(SRC_TIPP_DIR)




$(SERIAL_TIPP_EXE): $(BUILD_DIR)/delaunaySerialMain.o $(BUILD_DIR)/delaunaySerial.o $(BUILD_DIR)/triangulate.o $(BUILD_DIR)/domain.o $(BUILD_DIR)/partition.o $(BUILD_DIR)/triangle.o $(BUILD_DIR)/boundingBox.o $(BUILD_DIR)/gridBound.o $(BUILD_DIR)/gridElement.o $(BUILD_DIR)/linkList.o $(BUILD_DIR)/edge.o $(BUILD_DIR)/io.o $(BUILD_DIR)/common.o $(BUILD_DIR)/point.o
	$(MPICXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/delaunaySerialMain.o: $(SRC_TIPP_DIR)/delaunaySerialMain.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)



$(TIPP_EXE): $(BUILD_DIR)/delaunay2DMain.o $(BUILD_DIR)/delaunayMPI.o $(BUILD_DIR)/delaunayMPI_ProducerConsumer.o $(BUILD_DIR)/delaunaySerial.o $(BUILD_DIR)/triangulate.o $(BUILD_DIR)/domain.o $(BUILD_DIR)/partition.o $(BUILD_DIR)/triangle.o $(BUILD_DIR)/boundingBox.o $(BUILD_DIR)/gridBound.o $(BUILD_DIR)/gridElement.o $(BUILD_DIR)/linkList.o $(BUILD_DIR)/edge.o $(BUILD_DIR)/io.o $(BUILD_DIR)/common.o $(BUILD_DIR)/point.o
	$(MPICXX) $(CXXFLAGS) -o $@ $^


$(BUILD_DIR)/delaunay2DMain.o: $(SRC_TIPP_DIR)/delaunay2DMain.cpp
	$(MPICXX) -c -o $@ $< $(CXXFLAGS)

$(BUILD_DIR)/delaunayMPI_ProducerConsumer.o: $(SRC_TIPP_DIR)/delaunayMPI_ProducerConsumer.cpp
	$(MPICXX) -c -o $@ $< $(CXXFLAGS)

$(BUILD_DIR)/delaunayMPI.o: $(SRC_TIPP_DIR)/delaunayMPI.cpp
	$(MPICXX) -c -o $@ $< $(CXXFLAGS)

$(BUILD_DIR)/delaunaySerial.o: $(SRC_TIPP_DIR)/delaunaySerial.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(BUILD_DIR)/domain.o: $(SRC_TIPP_DIR)/domain.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/triangulate.o: $(SRC_TIPP_DIR)/triangulate.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/partition.o: $(SRC_TIPP_DIR)/partition.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/triangle.o: $(SRC_TIPP_DIR)/triangle.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/boundingBox.o: $(SRC_TIPP_DIR)/boundingBox.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/gridBound.o: $(SRC_TIPP_DIR)/gridBound.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/gridElement.o: $(SRC_TIPP_DIR)/gridElement.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/edge.o: $(SRC_TIPP_DIR)/edge.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/linkList.o: $(SRC_TIPP_DIR)/linkList.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/io.o: $(SRC_TIPP_DIR)/io.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/common.o: $(SRC_TIPP_DIR)/common.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BUILD_DIR)/point.o: $(SRC_TIPP_DIR)/point.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<





$(RBOX_EXE): $(BUILD_DIR)/rbox.o $(BUILD_DIR)/libqhull.o $(BUILD_DIR)/geom.o $(BUILD_DIR)/geom2.o $(BUILD_DIR)/global.o $(BUILD_DIR)/io_qhull.o $(BUILD_DIR)/mem.o $(BUILD_DIR)/merge.o $(BUILD_DIR)/poly.o $(BUILD_DIR)/poly2.o $(BUILD_DIR)/qset.o $(BUILD_DIR)/random.o $(BUILD_DIR)/rboxlib.o $(BUILD_DIR)/stat.o $(BUILD_DIR)/user.o $(BUILD_DIR)/usermem.o $(BUILD_DIR)/userprintf.o $(BUILD_DIR)/userprintf_rbox.o
	$(GCC) -O3 -o $@ $^ -lm


#-lm for math library
$(BUILD_DIR)/rbox.o: $(SRC_QHULLLIB_DIR)/rbox.c
	$(GCC) $(CFLAGS) -o $@ $< -I $(SRC_QHULLLIB_DIR)

$(BUILD_DIR)/rboxlib.o: $(SRC_QHULLLIB_DIR)/rboxlib.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/userprintf.o: $(SRC_QHULLLIB_DIR)/userprintf.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/userprintf_rbox.o: $(SRC_QHULLLIB_DIR)/userprintf_rbox.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/usermem.o: $(SRC_QHULLLIB_DIR)/usermem.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/qset.o: $(SRC_QHULLLIB_DIR)/qset.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/random.o: $(SRC_QHULLLIB_DIR)/random.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/libqhull.o: $(SRC_QHULLLIB_DIR)/libqhull.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/mem.o: $(SRC_QHULLLIB_DIR)/mem.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/merge.o: $(SRC_QHULLLIB_DIR)/merge.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/user.o: $(SRC_QHULLLIB_DIR)/user.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/geom.o: $(SRC_QHULLLIB_DIR)/geom.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/geom2.o: $(SRC_QHULLLIB_DIR)/geom2.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/global.o: $(SRC_QHULLLIB_DIR)/global.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/io_qhull.o: $(SRC_QHULLLIB_DIR)/io.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/poly.o: $(SRC_QHULLLIB_DIR)/poly.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/poly2.o: $(SRC_QHULLLIB_DIR)/poly2.c
	$(GCC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/stat.o: $(SRC_QHULLLIB_DIR)/stat.c
	$(GCC) $(CFLAGS) -o $@ $<





dir:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BIN_DIR)/* $(BUILD_DIR)/*o

