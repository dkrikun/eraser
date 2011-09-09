import inc.*;
import java.io.*;

public class Inc
{
        public static void main( String[] argv )
        {
                if( argv.length < 1 )
                {
                        System.err.println( "expected scenario number" );
                        return;
                }

                switch( Integer.parseInt(argv[0]) )
                {
                        case 0: two_unsynched_workers(); break;
                        case 1: two_synched_workers(); break;
                        case 2: two_method_synched_workers(); break;
                        case 3: two_synched_workers_and_main_thread_unsynched_but_initialization_issue_false_negative(); break;
                        case 4: three_different_workers(); break;
                        default: System.err.println( "no such scenario" ); return;
                }

                try{ Thread.sleep(10000); } catch(java.lang.InterruptedException e) { System.out.println("INterrupted!!"); }
        }

        public static void two_unsynched_workers()
        {
                int num_iter = 10 ;
                Cell cell = new Cell();

                Worker w1 = new Worker( 1, cell, num_iter ); 
                Worker w2 = new Worker( 2, cell, num_iter ); 
                w1.start();
                w2.start();
                System.out.println( "Expected lots of alarms" );
        }
        public static void two_synched_workers()
        {
                int num_iter = 10 ;
                Cell cell = new Cell();

                Worker w1 = new SynchWorker( 1, cell, num_iter ); 
                Worker w2 = new SynchWorker( 2, cell, num_iter ); 
                w1.start();
                w2.start();
                System.out.println( "Expected no alarms" );
        }

        public static void two_method_synched_workers()
        {
                int num_iter = 10 ;
                Cell cell = new Cell();

                Worker w1 = new MethodSynchWorker( 1, cell, num_iter ); 
                Worker w2 = new MethodSynchWorker( 2, cell, num_iter ); 
                w1.start();
                w2.start();
                System.out.println( "Expected no alarms" );
        }

        public static void two_synched_workers_and_main_thread_unsynched_but_initialization_issue_false_negative()
        {
                int num_iter = 10 ;
                Cell cell = new Cell();

                Worker w1 = new SynchWorker( 1, cell, num_iter ); 
                Worker w2 = new SynchWorker( 2, cell, num_iter ); 
                w1.start();
                w2.start();

                System.out.println("Result: " + cell.data + " " + cell.id);
                System.out.println( "Expected no alarms" );
        }

        public static void three_different_workers()
        {
                int num_iter = 10 ;
                Cell cell = new Cell();

                Worker w1 = new SynchWorker( 1, cell, num_iter ); 
                Worker w2 = new Worker( 2, cell, num_iter ); 
                Worker w3 = new MethodSynchWorker( 3, cell, num_iter ); 
                w1.start();
                w2.start();
                w3.start();

                System.out.println( "Expected alarms here" );
        }
} 
