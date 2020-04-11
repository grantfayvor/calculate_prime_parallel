#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>

#define max_rows 100000
#define send_data_tag 2001
#define return_data_tag 2002

static bool isPrime(int n)
{
  bool prime = true;

  if (n < 2)
  {
    prime = false;
  }

  for (int k = 2; k <= (int)sqrt(n) && prime; k++)
  {

    if (n % k == 0)
    {
      prime = false;
    }
  }

  return prime;
}

int count_primes(int lb, int ub)
{
  int freq = 0;
  for (int k = lb; k < ub; k = k + 1)
  {
    if (isPrime(k))
    {
      freq++;
    }
  }
  return freq;
}

struct timeval start_time, end_time;

float calc_milliseconds(struct timeval start, struct timeval stop)
{
  return ((double)(stop.tv_usec - start.tv_usec) / 100000 + (double)(stop.tv_sec - start.tv_sec) * 1000);
}

int main(int argc, char *argv[])
{
  int n_threads = 4;
  int p_threads;
  int errcodes;
  MPI_Status status;
  MPI_Info info;
  int root;
  MPI_Comm comm;
  MPI_Comm *intercomm;
  const char *command;

  int ierr = MPI_Init(&argc, &argv);

  int my_id, root_process = 0, num_rows_to_receive, sender, num_rows_received, num_rows_to_send;

  static const int N = 10000000;
  static const int RangeSize = 1000;
  const int SIZE = N / RangeSize + 1;

  int index[SIZE];

  int portion_count = floor(SIZE / n_threads);

  gettimeofday(&start_time, NULL);

  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
  ierr = MPI_Comm_size(MPI_COMM_WORLD, &n_threads);

  if (my_id == root_process)
  {
    int result = 0;

    for (int i = 0; i < SIZE; i++)
    {
      index[i] = i * RangeSize;
    }

    int sent = 0;

    for (int proc_id = 1; proc_id < n_threads; proc_id++)
    {
      int start_row = proc_id * portion_count + 1;
      int end_row = (proc_id + 1) * portion_count;

      if ((SIZE - end_row) < portion_count)
      {
        end_row = SIZE - 1;
      }

      num_rows_to_send = end_row - start_row + 2;

      sent += num_rows_to_send;

      ierr = MPI_Send(&num_rows_to_send, 1, MPI_INT, proc_id, send_data_tag, MPI_COMM_WORLD);

      ierr = MPI_Send(&index[start_row], num_rows_to_send, MPI_INT, proc_id, send_data_tag, MPI_COMM_WORLD);
    }

    int index_size = sizeof index / sizeof index[0];

    for (int i = 0; i < portion_count + 1; i++)
    {
      int begin = index[i];
      int stop = index[i + 1];
      result += count_primes(begin, stop);
    }

    for (int proc_id = 1; proc_id < n_threads; proc_id++)
    {

      int slave_result = 0;

      ierr = MPI_Recv(&slave_result, 1, MPI_LONG, MPI_ANY_SOURCE, return_data_tag, MPI_COMM_WORLD, &status);

      result += slave_result;
    }

    printf("\n");

    gettimeofday(&end_time, NULL);

    float running_time = calc_milliseconds(start_time, end_time);

    printf("Number of primes in range %d to %d = %d\n", 1, N, result);
    printf("%9.6f millisecs (%9.6f) secs \n", running_time, (running_time / 1000.0));
    printf("number of threads is = %d \n", n_threads);
  }
  else
  {

    int local_index[SIZE];

    ierr = MPI_Recv(&num_rows_to_receive, 1, MPI_INT, root_process, send_data_tag, MPI_COMM_WORLD, &status);

    ierr = MPI_Recv(&local_index, num_rows_to_receive, MPI_INT, root_process, send_data_tag, MPI_COMM_WORLD, &status);

    num_rows_received = num_rows_to_receive;

    int prime_count = 0;
    int local_index_size = sizeof local_index / sizeof local_index[0];

    for (int i = 0; i < num_rows_received - 1; i++)
    {
      int begin = local_index[i];
      int stop = local_index[i + 1];
      prime_count += count_primes(begin, stop);
    }

    ierr = MPI_Send(&prime_count, 1, MPI_LONG, root_process, return_data_tag, MPI_COMM_WORLD);
  }

  ierr = MPI_Finalize();
}