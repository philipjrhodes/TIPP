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
	 gridElement(int xInput, int yInput);
	 gridElement(const gridElement &gridElementInput);

	 void setX(int xInput);
	 void setY(int yInput);
	 int getX();
	 int getY();
};

