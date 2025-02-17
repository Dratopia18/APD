package shortestPathsFloyd_Warshall;

import static java.lang.Math.ceil;
import static java.lang.Math.min;

public class FWThread extends Thread {
    private int[][] graph;
    private final int tid;
    private final double N;
    private final double numberOfThreads;
    private final int k;

    public FWThread(int[][] graph, int tid, double N, double numberOfThreads, int k) {
        this.graph = graph;
        this.tid = tid;
        this.N = N;
        this.numberOfThreads = numberOfThreads;
        this.k = k;
    }

    @Override
    public void run() {
        int start = tid * (int)ceil(N / numberOfThreads);
        int end = (int) min((tid + 1) * (int)ceil(N / numberOfThreads), N);

        for (int i = start; i < end; i++) {
            for (int j = 0; j < N; j++) {
                synchronized (FWThread.class) {
                    graph[i][j] = Math.min(graph[i][k] + graph[k][j], graph[i][j]);
                }
            }
        }
    }
}