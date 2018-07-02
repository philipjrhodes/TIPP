/*
 * ConvertFile.h
 *
 *  Created on: Jul 18, 2015
 *      Author: kevin
 */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

typedef struct
{
    int recordSize;
    int bound;
    int dimension;
    char type[15];
} binaryVetices;

typedef struct
{
    int recordSize;
    int bound;
    int NumerOfVertices;
    char type[15];
} binaryCell;

typedef struct
{
    int recordSize;
    int bound;
    int NumerOfAttributes;
    char type[15];
} binaryAttribute;

typedef struct
{
    int recordSize;
    int bound;
    int dimension;
    char typeOfVertexId[15];
    int NumerOfAttributes;
    char typeOfAttributes[15];
} binaryVerticesAttributes;

class convertFile{
private:
	string getFileName(string fileName);

	/*determine the number of atributes of a point*/
	int attributeRecordSize(string dataTextFileName);

	/*determine the dimension (2D or 3D) from a indicesFile*/
	int dimensionIndices(string indicesTextFileName);

	void writeToBinaryFile(string fileNameStr, unsigned int *intArr, unsigned int intArrSize);
	void writeToBinaryFile(string fileNameStr, float *floatArr, unsigned int floatArrSize);
	void writeToBinaryFile(string fileNameStr, double *doubleArr, unsigned int doubleArrSize);
public:
	convertFile(){}
	/*convert a vertex text file into binary file: foo2.ver --> foo2bin.ver*/
	void convertVertexFile(string vertexTextFileName, int *recordSize, unsigned int *recordCount);

	/*convert a triangle text file into binary file: foo2.smp --> foo2bin.tri*/
	void convertIndicesFile(string indicesTextFileName, int *recordSize, unsigned int *recordCount);

	/*convert a attribute file into binary file: foo2.dat --> foo2bin.att*/
	void convertAttributeFile(string attributeFileName, int *recordSize, unsigned int *recordCount);

	/*convert a vertex text file and data file (attribute) into a binary file: foo2.ver + foo2.dat --> foo2ptdat.ver*/
	void convertVertexAttFile(string vertexTextFileName, string attTextFileName, int *recordVetexSize, int *recordAttributeSize, unsigned int *recordCount);

	/*The XML file that contain meta data file - info about the vertex file*/
	void generateVertexXFDL(string xmlFile, int bounds, int recordsize, int dim);

	/*The XML file that contain meta data file - info about the vertex + data file*/
	void generateVertexDataXFDL(string xmlFile, int bounds, int recordsize, int dim, int dataRecordSize);

	void generateIndicesXFDL(string xmlFile, int bounds, int recordsize, int dim);

	void readBinaryFile(string binaryFileName, int bufferSize);

	void printFloatFile(string fileNameStr, int recordSize, unsigned int recordCount);
	void printIntFile(string fileNameStr, int recordSize, unsigned int recordCount);
};

