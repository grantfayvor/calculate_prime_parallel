#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>

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
  static const int N = 10000000;
  static const int RangeSize = 1000;
  const int SIZE = N / RangeSize + 1;

  int n_threads = omp_get_max_threads();

  omp_set_num_threads(n_threads);

  int index[SIZE];

  for (int i = 0; i < SIZE; i++)
  {
    index[i] = i * RangeSize;
  }

  gettimeofday(&start_time, NULL);

  int result[SIZE - 1];

#pragma omp parallel for
  for (int j = 0; j < SIZE - 1; j++)
  {
    result[j] = count_primes(index[j], index[j + 1]);
  }

  int freq = 0;

  for (int i = 0; i < SIZE - 1; i++)
  {
    freq = freq + result[i];
  }

  printf("\n");

  gettimeofday(&end_time, NULL);

  float running_time = calc_milliseconds(start_time, end_time);

  printf("Number of primes in range %d to %d = %d\n", 1, N, freq);
  printf("%9.6f millisecs (%9.6f) secs \n", running_time, (running_time / 1000.0));
  printf("number of threads is = %d \n", n_threads);
}
