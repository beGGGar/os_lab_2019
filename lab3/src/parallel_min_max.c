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

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  int timeout = -1;  // Таймаут в секундах
  bool is_timeout = false;

  pid_t *child_pids = NULL;

  /*
    Прерывает выполнение при получении сигнала
  */
  void handle_alarm(int sig) {
    printf("Timeout reached, sending SIGKILL to child processes\n");
    is_timeout = true;
    for (int i = 0; i < pnum; i++) {
      if (child_pids[i] > 0) {
        kill(child_pids[i], SIGKILL);  // Отправляем сигнал SIGKILL
      }
    }
  }

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0},
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
            pnum = atoi(optarg);
            if (pnum <= 0) {  // Если количество процессов меньше или равно 0
              fprintf(stderr, "Error: pnum must be a positive integer.\n");
              return 1;  
            }
            break;
          case 3: // --by_files
            with_files = true;
            break;

          case 4: // --timeout
            timeout = atoi(optarg);
             if (timeout < 0) {
              fprintf(stderr, "Error: timeout must be a non-negative integer.\n");
              return 1;
            }
          break;
          default:
            printf("Index %d is out of options\n", option_index);
            return 1;
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n%d %d\n", optind, argc);
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  // установка timeout
  if (timeout != -1) {
    signal(SIGALRM, handle_alarm);  // Устанавливаем обработчик для SIGALRM
    alarm(timeout);  // Устанавливаем таймер на таймаут
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  int chunk_size = array_size/pnum;
  
  // для pipe
  int pipes[pnum][2];
  if (!with_files){
    
    for (int i = 0; i < pnum; i++) {
        if (pipe(pipes[i]) == -1) {
            printf("pipe_err");
            exit(1);
        }
    }

  }
  
  child_pids = malloc(pnum * sizeof(pid_t));

  for (int i = 0; i < pnum; i++) {
    child_pids[i] = fork();
    sleep(10000);
    /*
      Дочерние процессы закрываются хендлером, но маин процесс все еще спит. Не понимаю как решить
    */
    if (child_pids[i] >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pids[i] == 0) {
        // child process
        // определение границ
        int start = i*chunk_size;
        int end = (i == pnum - 1) ? array_size : (i + 1) * chunk_size;
        
        struct MinMax min_max = GetMinMax(array, start, end);

        if (with_files) {
          // use files here
          char filename[64];
          sprintf(filename, "result_%d.txt", i);
          FILE *file = fopen(filename, "w");
          if (file) {
            fprintf(file, "%d\n%d", min_max.min, min_max.max);
            fclose(file);
          }
        } else {
          // use pipe here
          close(pipes[i][0]); // закрыть чтение
          write(pipes[i][1], &min_max, sizeof(min_max));
          close(pipes[i][1]);  // закрыть запись
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  while (active_child_processes > 0) {
    if (is_timeout){
      break;
    }
    
    wait(NULL);  // Блокирует выполнение, пока любой дочерний процесс не завершится

    // Уменьшаем счетчик активных дочерних процессов
    active_child_processes -= 1;
  }

  //  while (active_child_processes > 0) {
  //   pid_t finished_pid = waitpid(-1, NULL, WNOHANG);
  //   if (finished_pid > 0) {
  //     active_child_processes -= 1;
  //   }
  //   usleep(100000);  // 100 миллисекунд
  // }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;
    struct MinMax min_max_buff;

    if (with_files) {
      // read from files
      char filename[64];
      sprintf(filename, "result_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file) {
        fscanf(file, "%d\n%d", &min, &max);
        fclose(file);
        if (remove(filename) != 0) {
          perror("Error deleting file");
        }
      }
    } else {
      
      // read from pipes
        read(pipes[i][0], &min_max_buff, sizeof(min_max));
        close(pipes[i][0]);
        min = min_max_buff.min;
        max = min_max_buff.max;
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  // // закрытие труб
  // for (int i = 0; i < pnum; i++) {
  //   close(pipes[i][0]); // Закрытие пайпа для чтения в родительском процессе
  // }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
