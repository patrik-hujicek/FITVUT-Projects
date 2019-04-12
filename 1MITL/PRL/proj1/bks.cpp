/*
* Project: Parallel bucket sort
* Author: David Bolvansky (xbolva00)
*/

#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#define TO_TREE_INDEX(x) (x + 1)
#define GET_TREE_INDEX(x) (x - 1)
#define IS_LEAF_NODE(x, p) (TO_TREE_INDEX(x) * 2 > p)
#define IS_ROOT_NODE(x) (x == 0)
#define PARENT_NODE(x) ((x - 1) / 2)
#define GET_TREE_LEVEL(x) (floor(log2(x)))
#define NEXT_POWER_2(x) ((1ULL << sizeof(x) * CHAR_BIT) >> __builtin_clz(x - 1))

// #define MEASURE_TIME

typedef unsigned char byte_t;

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank;
  int procs;
  size_t n = 0, n_m = 0;
  size_t fill_count = 0;
  byte_t *numbers = NULL;
#ifdef MEASURE_TIME
  double start = 0.0;
#endif
  /* Get the number of processors */
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  /* Get the rank/ID of the current processor */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (IS_ROOT_NODE(rank)) {
    FILE *input = fopen("numbers", "rb");
    fseek(input, 0L, SEEK_END);
    /* Get length of the file */
    size_t count = ftell(input);
    fseek(input, 0L, SEEK_SET);

    numbers = (byte_t *)malloc(sizeof(byte_t) * count);
    /* Read input sequence from the file */
    n = fread(numbers, sizeof(byte_t), count, input);
    fclose(input);

    size_t m = NEXT_POWER_2((procs + 1) / 2);
    n_m = ceil((double)n / m);
    /* Adjust input sequence count to deal with problematic tree cases */
    size_t new_n = n_m * m;
    /* Number of required zeroes to fill input sequence */
    fill_count = new_n - n;
    numbers = (byte_t *)realloc(numbers, sizeof(byte_t) * new_n);
    memset(numbers + n, 0, sizeof(byte_t) * fill_count);
    for (size_t i = 0; i < n; ++i)
      printf((i < n - 1) ? "%u " : "%u\n", numbers[i]);

    /* Root handles special cases */
    if (n == 1 || n == 2) {
      if (n == 1)
        printf("%u\n", numbers[0]);
      if (n == 2) {
        if (numbers[0] < numbers[1])
          printf("%u\n%u\n", numbers[0], numbers[1]);
        else
          printf("%u\n%u\n", numbers[1], numbers[0]);
      }
      free(numbers);
      MPI_Finalize();
      return 0;
    }

    n = new_n;
  }

  /* Broadcast variables which are required for futher computing */
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&n_m, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&fill_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (IS_ROOT_NODE(rank)) {
#ifdef MEASURE_TIME
    start = MPI_Wtime();
#endif
    // Send buckets to list processors
    for (int i = 0, e = n / n_m; i < e; ++i) {
      MPI_Send(numbers + i * n_m, n_m, MPI_BYTE, procs - i - 1, 0,
               MPI_COMM_WORLD);
    }
  }

  if (IS_LEAF_NODE(rank, procs)) {
    /* Leaf processors reads own bucket and sorts it */
    byte_t *arr = (byte_t *)malloc(sizeof(byte_t) * n_m);
    MPI_Recv(arr, n_m, MPI_BYTE, 0, 0, MPI_COMM_WORLD, NULL);
    std::sort(arr, arr + n_m);
    /* Send sorted bucket to the parent node */
    MPI_Send(arr, n_m, MPI_BYTE, PARENT_NODE(rank), 0, MPI_COMM_WORLD);
    free(arr);
  } else {
    size_t left_tree_index = TO_TREE_INDEX(rank) * 2;
    /* Get current depth level and compute number of nodes in this level */
    size_t current_depth = GET_TREE_LEVEL(left_tree_index);
    size_t nodes_in_level_count = 1 << current_depth;
    /* Compute how many numbers will be processed by a node */
    size_t range_per_node = n / nodes_in_level_count;
    size_t merge_range = range_per_node * 2;

    byte_t *arr = (byte_t *)malloc(sizeof(byte_t) * (range_per_node * 4));
    byte_t *left = arr;
    byte_t *right = arr + range_per_node;
    byte_t *merged = arr + merge_range;

    /* Receive number sequences from son nodes */
    MPI_Recv(left, range_per_node, MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
             NULL);
    MPI_Recv(right, range_per_node, MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
             NULL);
    /* Merges two number sequences into the one */
    std::merge(left, left + range_per_node, right, right + range_per_node,
               merged);

    if (IS_ROOT_NODE(rank)) {
#ifdef MEASURE_TIME
      double end = MPI_Wtime();
      printf("time: %lu ns\n", (size_t)((end - start) * 1000000000));
#endif
      for (size_t i = fill_count; i < n; ++i)
        printf("%u\n", merged[i]);
    } else {
      /* Send merged sequence futher to the parent node */
      MPI_Send(merged, merge_range, MPI_BYTE, PARENT_NODE(rank), 0,
               MPI_COMM_WORLD);
    }

    free(arr);
  }

  if (IS_ROOT_NODE(rank))
    free(numbers);

  MPI_Finalize();
  return 0;
}