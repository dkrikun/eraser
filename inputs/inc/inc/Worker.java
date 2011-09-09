
package inc;

public class Worker extends java.lang.Thread
{
        public final int id;
        public final Cell cell;
        public final int num_iter;

        public Worker(int id, Cell cell, int num_iter)
        {
                this.id = id;
                this.cell = cell;
                this.num_iter = num_iter;
        }

        public void run()
        {
                for( int j=0; j<num_iter; ++j )
                {
                        cell.data ++;
                        cell.id = id;
                        Cell.sdata ++;
                }
        }
}

