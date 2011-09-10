package ex1;

import ex1.Cell.status;
// a class representing a thread
public class LifeThreads extends Thread{
	Cell[][] board;
	int xBegin,yBegin,xEnd,yEnd,numOfGens;
	int mWidth, mHeight, numOfFinished;
	boolean[][] initialField;
	LifeThreads right,left,top,bottom,topRight,topLeft,botRight,botLeft;
	boolean ThisThreadChange, NeighborThreadChange;

	////   Constuctor for a thread. initializes parameters of a thread such as 
	///    height, width, number of generations to run location on main board and pointer to main board
	public LifeThreads(boolean[][] orig_board, int width, int height, int xBeg, int yBeg, int gen) {
		super();
		board = new Cell[height][width];
		initialField=orig_board;
		numOfFinished = 0;
		xBegin=xBeg;
		yBegin=yBeg;
		mWidth = width;
		mHeight = height;
		xEnd=xBeg+width-1;
		yEnd=yBeg+height-1;
		numOfGens=gen;
		// initializing the thread's board
		for (int i=xBegin; i <= xEnd; i++){
			for (int j=yBegin; j <= yEnd; j++){
				board[j-yBegin][i-xBegin]= new Cell(orig_board[j][i]);
			}
		}	
	}
	//// this method sets a thread's NeighborThreadChange flag to true and wakes it up
	public void setNeighborThreadChange(){
		synchronized (this){
			NeighborThreadChange=true;
			notifyAll();
		}
	}
	/// this method is used for a thread to reset its own NeighborThreadChange flag
	/// so it can start a new run
	public void resetNeighborThreadChange(){
		synchronized (this){
			NeighborThreadChange=false;
		}
	}
	/// this method is used by a thread to check if its flag is true, if it isn't it waits
	public void getNeighborThreadChange(){
		synchronized (this){
			while (NeighborThreadChange==false){
				try {
					wait();
				} catch (InterruptedException e) {}
			}
		}
	}
	
///// this method is used to initialize a threads pointers to neighbors 
 	public void setNeighbors(LifeThreads n1,LifeThreads n2,LifeThreads n3,LifeThreads n4,
			LifeThreads n5,LifeThreads n6,LifeThreads n7,LifeThreads n8){
		topLeft=n1;
		top=n2;
		topRight=n3;
		right=n4;
		botRight=n5;
		bottom=n6;
		botLeft=n7;
		left=n8;
	}
	
