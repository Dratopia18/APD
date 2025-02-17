package shortestPathsFloyd_Warshall;

public class Main {

    public static void main(String[] args) {
        int M = 9;
        int[][] graph = {{0, 1, M, M, M},
                {1, 0, 1, M, M},
                {M, 1, 0, 1, 1},
                {M, M, 1, 0, M},
                {M, M, 1, M, 0}};

        int numThreads = 4;
        int N = graph.length;
        Thread[] threads = new Thread[numThreads];

        for (int k = 0; k < N; k++) {
            for (int t = 0; t < numThreads; t++) {
                threads[t] = new FWThread(graph, t, N, numThreads, k);
                threads[t].start();
            }

            for (int t = 0; t < numThreads; t++) {
                try {
                    threads[t].join();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                System.out.print(graph[i][j] + " ");
            }
            System.out.println();
        }
    }
}