package inc;

public class MethodSynchWorker extends Worker
{
        public MethodSynchWorker(int id, Cell cell, int num_iter)
        {
                super(id,cell,num_iter);
        }

        public void run()
        {
             // sc.data = id;
                for( int j=0; j<num_iter; ++j )
                        step();
        }

        Object another_lock = new Object();
        Object yet_another_lock = new Object();

        synchronized public void step()
        {
                synchronized(another_lock)
                {
                        synchronized(yet_another_lock)
                        {
                                cell.data ++;
                                cell.id = id;
                             // Cell.sdata ++;
                             // cell.subcell = sc;
                        }
                }
        }
}

