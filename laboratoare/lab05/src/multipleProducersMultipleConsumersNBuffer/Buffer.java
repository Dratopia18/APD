package multipleProducersMultipleConsumersNBuffer;

import java.util.Queue;

public class Buffer {
    
    Queue<Integer> queue;
    private final int size;
    
    public Buffer(int size) {
        this.size = size;
        queue = new LimitedQueue<>(size);
    }

	public synchronized void put(int value) {
        while (queue.size() == size) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        queue.add(value);
        notifyAll();
	}

	public synchronized int get() {
        while (queue.isEmpty()) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        int value = queue.poll();
        notifyAll();
        return value;
	}
}
