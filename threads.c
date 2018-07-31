#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sysexits.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <limits.h>

#define MAX_SUM 1000000

typedef struct targ {
   int *pos;
   int load;
} Thread_arg;


struct timeval tv_delta(struct timeval start, struct timeval end) {
   struct timeval delta = end;

   delta.tv_sec -= start.tv_sec;
   delta.tv_usec -= start.tv_usec;
   if (delta.tv_usec < 0) {
      delta.tv_usec += 1000000;
      delta.tv_sec--;
   }

   return delta;
}

static void exit_with_usage();
static int *generate_array(int length, int seed);
static void *thread_max(void *args);
static void *thread_sum(void *args);

int main(int argc, char *argv[]) {
   int elt_cnt, thread_cnt, seed, task, print_res, is_yes, tmp;
   int i, *array, *thread_pos, quotient, remainder, status;
   int light_load, heavy_load, final_result;
   char tmp_chr;
   pthread_t *tids;
   void *(*thread_func) (void *), *result;
   Thread_arg *t_args;
   struct rusage start_ru, end_ru;
   struct timeval start_wall, end_wall;
   struct timeval diff_ru_utime, diff_wall, diff_ru_stime;

   /* Parse provided arguments, exiting and printing usage information
    * if an invalid argument or number of arguments is provided. */
   if (argc != 6) {
      exit_with_usage();
   }

   /* elt_cnt and thread_cnt must be > 0 */
   if ((tmp = atoi(argv[1])) > 0) {
      elt_cnt = tmp;
   } else {
      printf("%s is not a valid number of elements.\n", argv[1]);
      exit_with_usage();
   }

   if ((tmp = atoi(argv[2])) > 0) {
      thread_cnt = tmp;
   } else {
      printf("%s is not a valid number of threads.\n", argv[2]);
      exit_with_usage();
   }

   /* 0 is an acceptable seed value, so we have to validate
    * the input in a slightly more robust fashion. */
   if (sscanf(argv[3], "%d%c", &tmp, &tmp_chr) == 1) {
      seed = tmp;
   } else {
      printf("%s in not a valid seed.\n", argv[3]);
      exit_with_usage();
   }

   tmp = atoi(argv[4]);
   if (tmp == 1 || tmp == 2) {
      task = tmp;
   } else {
      printf("%s is not a valid task (must be 1 or 2)\n", argv[4]);
      exit_with_usage();
   }

   if (strlen(argv[5]) == 1 && (tmp_chr = argv[5][0])
       && ((is_yes = (tmp_chr == 'Y' || tmp_chr == 'y'))
	          || (tmp_chr == 'N' || tmp_chr == 'N'))) {

      if (is_yes) {
         print_res = 1;
      } else {
         print_res = 0;
      }
   } else {
      printf("%s is not a valid print value (must be Y or N)\n", argv[5]);
      exit_with_usage();
   }

   if (task == 1) {
      thread_func = thread_max;
   } else {
      thread_func = thread_sum;
   }

   /* Calculate number of elements to process per thread */
   quotient = elt_cnt / thread_cnt;
   /* If there are more threads than elements, reduce the number of threads
      to match the number of elements and assign one element per thread. */
   if (quotient == 0) {
      quotient = 1;
      thread_cnt = elt_cnt;     /* Is this the desired behavior? */
      remainder = 0;
   } else {
      remainder = elt_cnt % thread_cnt;
   }

   /* If there the array cannot be divided equally amongst the threads,
    * some threads will process one additional element (heavy_load) */
   light_load = quotient;
   heavy_load = quotient + 1;

   array = generate_array(elt_cnt, seed);
   thread_pos = array;

   /* Begin timing */
   getrusage(RUSAGE_SELF, &start_ru);
   gettimeofday(&start_wall, NULL);

   tids = calloc(thread_cnt, sizeof(pthread_t));
   if (!tids) {
      err(EX_OSERR, "Allocation failed.");
   }

   t_args = calloc(thread_cnt, sizeof(Thread_arg));
   if (!t_args) {
      err(EX_OSERR, "Allocation failed");
   }

   /* Create all necessary threads, assigning heavy or light loads
      based on the previously obtained remainder. */
   for (i = 0; i < thread_cnt; i++, remainder--) {
      t_args[i].pos = thread_pos;
      t_args[i].load = (remainder > 0) ? heavy_load : light_load;

      pthread_create(&tids[i], NULL, thread_func, &t_args[i]);

      thread_pos += t_args[i].load;
   }

   /* Join all threads, freeing each returned result after storing locally
      and using said result to compute final answer. */
   final_result = (task == 1) ? INT_MIN : 0;

   for (i = 0; i < thread_cnt; i++) {
      status = pthread_join(tids[i], &result);
      tmp = *(int *) result;
      free(result);

      if (!status) {
	 if (task == 1) {
	    if (tmp > final_result) {
	       final_result = tmp;
	    }
	 } else {
	    final_result = (final_result + tmp) % MAX_SUM;
	 }
      } else {
	 err(EX_OSERR, "Thread %ld did not exit properly.", tids[i]);
      }
   }

   /* End timing */
   gettimeofday(&end_wall, NULL);
   getrusage(RUSAGE_SELF, &end_ru);


   if (print_res) {
      if (task == 1) {
         printf("Maximum value: %d\n", final_result);
      } else {
         printf("Sum of elements: %d\n", final_result);
      }
   }

   /* Compute and print elapsed time */
   diff_ru_utime = tv_delta(start_ru.ru_utime, end_ru.ru_utime);
   diff_ru_stime = tv_delta(start_ru.ru_stime, end_ru.ru_stime);
   diff_wall = tv_delta(start_wall, end_wall);

   printf("User time: %ld.%06ld\n", diff_ru_utime.tv_sec, diff_ru_utime.tv_usec);
   printf("System time: %ld.%06ld\n", diff_ru_stime.tv_sec, diff_ru_stime.tv_usec);
   printf("Wall time: %ld.%06ld\n", diff_wall.tv_sec, diff_wall.tv_usec);

   
   free(array);
   free(tids);
   free(t_args);

   return EX_OK;
}

