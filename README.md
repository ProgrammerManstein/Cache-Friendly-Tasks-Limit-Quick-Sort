# Cache-Friendly-Tasks-Limit-Quick-Sort
Sorting is a common and fundamental problem in the field of computer science.
Improving the speed of sorting is a valuable pursuit. In this program, we propose a
way to paralle quicksort using OpenMP. The main method we use to improve the
performance is setting cut-offs. Our work reveals the importance of cache efficiency
and different strategies scheduling stra regarding the performance of parallel program.
## Files
og.cpp is the naive version that only use task for recursive.

task_limit_qst.cpp is our final version.

## How to use
Compiling it by typing: 
```
g++ -Wall -fopenmp -o task_limit_qst task_limit_qst.cpp
```
```
g++ -Wall -fopenmp -o og og.cpp
```
usage: 
```
./task_limit_qst t n
```
```
./og t n
```
with t threads for the array size n sort.

## Comparison
The files in the comparison are using one single strategy that is used by task_limit_qst.cpp. The pdf report explains the different strategies. You can use them to compare with og.cpp and task_limit_qst.cpp to see the improvement.
