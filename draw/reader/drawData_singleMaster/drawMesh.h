#include "linkList.h"
#include "common.h"

class drawMesh{
public:
	int scale;
	int originalX;
	int originalY;

	int driver, mode;
	
	drawMesh();
	void writeTriangleCoorsFromTriangleList(triangleNode *triangleList);
	void writeTriangleCoorsFromTriangleArr(triangle *triangleArr, unsigned int triangArrNum);
	void drawGridLines(int xPartNum, int yPartNum);
	void drawTriangles(triangleNode *triangleList, int color);
	void drawTriangleArr(triangle *triangleArr, int triangArrSize,  int color);
	void drawTriangleCoorArr(double *triangleArr, int triangArrSize,  int color);
	~drawMesh();
};
