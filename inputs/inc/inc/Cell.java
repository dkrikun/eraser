package inc;

public class Cell
{
        public int data;
        public int id;

        synchronized public void set_id( int new_id )
        {
                id = new_id;
        }

        public static int sdata;
}


      