	//// a getter for certain cell's generation. recieves coordinates in global form
	public int getCellGen(int x,int y) {
		return board[toLocalCordY(y)][toLocalCordX(x)].getGen();
	}
	//// a getter for certain cell's even value. recieves coordinates in global form	
	public boolean getCellEven(int x,int y) {
		return !(board[toLocalCordY(y)][toLocalCordX(x)].getEven()==Cell.status.DEAD ||
				board[toLocalCordY(y)][toLocalCordX(x)].getEven()==Cell.status.DEAD_FOR_GOOD);
	}
	//// a getter for certain cell's odd value. recieves coordinates in global form
	public boolean getCellOdd(int x,int y) {
		return !(board[toLocalCordY(y)][toLocalCordX(x)].getOdd()==Cell.status.DEAD ||
				board[toLocalCordY(y)][toLocalCordX(x)].getOdd()==Cell.status.DEAD_FOR_GOOD);
	}
	/// changes global x cordinates to local ones
	private int toLocalCordX(int x){
		return x-xBegin;
	}
	/// changes global y cordinates to local ones
	private int toLocalCordY(int y){
		return y-yBegin;
	}
/////////////////////////////////////////
/// this method checks the status of a cell's neighbor. returns 1 for alive and 0 for dead
///	if the requested generation isn't set yet, it returns 10.
	private int checkNeighbor(LifeThreads thread,int my_x,int my_y,int neighbor_x,int neighbor_y) {
		boolean neighborStatus;
		if (thread!=null) {
			if (thread.getCellGen(neighbor_x, neighbor_y)>=this.getCellGen(my_x, my_y)) {
				if ((this.getCellGen(my_x, my_y) % 2)==0) {
					neighborStatus=thread.getCellEven(neighbor_x, neighbor_y);
				}
				else {
					neighborStatus=thread.getCellOdd(neighbor_x, neighbor_y);
				}
				if (neighborStatus) {
					return 1;
				}
				else return 0;
			}
			else return 10; // greater than 8
		}
		return 0;
	}
	////	this method recieves cordinates and number of live neighbors and sets the next value for
	////	the given cell. if cell reaches last generation, it promotes the thread's numOfFinished cells
	private void updateCell(int x,int y, int numOfLive) {
		status current;
		if ((this.getCellGen(x, y) % 2)==0) {
			current=board[toLocalCordY(y)][toLocalCordX(x)].getEven();
			if ((current==status.DEAD && numOfLive==3) ||
					((current==status.ALIVE || current==status.SICK)&& (numOfLive==3 || numOfLive==2))) {
				board[toLocalCordY(y)][toLocalCordX(x)].setOdd(status.ALIVE);
			}
			else if (current==status.ALIVE && numOfLive==4) {
				board[toLocalCordY(y)][toLocalCordX(x)].setOdd(status.SICK);
			}
			else if (current==status.SICK && numOfLive==4) {
				board[toLocalCordY(y)][toLocalCordX(x)].setOdd(status.DEAD_FOR_GOOD);
			}
			else if (current==status.DEAD_FOR_GOOD) {
				board[toLocalCordY(y)][toLocalCordX(x)].setOdd(status.DEAD_FOR_GOOD);
			}
			else board[toLocalCordY(y)][toLocalCordX(x)].setOdd(status.DEAD);
		}
		else {
			current=board[toLocalCordY(y)][toLocalCordX(x)].getOdd();
			if ((current==status.DEAD && numOfLive==3) ||
					((current==status.ALIVE || current==status.SICK) && (numOfLive==3 || numOfLive==2))) {
				board[toLocalCordY(y)][toLocalCordX(x)].setEven(status.ALIVE);
			}
			else if (current==status.ALIVE && numOfLive==4) {
				board[toLocalCordY(y)][toLocalCordX(x)].setEven(status.SICK);
			}
			else if (current==status.SICK && numOfLive==4) {
				board[toLocalCordY(y)][toLocalCordX(x)].setEven(status.DEAD_FOR_GOOD);
			}
			else if (current==status.DEAD_FOR_GOOD) {
				board[toLocalCordY(y)][toLocalCordX(x)].setEven(status.DEAD_FOR_GOOD);
			}
			else board[toLocalCordY(y)][toLocalCordX(x)].setEven(status.DEAD);			
		}
		board[toLocalCordY(y)][toLocalCordX(x)].promoteGen();
		if (board[toLocalCordY(y)][toLocalCordX(x)].getGen()>=numOfGens){
			numOfFinished++;
		}
	}
	
	
	///////////////////////////
	////	this method checks all 8 of the cell's neighbors and returns the number of living neighbors
	////	if there is a cell that is not ready yet, returns a value greater than 8
	private int calcNumOfLive(LifeThreads topLeft, LifeThreads top, LifeThreads topRight, LifeThreads right,
		LifeThreads botRight, LifeThreads bottom, LifeThreads botLeft, LifeThreads left, int x, int y) {
			int numOfLive=0;
			numOfLive+=checkNeighbor(topLeft,x,y,x-1,y-1);
			numOfLive+=checkNeighbor(top,x,y,x,y-1);
			numOfLive+=checkNeighbor(topRight,x,y,x+1,y-1);
			numOfLive+=checkNeighbor(right,x,y,x+1,y);
			numOfLive+=checkNeighbor(botRight,x,y,x+1,y+1);
			numOfLive+=checkNeighbor(bottom,x,y,x,y+1);
			numOfLive+=checkNeighbor(botLeft,x,y,x-1,y+1);
			numOfLive+=checkNeighbor(left,x,y,x-1,y);			
			return numOfLive;
			
		}
	
/////////////////////////////////////////
	////	this method takes care of the option of a thread that has only one cell to calculate
	private void width1Height1Itr(){
		int numOfLive=0;
		while (numOfFinished<(mWidth*mHeight)) {
			if (this.getCellGen(xBegin, yBegin)<numOfGens) {
				resetNeighborThreadChange();
				ThisThreadChange=false;
				numOfLive=calcNumOfLive(topLeft,top,topRight,right,botRight,bottom,botLeft,left,xBegin,yBegin);
				if (numOfLive<=8) {
					updateCell(xBegin,yBegin,numOfLive);
					ThisThreadChange=true;
					neighbor(topLeft);
					neighbor(top);
					neighbor(topRight);
					neighbor(right);
					neighbor(botRight);
					neighbor(bottom);
					neighbor(botLeft);
					neighbor(left);
				}
			}
			if (ThisThreadChange==false) {
				getNeighborThreadChange();
			}
			numOfLive=0;
		}
	};
	////	this method takes care of a thread that is just one column
	private void width1Itr(){
		int numOfLive=0;
		while (numOfFinished<(mWidth*mHeight)) {
			resetNeighborThreadChange();
			ThisThreadChange=false;
			// top cell
			if (this.getCellGen(xBegin, yBegin)<numOfGens) {
				numOfLive=calcNumOfLive(topLeft,top,topRight,right,right,this,left,left,xBegin,yBegin);
				if (numOfLive<=8) {
					updateCell(xBegin, yBegin, numOfLive);
					ThisThreadChange=true;
					neighbor(left);
					neighbor(topLeft);
					neighbor(top);
					neighbor(topRight);
					neighbor(right);
				}
			}
			for (int j=yBegin+1; j< yEnd; j++) {
				if (this.getCellGen(xBegin, j)<numOfGens) {
					numOfLive = calcNumOfLive(left,this,right,right,right,this,left,left,xBegin,j);;
					if (numOfLive<=8) {
						updateCell(xBegin, j, numOfLive);
						ThisThreadChange=true;
						neighbor(right);
						neighbor(left);
					}
				}
			}
			if (this.getCellGen(xEnd, yEnd)<numOfGens) {
				numOfLive=calcNumOfLive(left,this,right,right,botRight,bottom,botLeft,left,xBegin,yEnd);
				if (numOfLive<=8) {
					updateCell(xEnd, yEnd, numOfLive);
					ThisThreadChange=true;
					neighbor(left);
					neighbor(right);			
					neighbor(botRight);
					neighbor(bottom);
					neighbor(botLeft);
				}
			}
			if (ThisThreadChange==false) {
				getNeighborThreadChange();
			}
			numOfLive=0;
		}
	};
/////////////////////////////////////////
	////	this method takes care of a thread that has just one row
	private void height1Itr(){
		int numOfLive=0;
		while (numOfFinished<(mWidth*mHeight)) {
			resetNeighborThreadChange();
			ThisThreadChange=false;
			// top cell
			if (this.getCellGen(xBegin, yBegin)<numOfGens) {
				numOfLive = calcNumOfLive(topLeft,top,top,this,bottom,bottom,botLeft,left,xBegin,yBegin);
				if (numOfLive<=8) {
					updateCell(xBegin, yBegin, numOfLive);
					ThisThreadChange=true;
					neighbor(topLeft);
					neighbor(top);
					neighbor(bottom);
					neighbor(botLeft);
					neighbor(left);
				}
			}
			for (int i=xBegin+1; i< xEnd; i++) {
				if (this.getCellGen(i, yBegin)<numOfGens) {
					numOfLive = calcNumOfLive(top,top,top,this,bottom,bottom,bottom,this,i,yBegin);
					if (numOfLive<=8) {
						updateCell(i, yBegin, numOfLive);
						ThisThreadChange=true;
						neighbor(top);
						neighbor(bottom);
					}
				}
			}
			if (this.getCellGen(xEnd, yEnd)<numOfGens) {
				numOfLive=calcNumOfLive(top,top,topRight,right,botRight,bottom,bottom,this,xEnd,yEnd);
				if (numOfLive<=8) {
					updateCell(xEnd, yEnd, numOfLive);
					ThisThreadChange=true;
					neighbor(top);
					neighbor(topRight);			
					neighbor(right);
					neighbor(botRight);
					neighbor(bottom);
				}
			}
			if (ThisThreadChange==false) {
				getNeighborThreadChange();
			}
			numOfLive=0;
		}
	};


/////////////////////////////////////////
	////	this method takes care of a thread in a normal case
	private void standardItr(){
		int numOfLive=0;
		while (numOfFinished<(mWidth*mHeight)) {
			resetNeighborThreadChange();
			ThisThreadChange=false;
			// update top left cell
			if (this.getCellGen(xBegin, yBegin)<numOfGens) {
				numOfLive = calcNumOfLive(topLeft,top,top,this,this,this,left,left,xBegin,yBegin);
				if (numOfLive<=8) {
					updateCell(xBegin, yBegin, numOfLive);
					ThisThreadChange=true;
					neighbor(topLeft);
					neighbor(top);
					neighbor(left);
				}
			}
			// update top row
			for (int i=xBegin+1; i< xEnd; i++) {
				if (this.getCellGen(i, yBegin)<numOfGens) {
					numOfLive = calcNumOfLive(top,top,top,this,this,this,this,this,i,yBegin);
					if (numOfLive<=8) {
						updateCell(i, yBegin, numOfLive);
						ThisThreadChange=true;
						neighbor(top);
					}
				}
			}
			// update top right cell
			if (this.getCellGen(xEnd, yBegin)<numOfGens) {
				numOfLive = calcNumOfLive(top,top,topRight,right,right,this,this,this,xEnd,yBegin);
				if (numOfLive<=8) {
					updateCell(xEnd, yBegin, numOfLive);
					ThisThreadChange=true;
					neighbor(top);
					neighbor(topRight);
					neighbor(right);
				}
			}
			// update right column
			for (int j=yBegin+1; j< yEnd; j++) {
				if (this.getCellGen(xEnd, j)<numOfGens) {
					numOfLive = calcNumOfLive(this,this,right,right,right,this,this,this,xEnd,j);
					if (numOfLive<=8) {
						updateCell(xEnd, j, numOfLive);
						ThisThreadChange=true;
						neighbor(right);
					}
				}
			}
			// update bottom right cell
			if (this.getCellGen(xEnd, yEnd)<numOfGens) {
				numOfLive = calcNumOfLive(this,this,right,right,botRight,bottom,bottom,this,xEnd,yEnd);
				if (numOfLive<=8) {
					updateCell(xEnd, yEnd, numOfLive);
					ThisThreadChange=true;
					neighbor(right);
					neighbor(botRight);
					neighbor(bottom);
				}
			}
			// update bottom row
			for (int i=xEnd-1; i> xBegin; i--) {
				if (this.getCellGen(i, yEnd)<numOfGens) {
					numOfLive = calcNumOfLive(this,this,this,this,bottom,bottom,bottom,this,i,yEnd);
					if (numOfLive<=8) {
						updateCell(i, yEnd, numOfLive);
						ThisThreadChange=true;
						neighbor(bottom);
					}
				}
			}
			// update bottom left cell
				if (this.getCellGen(xBegin, yEnd)<numOfGens) {
					numOfLive = calcNumOfLive(left,this,this,this,bottom,bottom,botLeft,left,xBegin,yEnd);
					if (numOfLive<=8) {
						updateCell(xBegin, yEnd, numOfLive);
						ThisThreadChange=true;
						neighbor(left);
						neighbor(botLeft);
						neighbor(bottom);
					}
				}
			// update left column
			for (int j=yEnd-1; j> yBegin; j--) {
				if (this.getCellGen(xBegin, j)<numOfGens) {
					numOfLive = calcNumOfLive(left,this,this,this,this,this,left,left,xBegin,j);
					if (numOfLive<=8) {
						updateCell(xBegin, j, numOfLive);
						ThisThreadChange=true;
						neighbor(left);
					}
				}
			}	
			// update the rest (the inner cells)
			for (int i=xBegin+1; i< xEnd; i++) {
				for (int j=yBegin+1; j< yEnd; j++) {
					if (this.getCellGen(i, j)<numOfGens) {
						numOfLive=calcNumOfLive(this,this,this,this,this,this,this,this,i,j);
						if (numOfLive<=8) {
							updateCell(i, j, numOfLive);
							ThisThreadChange=true;
						}
					}
				}
			}
			// check if thread has changed its board in the last run
			if (ThisThreadChange==false) {
				// if not, check if its neighbors changed it
				getNeighborThreadChange();
			}
		}
	}
	
////////////////////////////////////////////////
	/// if thread isn't null, sets the NeighborThreadChange flag of the given thread
	private void neighbor(LifeThreads thread) {
		if (thread != null) {
			thread.setNeighborThreadChange();
		}
	}

////////////////////////////////////////////////
	/// copies the updated board back to the original one
	public void copyBackToBoard() {
		if ((numOfGens % 2) ==0) {
			for (int i=xBegin; i <= xEnd; i++){
				for (int j=yBegin; j <= yEnd; j++){
					initialField[j][i]=((board[j-yBegin][i-xBegin].getEven()==Cell.status.ALIVE)
							|| (board[j-yBegin][i-xBegin].getEven()==Cell.status.SICK));
				}
			} 
		}
		else {
			for (int i=xBegin; i <= xEnd; i++){
				for (int j=yBegin; j <= yEnd; j++){
					initialField[j][i]=((board[j-yBegin][i-xBegin].getOdd()==Cell.status.ALIVE)
							|| (board[j-yBegin][i-xBegin].getOdd()==Cell.status.SICK));
				}
			}
		}
	}
////////////////////////////////////////////////
	////	thread running method which divides the run between different extreme cases
	public void run() {
		if ((mWidth == 1) && (mHeight == 1)) {
			width1Height1Itr();
		}
		else if ((mWidth == 1) && (mHeight != 1)) {
			width1Itr();
		}
		else if ((mWidth != 1) && (mHeight == 1)) {
			height1Itr();
		}
		else {
			standardItr();
		}
		copyBackToBoard();
	}
}
	

