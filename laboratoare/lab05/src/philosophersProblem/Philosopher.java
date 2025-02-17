package philosophersProblem;

import java.util.concurrent.locks.Lock;
public class Philosopher implements Runnable {
    private final Object leftFork;
    private final Object rightFork;
    private final int id;

    public Philosopher(int id, Object leftFork, Object rightFork) {
        this.leftFork = leftFork;
        this.rightFork = rightFork;
        this.id = id;
    }

    private void sleep() {
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void eat() {
        System.out.println("Philosopher " + id + " is eating");
        sleep();
    }

    public void think() {
        System.out.println("Philosopher " + id + " is thinking");
        sleep();
    }

    @Override
    public void run() {
        think();
        if (id % 2 == 0) {
            synchronized (leftFork) {
                synchronized (rightFork) {
                    eat();
                }
            }
        } else {
            synchronized (rightFork) {
                synchronized (leftFork) {
                    eat();
                }
            }
        }
    }
}
