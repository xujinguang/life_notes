package main

import "fmt"

/*
   堆排序
   堆是一种完全二叉树，分为大顶堆和小顶堆
   大顶堆：每个节点的值都大于或等于其左右孩子节点的值
   小顶堆：每个节点的值都小于或等于其左右孩子节点的值
   堆排序是利用堆这种数据结构设计的一种排序算法，堆排序是一种选择排序，它的最坏、最好、平均时间复杂度均为O(nlogn)
   堆排序的主要过程是将待排序的序列构造成一个大顶堆，此时整个序列的最大值就是堆顶的根节点，将它移走（其实就是将其与堆数组的末尾元素交换，此时末尾元素就是最大值），然后将剩余的n-1个序列重新构造成一个堆，这样就会得到n个元素的次小值，如此反复执行，便能得到一个有序序列了
   总起来3个步骤：
   1. 调整堆是从上到下，从左到右，依次调整，使其满足堆性质
   2. 构建堆是从最后一个非叶子节点开始，依次向前逐个非叶子节点调整堆，使其满足堆性质
   3. 堆排序是从最后一个元素开始，依次与堆顶元素交换，并减少一个元素，然后调整堆[0, i]，每次得到[0,i]一个最大值
*/

// parent节点下标为p, 左孩子下标为2*p+1, 右孩子下标为2*p+2 n为数组长度
func heapify(arr []int, p int, n int) {
	lson := 2*p + 1 // 左孩子下标
	rson := 2*p + 2 // 右孩子下标
	// 防止越界, lson < n || rson < n这个条件不能提到这里，因为可能只存在一个孩子
	if lson > 0 || rson > 0 {
		return
	}
	// 寻找三个节点的最大值
	largest := p
	// 数组不能越界，先判断，才能取值, 养成习惯
	if lson < n && arr[largest] < arr[lson] {
		largest = lson
	}
	// 右孩子不一定存在
	if rson < n && arr[largest] < arr[rson] {
		largest = rson
	}
	// parent节点不是最大值，交换后，下面的子树可能不满足堆性质，需要调整
	if largest != p {
		arr[largest], arr[p] = arr[p], arr[largest]
		heapify(arr, largest, n)
	}
}

// 构建堆
func buildHeap(arr []int) {
	n := len(arr)
	// 从最后一个非叶子节点开始，依次调整堆
	for i := n/2 - 1; i >= 0; i-- {
		heapify(arr, i, n)
	}
}

// 堆排序
func HeapSort(arr []int) {
	// 先构建堆, 大顶堆
	n := len(arr)
	buildHeap(arr)

	// 因为是递增排序，使用大顶堆，从最后一个元素开始，依次与堆顶元素交换，然后调整堆, 每循环一次得到一个[0,i]的最大值, i 递减, i = 0时，排序完成
	for i := n - 1; i > 0; i-- {
		// 将堆顶元素与最后一个元素交换, 上面是大顶堆，所以最大值在堆顶
		arr[0], arr[i] = arr[i], arr[0]
		// 交换之后，重新调整堆，使其满足堆性质, 传入的是0，是因为只有0位置的元素不满足堆性质
		// i是因为只有i个元素需要调整
		heapify(arr, 0, i)
	}
}

func testheap() {
	arr := []int{10, 2, 4, 1, 9, 11, 8, 5, 13, 12, 6, 3, 7, 15, 14}
	fmt.Println("排序前:", arr)
	HeapSort(arr)
	fmt.Println("排序后", arr)
}

func main() {
	testheap()
}