/* Prints usage information for the executable and exits. */
static void exit_with_usage() {
   static const char *usage =
      "Usage: threads <elt_cnt> <thead_cnt> <seed> <task> <print>\n\n"
      "elt_cnt     Number of elements in array\n"
      "thread_cnt  Number of threads in process\n"
      "seed        Seed for generating array elements\n"
      "task        Task to be run: 1 for Max, 2 for Sum\n"
      "print       Print results (Y/N)";
   puts(usage);
   exit(EX_USAGE);
}

/* Using seed, generates an array of pseudorandom numbers
   of the given length. Assumes length > 0. */
static int *generate_array(int length, int seed) {
   int i, *my_array;

   assert(length > 0);
   srand(seed);

   my_array = malloc(length * sizeof(int));
   if (!my_array) {
      err(EX_OSERR, "Allocation failed.");
   }

   for (i = 0; i < length; i++) {
      my_array[i] = rand();
   }

   return my_array;
}

/* Takes a pointer to a Thread_arg and returns the maximum
   value amongst the load number of elements after pos.
   Assumes there is at least 1 element to be processed. */
static void *thread_max(void *args) {
   Thread_arg *t_arg = args;
   int *array = t_arg->pos, length = t_arg->load, i, max, *result;

   max = array[0];
   for (i = 1; i < length; i++) {
      if (array[i] > max) {
         max = array[i];
      }
   }

   result = malloc(sizeof(int));
   *result = max;

   return result;
}

/* Takes a pointer to a Thread_arg and returns the sum of the
   load number of elements after pos (mod 10^6) */
static void *thread_sum(void *args) {
   Thread_arg *t_arg = args;
   int *array = t_arg->pos, length = t_arg->load, i, sum, *result;

   sum = 0;
   for (i = 0; i < length; i++) {
      sum = (sum + array[i]) % MAX_SUM;
   }

   result = malloc(sizeof(int));
   *result = sum;

   return result;
}
