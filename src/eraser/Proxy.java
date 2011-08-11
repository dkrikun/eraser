
public class Proxy
{

    private static int engaged = 0;

    private static native void newobj_(Object thread, Object o);
    public static void newobj(Object o)
    {
        if( engaged != 0 )
        {
            newobj_(Thread.currentThread(), o);
        }
    }

    private static native void newarr_(Object thread, Object o);
    public static void newarr(Object o)
    {
        if( engaged != 0 )
        {
            newarr_(Thread.currentThread(), o);
        }
    }

    private static native void monitor_enter_(Object thread, Object o);
    public static void monitor_enter(Object o)
    {
        if( engaged != 0 )
        {
            monitor_enter_(Thread.currentThread(), o);
        }
    }

    private static native void monitor_exit_(Object thread, Object o);
    public static void monitor_exit(Object o)
    {
        if( engaged != 0 )
        {
            monitor_exit_(Thread.currentThread(), o);
        }
    }
}
