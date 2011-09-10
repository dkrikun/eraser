package ex1;

// a class representing a cell
public class Cell {
	int gen=0;
	public enum status {DEAD, ALIVE, SICK, DEAD_FOR_GOOD}
	status even, odd;
	// constructor for a Cell object. gets a boolean value and sets status to ALIVE or DEAD accordingly
	public Cell(boolean data) {
		if (data==true) {
			even=status.ALIVE;
		}
		else even=status.DEAD;
		odd=status.DEAD;
	}
	// getter for the even generation's status
	public status getEven() {
		return even;
	}
	// setter for the even generation's status
	public void setEven(status even) {
		this.even = even;
	}
	// getter for the odd generation's status
	public status getOdd() {
		return odd;
	}
	// setter for the odd generation's status
	public void setOdd(status odd) {
		this.odd = odd;
	}
	// getter for the cell's current generation
	public int getGen() {
		return gen;
	}
	// promotes the cell's generation by 1
	public void promoteGen() {
		this.gen++;
	}
	
}
