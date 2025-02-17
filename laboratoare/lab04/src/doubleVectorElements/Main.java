package doubleVectorElements;

public class Main {

    public static void main(String[] args) {
        int N = 100000013;
        int[] v = new int[N];
        int P = 4; // the program should work for any P <= N

        for (int i = 0; i < N; i++) {
            v[i] = i;
        }

        // Parallelize me using P threads
        Thread[] threads = new Thread[P];
        int chunk = (N + P - 1) / P;
        for (int i = 0; i < P; i++) {
            final int start = i * chunk;
            final int end = Math.min(start + chunk, N);
            threads[i] = new Thread(() -> {
                for (int j = start; j < end; j++) {
                    v[j] *= 2;
                }
            });
            threads[i].start();
        }

        for (int i = 0; i < P; i++) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        for (int i = 0; i < N; i++) {
            if (v[i] != i * 2) {
                System.out.println("Wrong answer");
                System.exit(1);
            }
        }
        System.out.println("Correct");
    }

}
