#include "linkList.h"
#include "common.h"

class drawMesh{
public:
	int scale;
	int originalX;
	int originalY;

	int driver, mode;
	
	drawMesh();
	void oldDrawGridLines(int xPartNum, int yPartNum);
	void drawGridLines(point lowPoint, point highPoint, int xPartNum, int yPartNum, int color);
	void drawTriangles(triangleNode *triangleList, int color);
	void drawTriangleArr(triangle *triangleArr, int triangArrSize,  int color);
	void drawCoordinateTriangles(double *triangleCoorArr, int triangleCoorArrSize,  int color);
	void drawBox(point lowPoint, point highPoint, int color);
	boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);
	~drawMesh();
};
