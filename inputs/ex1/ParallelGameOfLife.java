package ex1;

public class ParallelGameOfLife implements GameOfLife {
	LifeThreads threads[][];
	int width,height;
	public boolean[][] invoke(boolean[][] initalField, int hSplit, int vSplit,
			int generations) {
		int boardWidth,boardHeight,size_horizontal,size_vertical,hleft,vleft;
		boolean[][] m_field;
		hleft=boardWidth=(initalField[0].length);
		vleft=boardHeight=(initalField.length);
		width=hSplit;
		height=vSplit;
		size_horizontal=boardWidth/hSplit;
		size_vertical=boardHeight/vSplit;
		m_field=initalField.clone();	
		threads= new LifeThreads[vSplit][hSplit];
		// initializes new threads giving them there size and location
		for (int i=0; i<hSplit-1; i++) {
			for (int j=0; j<vSplit-1; j++) {
				threads[j][i]=new LifeThreads(m_field, size_horizontal, size_vertical, i*size_horizontal, j*size_vertical, generations);
				vleft-=size_vertical;
				}
			threads[vSplit-1][i]=new LifeThreads(m_field, size_horizontal, vleft, i*size_horizontal, (vSplit-1)*size_vertical, generations);
			vleft=boardHeight;
			hleft-=size_horizontal;				
			}
		for (int j=0; j<vSplit-1; j++) {
			threads[j][hSplit-1]=new LifeThreads(m_field, hleft, size_vertical, (hSplit-1)*size_horizontal, j*size_vertical, generations);
			vleft-=size_vertical;
			}
		threads[vSplit-1][hSplit-1]=new LifeThreads(m_field, hleft, vleft, (hSplit-1)*size_horizontal, (vSplit-1)*size_vertical, generations);

		// sets the neighbors of each thread
		for (int i=0; i<hSplit; i++) {
			for (int j=0; j<vSplit; j++) {
				threads[j][i].setNeighbors(getThread(i-1,j-1),getThread(i,j-1), getThread(i+1,j-1),
						getThread(i+1,j), getThread(i+1,j+1), getThread(i,j+1), getThread(i-1,j+1), getThread(i-1,j));
			}
		}
		// starts all threads
		for (int i=0; i<hSplit; i++) {
			for (int j=0; j<vSplit; j++) {
				
				threads[j][i].start();
			}
		}	
		// waits for all threads to finish
		for (int i=0; i<hSplit; i++) {
			for (int j=0; j<vSplit; j++) {
				try {
					threads[j][i].join();
				} catch (InterruptedException e) {

				}
			}
		}				
		
		return m_field;
	}
	// checks if a coordinate is in the threads' array's bounds
	private boolean isInArr(int x,int y) {
		return ((x>=0) && (y>=0) && (x<width) && (y<height));
	}
	// returns a pointer to the requested thread
	private LifeThreads getThread(int x, int y){
		if (isInArr(x, y)) {
			return threads[y][x];
		}
		else return null;
	}
}
