package task3;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

public class Queens {
    private final int N;
    private final int[] arr;
    private final int step;
    private final int[] sol;
    private final int[] graph;
    private ExecutorService executor;
    private AtomicInteger inQueue;

    public Queens(int N, int[] arr, int step, int[] sol, int[] graph, ExecutorService executor, AtomicInteger inQueue) {
        this.N = N;
        this.arr = arr;
        this.step = step;
        this.sol = sol;
        this.graph = graph;
        this.executor = executor;
        this.inQueue = inQueue;
    }

    private static boolean check(int[] arr, int step) {
        for (int i = 0; i <= step; i++) {
            for (int j = i + 1; j <= step; j++) {
                if (arr[i] == arr[j] || arr[i] + i == arr[j] + j || arr[i] + j == arr[j] + i)
                    return false;
            }
        }
        return true;
    }
}
