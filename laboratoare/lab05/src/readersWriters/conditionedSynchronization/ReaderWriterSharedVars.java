package readersWriters.conditionedSynchronization;

import java.util.concurrent.Semaphore;

public class ReaderWriterSharedVars {
    volatile int shared_value;
    Semaphore mutex;
    Semaphore rw_mutex;
    int readers;

    ReaderWriterSharedVars(int init_shared_value) {
        this.shared_value = init_shared_value;
        this.mutex = new Semaphore(1);
        this.rw_mutex = new Semaphore(1);
        this.readers = 0;
    }

    public void startRead() throws InterruptedException {
        mutex.acquire();
        readers++;
        if (readers == 1) {
            rw_mutex.acquire();
        }
        mutex.release();
    }

    public void endRead() throws InterruptedException {
        mutex.acquire();
        readers--;
        if (readers == 0) {
            rw_mutex.release();
        }
        mutex.release();
    }

    public void startWrite() throws InterruptedException {
        rw_mutex.acquire();
    }

    public void endWrite() {
        rw_mutex.release();
    }
}