#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include<algorithm>
#include<vector>
#include<limits.h>
#include<cmath>

using namespace std;

int task_num = 1;

int cache_size = 500000;

int numThreads;
int depth_limit;

int lock = 0;

void swap_val(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

//Choose pivot using median3
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

//Sequential quicksort
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

void InsertionSort(int* array, int left, int right) {
	for (int i = left + 1; i < right; i++) {
		int key = array[i];
		int j = i - 1;

		// Move elements greater than key to one position ahead
		// of their current position 
		while (j >= left && array[j] > key) {
			array[j + 1] = array[j];
			j = j - 1;
		}
		array[j + 1] = key;
	}
}


void Random(int* a, int n, int l, int r, int seed)//Generate random numbers in the interval r~l
{
	srand(seed); 
	for (int i = 0; i < n; i++) {
		a[i] = rand() % (r - l + 1) + l;
	}
}



void quickSort_parallel(int* array, int left, int right, int threads, int cutoff);
void quickSort_parallel_internal(int* array, int left, int right, int cutoff, int is_task, int depth);


//Our parallel quicksort
void quickSort_parallel(int* array, int left, int right, int threads, int cutoff) {
	numThreads = threads;
	depth_limit = log(right - left);

#pragma omp parallel num_threads(numThreads)
	{
#pragma omp single nowait
		{
			quickSort_parallel_internal(array, left, right, cutoff, 1, 0);
		}
	}
}



void quickSort_parallel_internal(int* array, int left, int right, int cutoff, int is_task, int depth)
{
	//Use insertion sort if the length is smaller than 16
	if (right - left > 16) {
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
		//Generate tasks immediately when the length is greater than cache size;
		//Only when the tasks around the cache size are consumed until they are less than the number of threads will tasks continue to be generated
		//If there are too many tasks or the length is too small (less than cutoff), new tasks will not be generated, but sequential recursion will be performed.
		if (i - left > cache_size || (lock == 1 && (i - left > cutoff && task_num < 2 * numThreads) && depth < 4 * numThreads)) {
			depth++;
#pragma omp atomic
			task_num++;
#pragma omp task	
			{
				//Output the generation order of tasks
				//int id = omp_get_thread_num();
				//cout << id << ": " << left << "-" << i << endl;

				quickSort_parallel_internal(array, left, i, cutoff, 1, depth);
			}
		}
		else if (i - left > 1) { quickSort_parallel_internal(array, left, i, cutoff, 0, depth); }
		//Only when the tasks around the cache size are consumed until they are less than the number of threads will the lock be unlock;
		//Equal to" else if(lock == 0 && right - i - 1 < 2 && task_num < 2 * numThreads)"
		//,since we have" if (right - i - 1 > cache_size || (lock == 1 && (right - i - 1 > cutoff && task_num < 2 * numThreads)" before
		else if (lock == 0 && i - left < 2) {
			lock = 1;
		}

		//Generate tasks immediately when the length is greater than cache size;
		//Only when the tasks around the cache size are consumed until they are less than the number of threads will tasks continue to be generated
		//If there are too many tasks or the length is too small (less than cutoff), new tasks will not be generated, but sequential recursion will be performed.
		if (right - i - 1 > cache_size || (lock == 1 && (right - i - 1 > cutoff && task_num < 2 * numThreads) && depth < 4 * numThreads)) {
#pragma omp atomic
			task_num++;
#pragma omp task	
			{
				//Output the generation order of tasks
				//int id = omp_get_thread_num();
				//cout << id << ": " << i + 1 << "-" << right << endl;

				quickSort_parallel_internal(array, i + 1, right, cutoff, 1, depth);
			}
		}
		else if (right - i - 1 > 1) {
			quickSort_parallel_internal(array, i + 1, right, cutoff, 0, depth);
		}
		//Only when the tasks around the cache size are consumed until they are less than the number of threads will the lock be unlock;
		//Equal to" else if(lock == 0 && right - i - 1 < 2 && task_num < 2 * numThreads)"
		//,since we have" if (right - i - 1 > cache_size || (lock == 1 && (right - i - 1 > cutoff && task_num < 2 * numThreads)" before
		else if (lock == 0 && right - i - 1 < 2) {
			lock = 1;
		}
		if (is_task == 1) {
#pragma omp atomic
			task_num--;

		}
		//cout << task_num << " ";
	}
	else {  //Use insertion sort if the length is smaller than 16
		if (lock == 0) {
			lock = 1;
		}
		if (is_task == 1) {
#pragma omp atomic
			task_num--;

		}
		InsertionSort(array, left, right);
	}

}

int main(int argc, char* argv[])
{
	int arg_threads = atoi(argv[1]);
	int arg_size = atoi(argv[2]);	
	int* nums = new int[arg_size];
	double t_start;
	double t_taken;
	int flag = 1;
	int seed = 0;
	int size = arg_size;
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
	//Output time cost with different of cut
	for (int i = 100; i < size; i = i * 10) {
		cout << "cut off: " << i << "; ";
		Random(nums, size, 0, INT_MAX, seed);

		task_num = 1;
		lock = 0;

		t_start = omp_get_wtime();
		quickSort_parallel(nums, 0, size, arg_threads, i);
		t_taken = omp_get_wtime() - t_start;
		//Time output
		cout << t_taken << endl;
		//Check if sorting is correct
		flag = 1;
		for (int i = 0; i < size - 1; i++) {
			if (nums[i] > nums[i + 1])
				flag = 0;
		}
		cout << flag << endl;


		cout << endl;
	}

}

