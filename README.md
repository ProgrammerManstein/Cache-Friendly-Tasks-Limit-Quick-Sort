# Cache-Friendly-Tasks-Limit-Quick-Sort
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

## comparison
The files in the comparison are using one single strategy that is used by task_limit_qst.cpp. The pdf report explains the different strategies. You can use them to compare with og.cpp and task_limit_qst.cpp to see the improvement.
