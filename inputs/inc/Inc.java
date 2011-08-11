import inc.*;
import java.io.*;

public class Inc
{
	public static String readLine()
	{
		String s = "";
		try {
			InputStreamReader converter = new InputStreamReader(System.in);
			BufferedReader in = new BufferedReader(converter);
			s = in.readLine();
		} catch (Exception e) {
			System.out.println("Error! Exception: "+e); 
		}
		return s;
	}
        public static void main(String[] argv)
        {
                if(argv.length < 3)
                {
                        System.err.println("inc num_threads num_iter is_synched");
                        return;
                }
                int num_threads = Integer.parseInt(argv[0]);
                int num_iter = Integer.parseInt(argv[1]);
                boolean is_synched = Boolean.parseBoolean(argv[2]);

                //readLine();

                Cell cell = new Cell();
            //  Cell cell2 = new Cell();

                if(is_synched)
                        for (int i = 0; i < num_threads; i++)
                                new SynchWorker(i,cell, num_iter).run();
                else
                        for (int i = 0; i < num_threads; i++)
                                new Worker(i,cell, num_iter).run();
            //  Worker w1 = new Worker( 0, cell, num_iter ); 
            //  Worker w2 = new Worker( 1, cell, num_iter ); 
            //  Worker w3 = new Worker( 2, cell, num_iter ); 
            //  Worker w4 = new Worker( 3, cell, num_iter ); 
            //  w1.run();
            //  w2.run();
            //  w3.run();
            //  w4.run();

                System.out.println("Result: " + cell.data + " " + cell.id);
        }
} 
