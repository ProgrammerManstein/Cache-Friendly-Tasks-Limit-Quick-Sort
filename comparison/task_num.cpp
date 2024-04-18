#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include<algorithm>
#include<vector>
#include<limits.h>
using namespace std;

int task_count = 1;

int cache_size = 500000;

int numThreads;

int lock = 0;

void swap_val(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

int median3(int* arr, int left, int right) {
	int center = (left + right) / 2;
	if (arr[left] > arr[center])
		swap_val(&arr[left], &arr[center]);
	if (arr[left] > arr[right])
		swap_val(&arr[left], &arr[right]);
	if (arr[center] > arr[right])
		swap_val(&arr[center], &arr[right]);

	// arr[left] <= arr[center] <= arr[right]
	swap_val(&arr[center], &arr[left]);  // Hide pivot
	return arr[left];                // Return median value
}


void quickSort(int* arr, int left, int right)
{
	int i = left, j = right - 1;
	int tmp;
	int pivot = median3(arr, i, j);

	/* PARTITION PART */
	while (i < j) {
		while (i < j && arr[j] >= pivot)
			j--;
		arr[i] = arr[j];

		while (i < j && arr[i] <= pivot)
			i++;
		arr[j] = arr[i];

	}

	arr[i] = pivot;
	/* RECURSION PART */
	if (i - left > 1) { quickSort(arr, left, i); }
	if (right - i - 1 > 1) { quickSort(arr, i + 1, right); }
}




void Random(int* a, int n, int l, int r, int seed)//生成范围在l~r的随机数
{
	srand(seed); //设置时间种子
	for (int i = 0; i < n; i++) {
		a[i] = rand() % (r - l + 1) + l;//生成区间r~l的随机数
	}
}



void quickSort_parallel(int* array, int left, int right, int threads, int cutoff);
void quickSort_parallel_internal(int* array, int left, int right, int cutoff, int is_task);



void quickSort_parallel(int* array, int left, int right, int threads, int cutoff) {
	numThreads = threads;
#pragma omp parallel num_threads(numThreads)
	{
#pragma omp single nowait
		{
			quickSort_parallel_internal(array, left, right, cutoff, 1);
		}
	}
}



void quickSort_parallel_internal(int* array, int left, int right, int cutoff, int is_task)
{

	int i = left, j = right - 1;
	int tmp;
	int pivot = median3(array, i, j);


	{
		/* PARTITION PART */
		while (i < j) {
			while (i < j && array[j] >= pivot)
				j--;
			array[i] = array[j];

			while (i < j && array[i] <= pivot)
				i++;
			array[j] = array[i];

		}

		array[i] = pivot;


	}

	if (i - left > cache_size ||(lock==1 && (i - left > cutoff && task_count < 2 * numThreads))) {
#pragma omp atomic
		task_count++;
#pragma omp task	
		{
			//int id = omp_get_thread_num();
			//cout << id << ": " << left << "-" << i << endl;

			quickSort_parallel_internal(array, left, i, cutoff, 1);
		}
	}
	else if (i - left > 1) { quickSort_parallel_internal(array, left, i, cutoff, 0); }
	else if (lock==0&&i - left < 2) {
		lock = 1;
	}

	if (right - i - 1 > cache_size || (lock == 1 && (right - i - 1 > cutoff && task_count < 2 * numThreads))) {
#pragma omp atomic
		task_count++;
#pragma omp task	
		{
			//int id = omp_get_thread_num();
			//cout << id << ": " << i + 1 << "-" << right << endl;

			quickSort_parallel_internal(array, i + 1, right, cutoff, 1);
		}
	}
	else if (right - i - 1 > 1) {
		quickSort_parallel_internal(array, i + 1, right, cutoff, 0);
	}
	else if (lock == 0 && right - i - 1 < 2) {
		lock = 1;
	}
	if (is_task == 1) {
#pragma omp atomic
		task_count--;	
		
	}
	//cout << task_count << " ";
}

int main()
{
	int* nums = new int[100000000];
	double t_start;
	double t_taken;
	int flag = 1;
	int seed = 0;
	int size = 100000000;
	/*
		//sq
		Random(nums, size, 0, size, seed);

		t_start = omp_get_wtime();
		quickSort(nums, 0, size);
		t_taken = omp_get_wtime() - t_start;
		cout << t_taken << endl;
		for (int i = 0; i < (sizeof(nums) / sizeof(int)); i++) {
			if (nums[i] > nums[i + 1])
				flag = 0;
		}
		cout << flag << endl;


		//stl
		Random(nums, size, 0, size, seed);

		t_start = omp_get_wtime();
		sort(nums, nums + size);
		t_taken = omp_get_wtime() - t_start;
		cout << t_taken << endl;

		flag = 1;
		for (int i = 0; i < (sizeof(nums) / sizeof(int)) - 1; i++) {
			if (nums[i] > nums[i + 1])
				flag = 0;
		}
		cout << flag << endl;

		cout << endl;

	*/
	//parallel
	for (int i = 100; i < size; i = i * 10) {
		cout << "cut off: " << i << "; ";
		Random(nums, size, 0, INT_MAX, seed);

		task_count = 1;
		lock = 0;

		t_start = omp_get_wtime();
		quickSort_parallel(nums, 0, size, 16, i);
		t_taken = omp_get_wtime() - t_start;
		cout << t_taken << endl;

		flag = 1;
		for (int i = 0; i < size - 1; i++) {
			if (nums[i] > nums[i + 1])
				flag = 0;
		}
		cout << flag << endl;


		cout << endl;

	}

}

