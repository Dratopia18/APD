package bug2;

/**
 * Why does this code not block? We took the same lock twice!
 *
 * There is nothing to modify for this example. Just explain why we do NOT
 * have a deadlock.
 * Because taking the same lock twice by the same thread doesn't
 * cause a deadlock
 */
public class MyThread implements Runnable {
    static int i;

    @Override
    public void run() {
        synchronized (this) {
            synchronized (this) {
                i++;
            }
        }
    }
}
