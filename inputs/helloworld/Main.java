


public class Main
{
        public static void main(String[] argv)
        {
                int num_threads = 4;
                if(argv.length > 0)
                        num_threads = Integer.parseInt(argv[0]);

                for (int i = 0; i < num_threads; i++)
                {
                        final int nr = i ;
                        new Thread( new Runnable()
                                        {
                                                public void run()
                                                {
                                                        System.out.println("Thread " + new String[] { "one", "two", "three", "four" }[nr%4] + " says Hello World!");
                                                }
                                        }
                                  ).start();
                }
        }
}       

