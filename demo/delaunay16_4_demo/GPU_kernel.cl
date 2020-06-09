//////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D point with coordinate x and y
//////////////////////////////////////////////////////////////////////////////////////////////////////
class point
{	//coordinate x and y
private:
	double x;
	double y;
	//order of points in domain
	unsigned long long int Id;
public:
	point(double xInput, double yInput){
		x = xInput;
		y = yInput;
	}
	point(){}

	void setX(double xInput){	x = xInput;}
	void setY(double yInput){	y = yInput;}

	void set(point pointInput){
		x = pointInput.x;
		y = pointInput.y;
		Id = pointInput.Id;
	}

	double getX(){	return x;}
	double getY(){	return y;}

	void setId(unsigned long long int id){Id = id;}
	unsigned long long int getId(){return Id;}

	point operator = (const point &pointtInput){
		x = pointtInput.x;
		y = pointtInput.y;
	}

	bool operator == (point p){
	return (Id == p.Id);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
/* A triangle have 3 points each point store the id of that point in vertices list*/
////////////////////////////////////////////////////////////////////////////////////////////////////
class triangle
{
private:
	point p1;
	point p2;
	point p3;
	double centerX;
	double centerY;
	double radius;

public:
	triangle(point pointInput1, point pointInput2, point pointInput3){
		p1 = pointInput1;
		p2 = pointInput2;
		p3 = pointInput3;
	}
	triangle(){};

	void setPoint1(point pointInput){	point1 = pointInput;}
	void setPoint2(point pointInput){	point2 = pointInput;}
	void setPoint3(point pointInput){	point3 = pointInput;}

	point getPoint1(){	return p1; }
	point getPoint2(){	return p2;}
	point getPoint3(){	return p3;}

	bool operator == (const triangle &t){
		return (p1 == t.p1 || p1 == t.p2 || p1 == t.p3) &&
				(p2 == t.p1 || p2 == t.p2 || p2 == t.p3) &&
				(p3 == t.p1 || p3 == t.p2 || p3 == t.p3);
	}


//determine if a point stay inside the circle of triangle
//http://www.qc.edu.hk/math/Advanced%20Level/circle%20given%203%20points.htm
//http://www.regentsprep.org/regents/math/geometry/gcg6/RCir.htm
//http://math.stackexchange.com/questions/213658/get-the-equation-of-a-circle-when-given-3-points
	void computeCenterRadius(){
		double x1 = p1.getX();
		double y1 = p1.getY();

		double x2 = p2.getX();
		double y2 = p2.getY();

		double x3 = p3.getX();
		double y3 = p3.getY();

		double ab = (x1 * x1) + (y1 * y1);
		double cd = (x2 * x2) + (y2 * y2);
		double ef = (x3 * x3) + (y3 * y3);

		centerX =(double)(ab * (y3 - y2) + cd * (y1 - y3) + ef * (y2 - y1)) / (double)(x1 * (y3 - y2) + x2 * (y1 - y3) + x3 * (y2 - y1)) / double(2.0);
		centerY = (double)(ab * (x3 - x2) + cd * (x1 - x3) + ef * (x2 - x1)) / (double)(y1 * (x3 - x2) + y2 * (x1 - x3) + y3 * (x2 - x1)) / double(2.0);
		radius = sqrt(((x1 - centerX) * (x1 - centerX)) + ((y1 - centerY) * (y1 - centerY)));
	}


	bool circumCircleContains(point p){
		double x = p.getX();
		double y = p.getY();
	
		double dist = sqrt(((x - centerX) * (x - centerX)) + ((y - centerY) * (y - centerY)));
		return dist < radius;
	}

	double triangle::getFarestCoorX(){
		return centerX + radius;
	}

	double triangle::getHighestCoorY(){
		return centerY + radius;
	}


/*	1001	1000	1010
	0001	0000	0010
	0101	0100	0110
*/
	BYTE triangle::outCode(point lowPoint, point highPoint, point p){
		point lowerPoint(lowPoint);
		point upperPoint(highPoint);
		double x = p.getX();
		double y = p.getY();

		BYTE b = (((y > highPoint.getY()?1:0) << 3) |
				((y < lowPoint.getY()?1:0) << 2) |
				((x > highPoint.getX()?1:0) << 1) |
				((x < lowPoint.getX()?1:0)));
		return b;	
	}

	//distance between two points
	double triangle::distance(point p1, point p2){
		return sqrt( (p1.getX()-p2.getX())*(p1.getX()-p2.getX()) + (p1.getY()-p2.getY())*(p1.getY()-p2.getY()) );
	}

	//check intersection between a rectangle (partition) and current triangle
	//refenrence: https://yal.cc/rectangle-circle-intersection-test/
	bool intersect(point lowPoint, point highPoint){
		point center = point(centerX, centerY);

		double d;//distance
		BYTE code = outCode(lowPoint, highPoint, center);
		switch (code){
			//center of circle stays inside rectangle
			case 0x00: return true; break;
			//center of circle stays on top or bottom of rectangle
			case 0x08: d = distance(point(highPoint.getX(), centerY), highPoint);break;
			case 0x04: d = distance(point(lowPoint.getX(), centerY), lowPoint);break;

			//center of circle stays on left or right of rectangle
			case 0x01: d = distance(point(centerX, lowPoint.getY()), lowPoint);break;
			case 0x02: d = distance(point(centerX, highPoint.getY()), highPoint);break;

			//center of circle stays on outside of low coner or outside of right coner of rectangle
			case 0x05: d = distance(point(centerX, centerY), lowPoint);break;
			case 0xA: d = distance(point(centerX, centerY), highPoint);break;

			//center of circle stays on outside of high left coner or outside of right low coner of rectangle
			case 0x06: d = distance(center, point(highPoint.getX(), lowPoint.getY()) );break;
			case 0x09: d = distance(center, point(lowPoint.getX(), highPoint.getY()) );
		}
		if(d<=radius) return true;
		else return false;
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
//logical coordinates of a partition element
//partition a square geology into 2x2 --> (0,0), (0,1) (1,0), (1,1)
//////////////////////////////////////////////////////////////////////////////////////////////////////
class gridElement
{
private:
	int gridx;
	int gridy;
public:
	gridElement(int xInput, int yInput){
		gridx = xInput;
		gridy = yInput;
	}
	gridElement(const gridElement &gridElementInput){
		gridx = gridElementInput.gridx;
		gridy = gridElementInput.gridy;
	}

	gridElement(){}

	 void setX(int xInput){	gridx = xInput;}
	 void setY(int yInput){	gridy = yInput;}
	 int getX(){	return gridx;}
	 int getY(){	return gridy;}

	gridElement operator = (const gridElement &gridElementInput){
		gridx = gridElementInput.gridx;
		gridy = gridElementInput.gridy;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
//This is the definition of a grid box
// A gridbox includes low left corner, and high right corner partiton element
//////////////////////////////////////////////////////////////////////////////////////////////////////
class gridBound
{	
private:
	//low left corner partition
	gridElement lowGridElement;

	//high right corner partition
	gridElement highGridElement;
public:
	gridBound(gridElement lowGridElementInput, gridElement highGridElementInput){
		lowGridElement = lowGridElementInput;
		highGridElement = highGridElementInput;
	}
	void setLowGridElement(gridElement gridElementInput){
		lowGridElement = gridElementInput;
	}
	void setHighGridElement(gridElement gridElementInput){
		highGridElement = gridElementInput;
	}
	gridElement getLowGridElement(){
		return gridElement(lowGridElement.getX(), lowGridElement.getY());
	}
	gridElement getHighGridElement(){
		return gridElement(highGridElement.getX(), highGridElement.getY());
	}

	/* Count the number of intersections of a bounding box (around the cell) to the grid elements in geology bound*/
	int elementCount(){
		/*the coordiante of two corners of bounding grid*/
		int beginx = lowGridElement.getX();
		int beginy = lowGridElement.getY();
		int endx = highGridElement.getX();
		int endy = highGridElement.getY();	
		return (endy - beginy + 1) * (endx - beginx + 1);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////
/*the rectangle on the surface including lowLeft and higHright corners*/
////////////////////////////////////////////////////////////////////////////////////////////
class GBound
{
private:
	//low left corner coordinates
	point lowPoint;

	//high right corner coordinates
	point highPoint;

/*  intersection_helper()

  This private helper function will return true if the line v1-v2 intersects the line b1-b2, and
  false otherwise. The last argument specifies whether b1-b2 is horizontal or vertical, using
  the contants defined below.

    It is important that b1 < b2 for the axis being checked. If not, incorrect results will be
   generated. Also, b1-b2 is meant to be one side of a GBounds, so it is currently assumed to be
   either horizontal or vertical, depending on the last argument. However, v1-v2 can have any
   orientation.
*/
	bool intersection_helper_vertical(point v1, point v2, point b1, point b2){
		double t = (b1.getX() - v1.getX())/(v2.getX() - v1.getX());
		if((t<0) || (t>1)) //no intersection
			return false;
		else {
			double y0 = v1.getY();
			double y1 = v2.getY();
			double y = y0 + t*(y1-y0);
			if((y < b1.getY()) || (y > b2.getY())) return false;
			else return true;
		}
	}

	bool intersection_helper_horizontal(point v1, point v2, point b1, point b2){
		double t = (b1.getY() - v1.getY())/(v2.getY() - v1.getY());
		if((t<0) || (t>1)) //no intersection
			return false;
		else {
			double x0 = v1.getX();
			double x1 = v2.getX();
			double x = x0 + t*(x1-x0);
			if((x < b1.getX()) || (x > b2.getX())) return false;
			else return true;
		}
	}

public:
	GBound(point lowPointInput, point highPointInput){
		lowPoint = lowPointInput;
		highPoint = highPointInput;
	}
	GBound(double lowX, double lowY, double highX, double highY){
		lowPoint.setX(lowX);
		lowPoint.setY(lowY);
		highPoint.setX(highX);
		highPoint.setY(highY);
	}
	 GBound(){}

	 void setLowPoint(point pointInput){
		lowPoint = pointInput;
	}
	 void setHighPoint(point pointInput){
		highPoint = pointInput;
	}
	 point getLowPoint(){	
		return point(lowPoint.getX(), lowPoint.getY());
	}
	 point getHighPoint(){	
		return point(highPoint.getX(), highPoint.getY());
	}


	/*	1001	1000	1010
		0001	0000	0010
		0101	0100	0110
	*/
	BYTE outCode(point p){
		point lowerPoint(lowPoint.getX(), lowPoint.getY());
		point upperPoint(highPoint.getX(), highPoint.getY());
		double x = p.getX();
		double y = p.getY();
	
		BYTE b = (((y > upperPoint.getY()?1:0) << 3) |
				((y < lowerPoint.getY()?1:0) << 2) |
				((x > upperPoint.getX()?1:0) << 1) |
				((x < lowerPoint.getX()?1:0)));
			return b;	
	}

	//This function check if a cell intersect with a GBound rectangle
	bool intersect(triangleCell cell){
		point lowerPoint(lowPoint.getX(), lowPoint.getY());
		point upperPoint(highPoint.getX(), highPoint.getY());

		int numClearlyOutside = 0; //number of segments of a cell are clearly outside the GBound rectangle
		point cellVertices[3] = {cell.getPoint1(), cell.getPoint2(), cell.getPoint3()};

		for(int i=0; i<3; i++){ //scan 3 edges of a cell
			int j=(i+1) % 3; //i, j are two point indices: (0,1),(1,2),(2,0)
			BYTE outCode1, outCode2, outCodeOR; //opCode from two points of a segment
			outCode1 = outCode(cellVertices[i]);
			outCode2 = outCode(cellVertices[j]);

			//if a point falls inside GBound reactangle --> intersect is true
			if((outCode1==0)||(outCode2==0)) return true;

    	    // Test for lines that cross straight through from left to right, or top to bottom 
			outCodeOR = (BYTE) outCode1 | outCode2;
			if((outCode1==0x03) ||(outCode2==0x0C)) return true;

	        // Count the number of edges that are clearly outside the rectangle 
			if(outCode1 & outCode2 !=0) numClearlyOutside++; //no intersection with GBound rectangle
			else // The edge is not clearly outside, so we'll have to do some line intersections.
			{
				//If one vertex has the center left outcode, then we can try intersecting the edge with the left rectangle boundary.
				if((outCode1 == 0x01) || (outCode2 == 0x01)){ //left edge
					point p(lowerPoint.getX(), upperPoint.getY());
					if(intersection_helper_vertical(cellVertices[i], cellVertices[j], lowerPoint, p))
					return true;
				}
				else if((outCode1 == 0x02) || (outCode2 == 0x02)){ //right edge
					point p(upperPoint.getX(), lowerPoint.getY());
					if(intersection_helper_vertical(cellVertices[i], cellVertices[j], p, upperPoint))
					return true;
				}
				else if((outCode1 == 0x04) || (outCode2 == 0x04)){ //bottom edge
					point p(upperPoint.getX(), lowerPoint.getY());
					if(intersection_helper_horizontal(cellVertices[i], cellVertices[j], lowerPoint, p))
					return true;
				}
				else if((outCode1 == 0x08) || (outCode2 == 0x08)){ //top edge
					point p(lowerPoint.getX(), upperPoint.getY());
					if(intersection_helper_horizontal(cellVertices[i], cellVertices[j], p, upperPoint))
					return true;
				}
				else if(outCodeOR == 0x0F){ //diagonals
					// Here, we handle the case where the vertices lie in opposite corner outcodes.
					// First, we'll see if the edge intersects with the left side.

					point p(lowerPoint.getX(), upperPoint.getY());
					if(intersection_helper_vertical(cellVertices[i], cellVertices[j], lowerPoint, p))
					return true;
				}
				// Now, we have to see which of the two kinds of diagonals we have,
				// and either test the top or bottom sides of the rectangle.
				if((outCode1 == 0x09) || (outCode2 == 0x09)){
					// The line must go from upper left to lower right, so we'll test the top side.
					point p(lowerPoint.getX(), upperPoint.getY());
					if(intersection_helper_horizontal(cellVertices[i], cellVertices[j], p, upperPoint))
					return true;

				} else {
					// The line must go from lower left to upper right, so we'll test the bottom side.
					point p(upperPoint.getX(), lowerPoint.getY());
					if(intersection_helper_horizontal(cellVertices[i], cellVertices[j], lowerPoint, p))
					return true;
				}

			}
	}
	if(numClearlyOutside == 3)
		return false;
	else return cell.inside(lowerPoint);
	}

};

////////////////////////////////////////////////////////////////////////////////////////////
//the rectangle that covers the triangle cell (Bounding Box)
////////////////////////////////////////////////////////////////////////////////////////////
GBound boundingBox(triangleCell cell){
	double minX = min(min(cell.getPoint1().getX(), cell.getPoint2().getX()), cell.getPoint3().getX());
	double minY = min(min(cell.getPoint1().getY(), cell.getPoint2().getY()), cell.getPoint3().getY());
//	point lowPoint(minX, minY);

	double maxX = max(max(cell.getPoint1().getX(), cell.getPoint2().getX()), cell.getPoint3().getX());
	double maxY = max(max(cell.getPoint1().getY(), cell.getPoint2().getY()), cell.getPoint3().getY());
//	point highPoint(maxX, maxY);

//  return GBound(lowPoint, highPoint);
  return GBound(minX, minY, maxX, maxY);
}

/* Returns true if this TriangleCell is contained by the GBounds argument, false otherwise.*/
bool containBy(GBound gb, triangleCell cell){
	if((cell.getPoint1().getX() < gb.getLowPoint().getX()) || (cell.getPoint1().getX() > gb.getHighPoint().getX())) return false;
	if((cell.getPoint1().getY() < gb.getLowPoint().getY()) || (cell.getPoint1().getY() > gb.getHighPoint().getY())) return false;

	if((cell.getPoint2().getX() < gb.getLowPoint().getX()) || (cell.getPoint2().getX() > gb.getHighPoint().getX())) return false;
	if((cell.getPoint2().getY() < gb.getLowPoint().getY()) || (cell.getPoint2().getY() > gb.getHighPoint().getY())) return false;

	if((cell.getPoint3().getX() < gb.getLowPoint().getX()) || (cell.getPoint3().getX() > gb.getHighPoint().getX())) return false;
	if((cell.getPoint3().getY() < gb.getLowPoint().getY()) || (cell.getPoint3().getY() > gb.getHighPoint().getY())) return false;

	return true;
}


/* Map a point on the partitioning. If p falls on a partition boundary, we choose the
 partition with the higher index. Ex: 2.5/2 = 2 --> column/row 0, 1, 2; 2.0/2 = 1 */
gridElement mapHigh(point p, GBound geoBound, double gridElementSizeX, double gridElementSizeY){
	int gridx = (p.getX() - geoBound.getLowPoint().getX())/gridElementSizeX;
	int gridy = (p.getY() - geoBound.getLowPoint().getY())/gridElementSizeY;

	int gridMaxX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/gridElementSizeX -1;
	int gridMaxY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/gridElementSizeY -1;

	//check in special case gridx and gridy greater than number of grids
	if(gridx > gridMaxX) gridx = gridMaxX;
	if(gridy > gridMaxY) gridy = gridMaxY;

   	return gridElement(gridx, gridy);
}

/* Map a point on the partitioning. If p falls on a partition boundary, we choose the
 partition with the lower index. 2.5/2 = 2 --> column/row 0, 1, 2; 4.0/2 = 2, need to be subtract by 1 --> 1
 if point p fall on the grid line (partition line) --> take lower element.*/
gridElement mapLow(point p, GBound geoBound, double gridElementSizeX, double gridElementSizeY){
	int gridx, gridy;
		double v = (p.getX() - geoBound.getLowPoint().getX())/gridElementSizeX;
	if((v-(int)v)==0) gridx = (int)v - 1;
	else gridx = (int)v;
		v = (p.getY() - geoBound.getLowPoint().getY())/gridElementSizeY;
	if((v-(int)v)==0) gridy = (int)v - 1;
	else gridy = (int)v;

	//check in special case
	if(gridx<0) gridx=0;
	if(gridy<0) gridy=0;

	gridElement ge(gridx, gridy);
	return ge;
}

//Identify boundingGrid for a localBound (inside globalBound)
gridBound boundingGrid(GBound localBound, GBound globalBound, int xPartitionNum,  int yPartitionNum){
	double gridElementSizeX = (globalBound.getHighPoint().getX() - globalBound.getLowPoint().getX())/yPartitionNum;
    double gridElementSizeY = (globalBound.getHighPoint().getY() - globalBound.getLowPoint().getY())/xPartitionNum;

	point lowPoint = localBound.getLowPoint();
	point highPoint = localBound.getHighPoint();
   	gridElement lowGridElement(mapHigh(lowPoint, globalBound, gridElementSizeX, gridElementSizeY));
    gridElement highGridElement(mapLow(highPoint, globalBound, gridElementSizeX, gridElementSizeY));

   	gridBound gb(lowGridElement, highGridElement);
    return gb;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
* preComputePartition runs on OpenCL to count the number of triangleCells that intersect with each partition
* input:
*   point2DList: the list of point,
*   triangleCellIdList: the list of triangle cells, each cell contain2 3 pointid,
*   pointListBound: the maximum number of points,
*   cellListBound: the maximum number of cells,
*   geoBound: the geology bound of whole dataset,
*   xPartitionNum: number of partitions on x axes,
*   yPartitionNum: number of partition on y axes
* output:
*   partitionElementList: the list of partition element which has number of intersection with cells
*	Partition a geology into 4x4 elements, partitionElementList[0] <--> partition element (0,0),
*	partitionElementList[15] <--> partition element (3x4,3)
*/
__kernel void preComputePartitionGPU(__global double *coordinateList,
              				         __global int *cellPartitionList,
              				          unsigned int cellListBound,
              				          class GBound geoBound,
              				          int xPartitionNum, int yPartitionNum)
{
	//threadID is the index of triangleCellIdList
    int threadId = get_global_id(0);
//printf("%d ", threadId);

	//The index of 3 points and coordinates in a cell
	triangleCell cell;
	point p1(coordinateList[threadId*6], coordinateList[threadId*6+1]);
	point p2(coordinateList[threadId*6+2], coordinateList[threadId*6+3]);
	point p3(coordinateList[threadId*6+4], coordinateList[threadId*6+5]);
	cell.setPoint1(p1);
	cell.setPoint2(p2);
	cell.setPoint3(p3);
	
	//Find a bounding box cover around the triangle
	GBound bBox = boundingBox(cell);
//	printf("%f %f   %f %f\n", bBox.getLowPoint().getX(), bBox.getLowPoint().getY(), bBox.getHighPoint().getX(), bBox.getHighPoint().getY());

	//gridBox that intersects with the bounding box of a triangle
	gridBound gb = boundingGrid(bBox, geoBound, xPartitionNum, yPartitionNum);
//	printf("%d %d   %d %d\n", gb.getLowGridElement().getX(), gb.getLowGridElement().getY(), gb.getHighGridElement().getX(), gb.getHighGridElement().getY());

	//Scan bounding grid to count the number of intersections of a bounding box (around the cell) to the grid elements in geology bound
	//the coordiante of two corners of bounding grid
	int beginx = gb.getLowGridElement().getX();
	int beginy = gb.getLowGridElement().getY();
	int endx = gb.getHighGridElement().getX();
	int endy = gb.getHighGridElement().getY();

	cellPartitionList[threadId] = (endy - beginy + 1) * (endx - beginx + 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
* unstructuredSplitter runs on CUDA to count the number of triangleCells that intersect with each partition
* input: 2DPointList, triangleCellList
* output: partitionEleList
*/
 __kernel void
dataSplitterGPU( __global double *coordinateList,
                 __global int *ownerCellPartitionList,
                 __global int *borrowerCellPartitionList,
                 __global unsigned int *borrowerPartitionIndex,
                 unsigned int cellListBound,
                 class GBound geoBound,
                 int xPartitionNum,
                 int yPartitionNum)
{
	//threadID is the index of triangleCellIdList
    int threadId = get_global_id(0);
//printf("%d ", threadId);
    double gridElementSizeX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/xPartitionNum;
    double gridElementSizeY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/yPartitionNum;

	double originalLowX = geoBound.getLowPoint().getX();
	double originalLowY = geoBound.getLowPoint().getY();

	//The index of 3 points and coordinates in a cell
	triangleCell cell;
	point p1(coordinateList[threadId*6], coordinateList[threadId*6+1]);
	point p2(coordinateList[threadId*6+2], coordinateList[threadId*6+3]);
	point p3(coordinateList[threadId*6+4], coordinateList[threadId*6+5]);
	cell.setPoint1(p1);
	cell.setPoint2(p2);
	cell.setPoint3(p3);
	
	//Find a bounding box cover around the triangle
	GBound bBox = boundingBox(cell);

	//gridBox that intersects with the bounding box of a triangle
	gridBound gb = boundingGrid(bBox, geoBound, xPartitionNum, yPartitionNum);

	//Scan bounding grid to count the number of intersections of a bounding box (around the cell) to the grid elements in geology bound
	//the coordiante of two corners of bounding grid
	int beginx = gb.getLowGridElement().getX();
	int beginy = gb.getLowGridElement().getY();
	int endx = gb.getHighGridElement().getX();
	int endy = gb.getHighGridElement().getY();

	//Scan bounding grid
	for(int i = beginy; i<=endy; i++)
		for(int j = beginx; j<=endx; j++){
			//mapping a partition element into an element in partitionElementList
			int partitionEletIdx = xPartitionNum * i + j;

			//set a grid element
			GBound gridElement(gridElementSizeX*j + originalLowX, gridElementSizeY*i + originalLowY,
									gridElementSizeX*(j+1) + originalLowX, gridElementSizeY*(i+1) + originalLowY);

			if(containBy(gridElement, cell)) //cell fall in gridElement
					//set owner for current cell --> gridElement
					ownerCellPartitionList[threadId] = partitionEletIdx;
			else if(gridElement.intersect(cell)){
					if(ownerCellPartitionList[threadId] == -1)
						ownerCellPartitionList[threadId] = partitionEletIdx;
					else{
						borrowerCellPartitionList[borrowerPartitionIndex[threadId]] = partitionEletIdx;
						borrowerPartitionIndex[threadId]++;
					}
				}
		}

}


