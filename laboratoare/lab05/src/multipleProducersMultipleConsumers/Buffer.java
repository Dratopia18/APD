package multipleProducersMultipleConsumers;

public class Buffer {
    private int a;
    private boolean empty = true;

    synchronized void put(int value) {
        while (!empty) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        a = value;
        empty = false;
        notifyAll();
    }

    synchronized int get() {
        while (empty) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        int value = a;
        empty = true;
        notifyAll();
        return value;
    }
}
