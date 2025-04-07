package main

func maxrain(array []int) int {
	ans := 0
	if len(array) <= 1 {
		return ans
	}

	left := 0
	right := len(array) - 1
	for left < right {
		if array[left] < array[right] {
			ans = max(ans, array[left]*(right-left))
			left++
		} else {
			ans = max(ans, array[right]*(right-left))
			right--
		}
	}
	return ans
}

func bubbleSort(array []int) []int {
	i := 0
	seg := len(array) - 1
	for i < len(array)-1 {
		j := 0
		isSorted := false
		changePos := 0
		for j < seg {
			if array[j] > array[j+1] {
				array[j], array[j+1] = array[j+1], array[j]
				isSorted = true
				changePos = j
			}
			j++
		}
		seg = changePos
		if !isSorted {
			break
		}
		i++
	}
	return array
}

func bubbleSort2(array []int) []int {
	for i := 0; i < len(array)-1; i++ {
		for j := 0; j < len(array)-1-i; j++ {
			if array[j] > array[j+1] {
				array[j], array[j+1] = array[j+1], array[j]
			}
		}
	}
	return array
}

func selectionSort(array []int) []int {
	for i := 0; i < len(array)-1; i++ {
		minPos := i
		for j := i + 1; j < len(array); j++ {
			if array[j] < array[minPos] {
				minPos = j
			}
		}
		if minPos != i {
			array[i], array[minPos] = array[minPos], array[i]
		}
	}
	return array
}

func insertSort(array []int) []int {
	for i := 1; i < len(array); i++ {
		j := i
		for j > 0 {
			if array[j] < array[j-1] {
				array[j], array[j-1] = array[j-1], array[j]
				j--
			} else {
				break
			}
		}
	}
	return array
}

func patition(array []int, start int, end int) int {
	pivot := array[start]
	for start < end {
		for start < end && array[end] >= pivot {
			end--
		}
		array[start] = array[end]
		for start < end && array[start] <= pivot {
			start++
		}
		array[end] = array[start]
	}
	array[start] = pivot
	return start
}

func quickSort(array []int, start int, end int) {
	if start >= end {
		return
	}
	index := patition(array, start, end)
	quickSort(array, start, index-1)
	quickSort(array, index+1, end)
}

func mergeSort(array []int, start int, end int) {
	if start >= end {
		return
	}
	mid := start + (end-start)/2
	mergeSort(array, start, mid)
	mergeSort(array, mid+1, end)
	merge(array, start, mid, end)
}

func merge(array []int, start int, mid int, end int) {
	i := start
	j := mid + 1
	tmp := make([]int, 0)
	for i <= mid && j <= end {
		if array[i] < array[j] {
			tmp = append(tmp, array[i])
			i++
		} else {
			tmp = append(tmp, array[j])
			j++
		}
	}
	for i <= mid {
		tmp = append(tmp, array[i])
		i++
	}
	for j <= end {
		tmp = append(tmp, array[j])
		j++
	}
	for i := 0; i < len(tmp); i++ {
		array[start+i] = tmp[i]
	}
}

func main() {
	array := []int{1, 3, 5, 2, 4, 6}
	// fmt.Println(maxrain(array))
	// fmt.Println(bubbleSort(array))
	// fmt.Println(bubbleSort2(array))
	// fmt.Println(selectionSort(array))
	// fmt.Println(insertSort(array))
	// quickSort(array, 0, len(array)-1)
	mergeSort(array, 0, len(array)-1)
	// fmt.Println(array)
}
