
package inc;

public class SynchWorker extends Worker
{
        public SynchWorker(int id, Cell cell, int num_iter)
        {
                super(id,cell,num_iter);
        }

        public void run()
        {
                sc.data = id;
                synchronized(this)
                {
                        for( int j=0; j<num_iter; ++j )
                        {
                                cell.data ++;
                                cell.id = id;
                                Cell.sdata ++;
                                cell.subcell = sc;
                        }
                }
        }
}

