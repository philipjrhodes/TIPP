//g++ -std=c++11 qHullConvert3DFile.cpp -o qHullConvert3DFile
//./qHullConvert2DFile ./data/2D/10vertices

#include "qHullConvert3DFile.h"

string convertFile::getFileName(string fullName){
	int lastIndex = fullName.find_last_of(".");
	if(lastIndex!=0)
		return fullName.substr(0, lastIndex);
	else
		return fullName;
}

void convertFile::writeToBinaryFile(string fileNameStr, unsigned int *intArr, unsigned int intArrSize){
	FILE *f = fopen(fileNameStr.c_str(), "w");
	fwrite(intArr, sizeof(unsigned int), intArrSize, f);
	fclose(f);
}
void convertFile::writeToBinaryFile(string fileNameStr, float *floatArr, unsigned int floatArrSize){
	FILE *f = fopen(fileNameStr.c_str(), "w");
	fwrite(floatArr, sizeof(float), floatArrSize, f);
	fclose(f);
}
void convertFile::writeToBinaryFile(string fileNameStr, double *doubleArr, unsigned int doubleArrSize){
	FILE *f = fopen(fileNameStr.c_str(), "w");
	fwrite(doubleArr, sizeof(double), doubleArrSize, f);
	fclose(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void convertFile::readBinaryFile(string binaryFileName, int bufferSize){

/*	ifstream binaryFile (binaryFileName.c_str(), ios::out | ios::binary);
	float *data = new float[bufferSize];
	binaryFile.read(reinterpret_cast<char*>(data), bufferSize*sizeof(float));
	binaryFile.close();*/

	FILE *fp = fopen(binaryFileName.c_str(), "r");
	//cout<<binaryFileName.c_str();
	float *data = new float[bufferSize];
	fread(data, sizeof(float), bufferSize, fp);
	fclose(fp);

//	unsigned int *data = new unsigned int[bufferSize];
//	fread(data, sizeof(unsigned int), bufferSize, fp);
//	fclose(fp);
	for(int i=0; i<bufferSize; i++){
//	cout<<i<" "<<;
	cout<<data[i]<<" ";
		//if((i+1)%4==0) cout<<"\n";
		if((i+1)%2==0) cout<<"\n";
		//if((i+1)%3==0) cout<<"\n";
	}
	delete [] data;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//read the content of a file, recordSize is number of element in a row, recordCount is the number of rows
void convertFile::printFloatFile(string fileNameStr, int recordSize, unsigned int recordCount){
	unsigned int size = recordSize * recordCount;
	FILE *f = fopen(fileNameStr.c_str(), "r");
	//if(!f) {fclose(f); return;}
	cout<<"The content of "<<fileNameStr<<", "<<recordSize<<" "<<recordCount<<endl;
	float* dataArr = new float[size];
	fread(dataArr, sizeof(float), size, f);
	fclose(f);

	cout.precision(17);

	for(unsigned int i=0; i<size; i++){
		if(i%recordSize==0) cout<<"\n";
		cout<<dataArr[i]<<" ";
	}
	cout<<endl;
	delete [] dataArr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//read the content of a file, recordSize is number of element in a row, recordCount is the number of rows
void convertFile::printIntFile(string fileNameStr, int recordSize, unsigned int recordCount){
	unsigned int size = recordSize * recordCount;
	FILE *f = fopen(fileNameStr.c_str(), "r");
	//if(!f) {fclose(f); return;}
	cout<<"The content of "<<fileNameStr<<", "<<recordSize<<" "<<recordCount<<endl;
	unsigned int* dataArr = new unsigned int[size];
	fread(dataArr, sizeof(unsigned int), size, f);
	for(int i=0; i<size; i++){
		if(i%recordSize==0) cout<<"\n";
		cout<<dataArr[i]<<" ";
	}
	cout<<endl;
	fclose(f);
	delete [] dataArr;
}

/*Compute the record size of datatextFile (number of attributes of a vertex)*/
int convertFile::attributeRecordSize(string dataTextFileName){
	// Open data text file (attributes)
	ifstream instreamData(dataTextFileName.c_str());
	if((dataTextFileName=="")||(!instreamData)) return 0;
	else{
		/* Identify the number of attributes in dataTextFileName */
		string line;
		int AttrNum=0;
		getline(instreamData, line);/*read the first line of data file*/
		if(line.c_str() == "") AttrNum = 0;
		else for(int i=0; i<line.length(); i++)
				if(line[i]==' ') AttrNum++;
		//int blankCount = count(line, ' ');/*count the number of blank space*/
		return AttrNum+1;
	}
	instreamData.close();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int convertFile::dimensionIndices(string indicesTextFileName){
	// Open indices text file
	ifstream instreamIndices(indicesTextFileName.c_str());
	if((indicesTextFileName=="")||(!instreamIndices)) return 0;
	else{
		/* Identify the dimension */
		string line;
		int blankCount=0;
		getline(instreamIndices, line);/*skip the first line of index file*/
		getline(instreamIndices, line);/*read the second line in order to know the number of different numbers in a line*/
		instreamIndices.close();
		if(line.c_str() == "") blankCount = 0;
		else for(int i=0; i<line.length(); i++)
				if(line[i]==' ') blankCount++;
		int verticesNum = blankCount;
		return verticesNum;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*convert a attribute file into binary file: foo2.dat --> foo2bin.att*/
void convertFile::convertAttributeFile(string attributeFileName, int *recordSize, unsigned int *recordCount){
	ifstream instreamVertex(attributeFileName.c_str());
	std::string strItem;

	if(!instreamVertex)
		cout<<"There is no filename : "<<attributeFileName;
	else
	{
        /* First line of input contains dimensionality */
		instreamVertex >> strItem;
		*recordSize = atoi(strItem.c_str());

		/* Second line contains number of records */
		instreamVertex >> strItem;
		*recordCount = atoi(strItem.c_str());

		unsigned int size = (*recordSize) * (*recordCount);

		float *data = new float[size];

		/*load content from text file to array data*/
		int i = 0;
		while(!instreamVertex.eof()&&(i<size)){
			instreamVertex >> strItem;
			data[i] = atof(strItem.c_str());
			i++;
		}
		//Write data array to a new binary file

		string newBinaryFileNameStr = getFileName(attributeFileName) + "bin.att";
		FILE *fVertices = fopen(newBinaryFileNameStr.c_str(), "w");
		fwrite(data, sizeof(float), size, fVertices);
		fclose(fVertices);

		/*free array data*/
		delete [] data;

		//Create a metadata file: .xfdl
		ofstream xfdlFile;
		string metaDataFile = newBinaryFileNameStr + ".xfdl";
		xfdlFile.open (metaDataFile.c_str(), std::ofstream::out | std::ofstream::trunc);
		xfdlFile<<(*recordSize)<<"\n"<<(*recordCount);
		xfdlFile.close();


/*		//Create a metadata file: .xfdl
		int recordsize = sizeof(float) * (*recordSize);
		binaryAttribute attributeInfo = {recordsize, (*recordCount), (*recordSize), "float"};
		string metaDataFile = newBinaryFileNameStr + ".xfdl";
		FILE *fout = fopen(metaDataFile.c_str(), "w");		
		fwrite(&attributeInfo, sizeof(binaryAttribute), 1, fout);
		fclose(fout);
*/
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void convertFile::convertVertexFile(string vertexTextFileName, int *recordSize, unsigned int *recordCount){
	ifstream instreamVertex(vertexTextFileName.c_str());
	std::string strItem;

	if(!instreamVertex){
		cout<<"There is no filename : "<<vertexTextFileName;
		return;
	}

    /* First line of input contains dimensionality */
	instreamVertex >> strItem;
	*recordSize = atoi(strItem.c_str());

	/* Second line contains number of records */
	instreamVertex >> strItem;
	*recordCount = atoi(strItem.c_str());

	unsigned int size = (*recordSize) * (*recordCount);

	float *data = new float[size];

	/*load content from text file to array data*/
	int i = 0;
	while(!instreamVertex.eof()&&(i<size)){
		instreamVertex >> strItem;
		data[i] = atof(strItem.c_str());
//		if(i%2==0) cout<<endl;
//		cout<<strItem<<" ";
		i++;
	}
	instreamVertex.close();
	
	string newBinaryFileNameStr = getFileName(vertexTextFileName) + "bin.ver";
//	string newBinaryFileNameStr = "./data/2D/24vertices/mydatabin.ver";
	writeToBinaryFile(newBinaryFileNameStr.c_str(), data, size);

	/*free array data*/
	delete [] data;

	//Create a metadata file: .xfdl
	ofstream xfdlFile;
	string metaDataFile = newBinaryFileNameStr + ".xfdl";
	xfdlFile.open (metaDataFile.c_str(), std::ofstream::out | std::ofstream::trunc);
	xfdlFile<<(*recordSize)<<"\n"<<(*recordCount);
	xfdlFile.close();


/*	//Create a metadata file: .xfdl
	int recordsize = sizeof(float) * (*recordSize);
	binaryVetices verticesInfo = {recordsize, (*recordCount), (*recordSize), "float"};
	string metaDataFile = newBinaryFileNameStr + ".xfdl";
	FILE *fout = fopen(metaDataFile.c_str(), "w");	
	fwrite(&verticesInfo, sizeof(binaryVetices), 1, fout);
	fclose(fout);
*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** Convert triangle text file into binary file: mydata.smp --> mydatabin.tet
  */
void convertFile::convertIndicesFile(string indicesTextFileName, int *recordSize, unsigned int *recordCount){
	ifstream instreamIndices;
	instreamIndices.open (indicesTextFileName.c_str(), std::ifstream::in);

	if(!instreamIndices){
		cout<<"There is no filename : "<<indicesTextFileName;
		return;
	}

	string strItem;
	/*determine the dimension*/
	*recordSize = 4;

	/* First line of input contains number of triangles */
	instreamIndices >> strItem;
	*recordCount = atoi(strItem.c_str());

	unsigned int size = (*recordSize)*(*recordCount);

	/*Allocate memory for data*/
	unsigned int *data = new unsigned int[size];

	//load content from text file to array data
	unsigned int index = 0;
	
	for(unsigned int i=0; i<*recordCount*(*recordSize+1); i++){
		if(i%5==0)	instreamIndices >> strItem;
		else{
			instreamIndices >> strItem;
			data[index] = atoi(strItem.c_str());
			index++;
		}
	}

	instreamIndices.close();

	string newBinaryFileNameStr = getFileName(indicesTextFileName) + "bin.tet";
	writeToBinaryFile(newBinaryFileNameStr.c_str(), data, size);

	//free array data
	delete [] data;

	//Create a metadata file: .xfdl
	ofstream xfdlFile;
	string metaDataFile = newBinaryFileNameStr + ".xfdl";
	xfdlFile.open (metaDataFile.c_str(), std::ofstream::out | std::ofstream::trunc);
	xfdlFile<<(*recordSize)<<"\n"<<(*recordCount);
	xfdlFile.close();


/*	//Create a metadata file: .xfdl
	int recordsize = sizeof(int) * (*recordSize);
	binaryCell cellInfo = {recordsize, *recordCount, *recordSize, "unsigned int"};
	string metaDataFile = newBinaryFileNameStr + ".xfdl";
	FILE *fout = fopen(metaDataFile.c_str(), "w");
	fwrite(&cellInfo, sizeof(binaryCell), 1, fout);
	fclose(fout);
*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** convertVertexDataFile converts and combine two files: vertex text file (mydata.ver) and its associated data text file (mydata.dat) into a single binary file
 * Kevin (CN) adds one method convertVertexDataFile 7/20/2015
 * Input: vertexTextFileName is vertex text file which is generated by qhull, attTextFileName is attribute file which describes the properties of each vertex
 * Output: two files: <name>ptdat.ver and <name>ptdat.ver.xfdl
 * Note: must have no empty line in datasource files (vertex, indices, data)
 * */
void convertFile::convertVertexAttFile(string vertexTextFileName, string attTextFileName, int *recordVetexSize, int *recordAttributeSize, unsigned int *recordCount){
	// Open vertex text file
	ifstream instreamVertex(vertexTextFileName.c_str());
	if(!instreamVertex){
		cout<< vertexTextFileName << " is not exist";
		exit(1);
	}

	// Open data text file (attributes)
//	ifstream instreamAttribute(attTextFileName.c_str());
//	if((attTextFileName=="")||(!instreamAttribute)){
		/*There is no attribute file*/
//		convertVertexFile(vertexTextFileName, recordVetexSize, recordCount);
//		instreamVertex.close();
//	}
	else{/*Convert vertex and attribute file into a binary file*/
		std::string strItem;

        /* First line of input contains dimensionality */
		instreamVertex >> strItem;
		int dim = atoi(strItem.c_str());
		*recordVetexSize = dim;

		/* Second line contains number of records */
		instreamVertex >> strItem;
		unsigned int bound = atoi(strItem.c_str());
		*recordCount = bound;

		/* Identify the number of attributes in attTextFileName */
//		int attrNum = attributeRecordSize(attTextFileName);
        int attrNum = 2;
//		instreamAttribute >> strItem;
//		int attrNum = atoi(strItem.c_str());
		*recordAttributeSize = attrNum;

//		instreamAttribute >> strItem;

		/*Allocate enough memory for vetices and their attributes*/
		float *data = new float[(dim + attrNum) * bound];

		/*load content from text file and data file to the array data*/
		int index = 0;
		for(int i=0; i< bound; i++){
			for(int j=0; j<dim; j++){/*load vertices*/
				instreamVertex >> strItem;
				data[index] = atof(strItem.c_str());
				index++;
			}
			for(int k=0; k<attrNum; k++){/*load attributes*/
//				instreamAttribute >> strItem;
//				data[index] = atof(strItem.c_str());
                float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                data[index] = r;
				index++;
			}
		}
		instreamVertex.close();
//		instreamAttribute.close();

		/*Write vertices coordinates + attributes to binary file*/
		string newBinaryFileNameStr = getFileName(vertexTextFileName) + "bin.veratt";
		ofstream newBinaryVertexFile (newBinaryFileNameStr.c_str(), ios::out | ios::binary);
		newBinaryVertexFile.write((char *)data, (dim + attrNum) * bound * sizeof(float));
		newBinaryVertexFile.close();

		/*free array data*/
		delete [] data;

		//Create a metadata file: .xfdl
		ofstream xfdlFile;
		string metaDataFile = newBinaryFileNameStr + ".xfdl";
		xfdlFile.open (metaDataFile.c_str(), std::ofstream::out | std::ofstream::trunc);
		xfdlFile<<(*recordVetexSize)<<" "<<(*recordAttributeSize)<<"\n"<<(*recordCount);
		xfdlFile.close();

/*		//Create a metadata file: .xfdl
		int recordsize = sizeof(float) * (dim + attrNum);
		binaryVerticesAttributes cellAttrInfo = {recordsize, bound, dim, "unsigned int", attrNum, "float"};
		string metaDataFile = newBinaryFileNameStr  + ".xfdl";
		FILE *fout = fopen(metaDataFile.c_str(), "w");
		fwrite(&cellAttrInfo, sizeof(binaryVerticesAttributes), 1, fout);
		fclose(fout);
*/
		}
}


void convertFile::generateVertexXFDL(string xmlFile, int bounds, int recordsize, int dim){
    ofstream xmlOutFile(xmlFile.c_str());
    xmlOutFile << "<?xml version = 1.0 encoding = UTF-8 ?>\n";
    xmlOutFile << "<!DOCTYPE FileDescriptor PUBLIC " << '"' << "-//SDB//DTD//EN" <<'"' << ' '<< '"'<< "fdl.dtd" << '"' << ">\n";
    xmlOutFile << "<FileDescriptor fileName = " << '"' << xmlFile << '"' << " fileType = " << '"' <<"binary" << '"' << " recordSize =" << '"' << recordsize << '"' <<" >\n";
    xmlOutFile << "    <Bounds lower = " << '"' << 0 << '"' << "   upper = " << '"' << bounds << '"' << " >\n";

    for(int i=0; i<dim; i++) {
    	xmlOutFile << "    <Field fieldName = " << '"' << "dim" << i << '"' <<  " fieldType= " <<'"' << "float" << '"' <<">\n";
    }
    xmlOutFile << "</FileDescriptor>\n";
    xmlOutFile.close();
}

void convertFile::generateIndicesXFDL(string xmlFile, int bounds, int recordsize, int dim){
    ofstream xmlOutFile(xmlFile.c_str());
    xmlOutFile << "<?xml version = 1.0 encoding = UTF-8 ?>\n";
    xmlOutFile << "<!DOCTYPE FileDescriptor PUBLIC " << '"' << "-//SDB//DTD//EN" <<'"' << ' '<< '"'<< "fdl.dtd" << '"' << ">\n";
    xmlOutFile << "<FileDescriptor fileName = " << '"' << xmlFile << '"' << " fileType = " << '"' <<"binary" << '"' << " recordSize =" << '"' << recordsize << '"' <<" >\n";
    xmlOutFile << "    <Bounds lower = " << '"' << 0 << '"' << "   upper = " << '"' << bounds << '"' << " >\n";

    for(int i=0; i<dim; i++) {
    	xmlOutFile << "    <Field fieldName = " << '"' << "dim" << i << '"' <<  " fieldType= " <<'"' << "float" << '"' <<">\n";
    }
    xmlOutFile << "</FileDescriptor>\n";
    xmlOutFile.close();

}

void convertFile::generateVertexDataXFDL(string xmlFile, int bounds, int recordsize, int dim, int dataRecordSize){
    ofstream xmlOutFile(xmlFile.c_str());
    xmlOutFile << "<?xml version = 1.0 encoding = UTF-8 ?>\n";
    xmlOutFile << "<!DOCTYPE FileDescriptor PUBLIC " << '"' << "-//SDB//DTD//EN" <<'"' << ' '<< '"'<< "fdl.dtd" << '"' << ">\n";
    xmlOutFile << "<FileDescriptor fileName = " << '"' << xmlFile << '"' << " fileType = " << '"' <<"binary" << '"' << " recordSize =" << '"' << recordsize << '"' <<" >\n";
    xmlOutFile << "    <Bounds lower = " << '"' << 0 << '"' << "   upper = " << '"' << bounds << '"' << " >\n";

    for(int i=0; i<dim; i++) {
    	xmlOutFile << "    <Field fieldName = " << '"' << "dim" << i << '"' <<  " fieldType= " <<'"' << "float" << '"' <<">\n";
    }
    for(int i=0; i<dataRecordSize; i++) {
    	xmlOutFile << "    <Field fieldName = " << '"' << "data" << i << '"' <<  " fieldType= " <<'"' << "float" << '"' <<">\n";
    }

    xmlOutFile << "</FileDescriptor>\n";
    xmlOutFile.close();

}

int main(int argc, char **argv){
	convertFile cf;
	int recordSize;
	unsigned int recordCount;
	string path = "./data/2D/24vertices/";

	if(argc==1){
		cout<<"You need to input the path to store datasource\n";
		cout<<"Convert nothing!!!\n";		
	}
	else if(argc==2){
		cout<<"The path is: "<<argv[1]<<endl;
		path = argv[1];

//	cout<<"-----------------------------------------------------\n";
	//convert coordinate file
//	cf.convertVertexFile(path + "/mydata.ver", &recordSize, &recordCount);
//	cf.printFloatFile("./data/2D/24vertices/mydatabin.ver", recordSize, recordCount);

//	cout<<"-----------------------------------------------------\n";
	//convert vertexId file
	cf.convertIndicesFile(path + "/mydata3D.smp", &recordSize, &recordCount);
//	cf.printIntFile("./data/2D/10vertices/mydatabin.tri", recordSize, recordCount);

//	cout<<"-----------------------------------------------------\n";
	//convert vertexId file
//	cf.convertAttributeFile(path + "/mydata.att", &recordSize, &recordCount);
//	cf.printFloatFile("./data/2D/24vertices/mydatabin.att", recordSize, recordCount);

	int recordVetexSize;
	int recordAttributeSize;
	unsigned int recordCount;
	cf.convertVertexAttFile(path + "/mydata3D.ver", path + "/mydata3D.att", &recordVetexSize, &recordAttributeSize, &recordCount);
//	cf.printFloatFile("./data/2D/10vertices/mydatabin.veratt", recordVetexSize + recordAttributeSize, recordCount);

	cout<<"\ndone!!!!\n";
	}
 return 0;
}

