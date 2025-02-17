package task2;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

public class ColorGraph implements Runnable {
    private final int[] colors;
    private final int step;
    private final int N;
    private final int COLORS;
    private final int[][] graph;
    private final ExecutorService executor;
    private final AtomicInteger inQueue;
    private static final List<String> results = Collections.synchronizedList(new ArrayList<>());

    public ColorGraph(int[] colors, int step, int N, int COLORS, int[][] graph, ExecutorService executor, AtomicInteger inQueue) {
        this.colors = colors;
        this.step = step;
        this.N = N;
        this.COLORS = COLORS;
        this.graph = graph;
        this.executor = executor;
        this.inQueue = inQueue;
    }

    @Override
    public void run() {
        colorGraph(colors, step);
        inQueue.decrementAndGet();
        if (inQueue.get() == 0) {
            executor.shutdown();
            printResults();
        }
    }

    private void colorGraph(int[] colors, int step) {
        if (step == N) {
            addResult(colors);
            return;
        }

        for (int i = 0; i < COLORS; i++) {
            int[] newColors = colors.clone();
            newColors[step] = i;
            if (verifyColors(newColors, step)) {
                inQueue.incrementAndGet();
                executor.submit(new ColorGraph(newColors, step + 1, N, COLORS, graph, executor, inQueue));
            }
        }
    }

    private boolean verifyColors(int[] colors, int step) {
        for (int i = 0; i < step; i++) {
            if (colors[i] == colors[step] && isEdge(i, step))
                return false;
        }
        return true;
    }

    private boolean isEdge(int a, int b) {
        for (int[] edge : graph) {
            if (edge[0] == a && edge[1] == b)
                return true;
        }
        return false;
    }

    private void addResult(int[] colors) {
        StringBuilder aux = new StringBuilder();
        for (int color : colors) {
            aux.append(color).append(" ");
        }
        results.add(aux.toString().trim());
    }

    private void printResults() {
        Collections.sort(results);
        for (String result : results) {
            System.out.println(result);
        }
    }
}
