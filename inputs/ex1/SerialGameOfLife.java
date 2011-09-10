package ex1;


public class SerialGameOfLife implements GameOfLife {
	public boolean[][] invoke(boolean[][] initalField, int hSplit, int vSplit, int generations) {
		
		// init the field -> copy the field to the local array
		boolean[][] input=new boolean[initalField.length][];
		boolean[][] sick=new boolean[initalField.length][];
		boolean[][] dead4good=new boolean[initalField.length][];
		
		for (int i=0;i<initalField.length;i++){
			input[i]=new boolean[initalField[0].length];
			sick[i]=new boolean[initalField[0].length];
			dead4good[i]=new boolean[initalField[0].length];
			for (int k=0;k<input[i].length;k++){
				input[i][k]=initalField[i][k];
				sick[i][k]=false;
				dead4good[i][k]=false;
			}
		}

		boolean[][] result=new boolean[initalField.length][];
		
		for (int g=0;g<generations;g++){
			for (int i=0;i<initalField.length;i++){
				if (result[i]==null){
					// using first time -> copy the array
					result[i]=new boolean[initalField[i].length];
				}
				
				for (int j=0;j<initalField[i].length;j++){
					int numNeighbors=numNeighbors(i,j,input);
					result[i][j]=false; 

					if (dead4good[i][j])
						continue;

					if (numNeighbors==3 || (input[i][j]&& numNeighbors==2))
					{
							result[i][j]=true;
							sick[i][j]=false;							
					}
						
					if (numNeighbors==4)
					{
						if (sick[i][j])
						{
							// Sick for the second time, dies for good
				      		dead4good[i][j]=true;
							sick[i][j]=false;
						}
						else
						{
				      		if (input[i][j])
				      		{
					      		sick[i][j]=true;
					      		result[i][j]=true;
				      		}
						}
					}
					else
					{
						sick[i][j]=false;
					}
				}
			}
			boolean[][] tmp;
			tmp=input;
			input=result;
			result=tmp;
		}
		return input;
	}
	
	private int numNeighbors(int x,int y, boolean[][] field ){
		int counter=(field[x][y]?-1:0);
		for (int i=x-1; i<=x+1;i++ ){
			if (i<0||i>=field.length){ continue ; }
			for (int j=y-1; j<=y+1;j++){
				if (j<0||j>=field[0].length){ continue ; }
				counter+=(field[i][j]?1:0);
			}
		}
		return counter;
	}
}
