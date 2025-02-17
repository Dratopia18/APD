package multipleProducersMultipleConsumers;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class Buffer {
	private final BlockingQueue<Integer> queue;

	public Buffer(int size) {
		this.queue = new ArrayBlockingQueue<>(size);
	}

	void put(int value) {
		try {
			queue.put(value);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	int get() {
		try {
			return queue.take();
		} catch (InterruptedException e) {
			e.printStackTrace();
			return -1;
		}
	}
}
