#include "linkList.h"
#include "common.h"

class drawMesh{
public:
	int scale;
	int originalX;
	int originalY;

	int driver, mode;
	
	drawMesh();
	void drawGridLines(int xPartNum, int yPartNum);
	void drawTriangles(triangleNode *triangleList, int color);
	void drawTriangleArr(triangle *triangleArr, int triangArrSize,  int color);
	~drawMesh();
};
