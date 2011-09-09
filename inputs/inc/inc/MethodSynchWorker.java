package inc;

public class MethodSynchWorker extends Worker
{
        public MethodSynchWorker(int id, Cell cell, int num_iter)
        {
                super(id,cell,num_iter);
        }

        public void run()
        {
                for( int j=0; j<num_iter; ++j )
                        cell.set_id( id );
        }
}

