package task1;

import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

public class GetPath implements Runnable {
    private final ArrayList<Integer> partialPath;
    private final int destination;
    private final int[][] graph;
    private final ExecutorService executor;
    private final AtomicInteger inQueue;

    public GetPath(ArrayList<Integer> partialPath, int destination, int[][] graph, ExecutorService executor, AtomicInteger inQueue) {
        this.partialPath = partialPath;
        this.destination = destination;
        this.graph = graph;
        this.executor = executor;
        this.inQueue = inQueue;
    }

    @Override
    public void run() {
        findPath(partialPath, destination);
        inQueue.decrementAndGet();
        if (inQueue.get() == 0) {
            executor.shutdown();
        }
    }

    private void findPath(ArrayList<Integer> partialPath, int destination) {
        if (partialPath.get(partialPath.size() - 1) == destination) {
            System.out.println(partialPath);
            return;
        }

        int lastNodeInPath = partialPath.get(partialPath.size() - 1);
        for (int[] edge : graph) {
            if (edge[0] == lastNodeInPath) {
                if (partialPath.contains(edge[1])) {
                    continue;
                }
                ArrayList<Integer> newPartialPath = new ArrayList<>(partialPath);
                newPartialPath.add(edge[1]);
                inQueue.incrementAndGet();
                executor.submit(new GetPath(newPartialPath, destination, graph, executor, inQueue));
            }
        }
    }
}
