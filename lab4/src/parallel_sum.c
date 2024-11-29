#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>
#include <pthread.h>

#include "../../lab3/src/utils.h"
#include <stdint.h>
#include "sum.h"

struct SumArgs {
  int *array;
  int begin;
  int end;
};

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)SumParallel(sum_args);
}

  int main(int argc, char **argv) {
    /*
    *  threads_num by command line arguments
    *  array_size by command line arguments
    *	 seed by command line arguments
    */
  uint32_t threads_num = 0; //беззнаковое 32бит целое
  uint32_t array_size = 0;
  uint32_t seed = 0;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0: // --seed
            seed = atoi(optarg);
            if (seed < 0) {  // Если seed отрицателен
              fprintf(stderr, "Error: seed must be a non-negative integer.\n");
              return 1;  
            }
            break;
          case 1: // --array_size
            array_size = atoi(optarg);
            if (array_size <= 0) {  // Если размер массива меньше или равен 0
              fprintf(stderr, "Error: array_size must be a positive integer.\n");
              return 1;  
            }
            break;
          case 2: // --pnum
            threads_num = atoi(optarg);
            if (threads_num == 0){
              printf("Arg pnum cannot be 0\n");
              return 1;
            }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
            return 1;
        }
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (threads_num > array_size) {
    printf("Error: number of threads is greater than the array size!\n");
    return 1;
  }
  /*
   * TODO:
   * your code here
   * Generate array here
   */

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  if (!array) {
    printf("Error: memory allocation failed.\n");
    return 1;
  }

  // фиксация запуска
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  // обеспечение многопоточности
  pthread_t threads[threads_num];
  int chunk_size = array_size/threads_num;
  struct SumArgs args[threads_num];

  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * chunk_size;
    args[i].end = (i + 1) * chunk_size;

    // Если есть остаток, то добавим его в последний поток
    if (i == threads_num - 1) {
      args[i].end += (int)array_size % threads_num; 
    }
  }

  // создание процессов
  for (uint32_t i = 0; i < threads_num; i++) {
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  // ожидание завершения процессов
  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int sum = 0;
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }
  // фиксация конца
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  printf("Total: %d\n", total_sum);
  printf("Total time: %fms\n", elapsed_time);
  return 0;
}
// ./psum --seed 123 --array_size 100 --pnum 4