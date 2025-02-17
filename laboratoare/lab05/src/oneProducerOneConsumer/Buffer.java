package oneProducerOneConsumer;

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
        notify();
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
        notify();
        return value;
    }
}
