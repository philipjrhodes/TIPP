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

	void writeToBinaryFile(string fileNameStr, unsigned int *intArr, unsigned int intArrSize);
	void writeToBinaryFile(string fileNameStr, float *floatArr, unsigned int floatArrSize);
	void writeToBinaryFile(string fileNameStr, double *doubleArr, unsigned int doubleArrSize);
public:
	convertFile(){}
	/*convert a vertex text file into binary file: foo2.ver --> foo2bin.ver*/
	void convertVertexFile(string vertexTextFileName, int attributeRecordSize, int *recordSize, unsigned long long int *recordCount);

	void readBinaryFile(string binaryFileName, int bufferSize);

	void printDoubleFile(string dataFileNameStr, string infoFileNameStr);
	void printIntFile(string fileNameStr, int recordSize, unsigned int recordCount);
};

