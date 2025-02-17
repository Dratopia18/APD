package parallelTree;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

public class ReadTreePart implements Runnable {
	TreeNode tree;
	String fileName;
	CyclicBarrier barrier;

	public ReadTreePart(TreeNode tree, String fileName, CyclicBarrier barrier) {
		this.tree = tree;
		this.fileName = fileName;
		this.barrier = barrier;
	}

	@Override
	public void run() {
		try {
			Scanner scanner = new Scanner(new File(fileName));
			while (scanner.hasNextInt()) {
				int child = scanner.nextInt();
				int root = scanner.nextInt();

				TreeNode treeNode = tree.getNode(root);
				synchronized (treeNode) {
					while (treeNode == null) {
						treeNode = tree.getNode(root);
					}
					treeNode.addChild(new TreeNode(child));
				}
			}
			barrier.await();
		} catch (FileNotFoundException | InterruptedException | BrokenBarrierException e) {
			e.printStackTrace();
		}
	}
}