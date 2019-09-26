/*
 * Project: Determining levels of the binary tree nodes
 * Author: David Bolvansky (xbolva00)
 */

#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 //#define MEASURE_TIME

const int INVALID_NODE = -1;

typedef unsigned char byte_t;

/* Represents edge (u,v) */
struct edge {
  int from; /* u */
  int to;   /* v */
};

static int get_parent_node(int rank) { return (rank - 1) / 2; }

static int is_root_node(int rank) { return rank == 0; }

static struct edge reverse_edge(struct edge e) { return {e.to, e.from}; }

static int edge_to_rank(struct edge e, size_t edges_count) {
  return e.from < e.to ? e.to - 1 : e.from - 1 + edges_count / 2;
}

static bool is_forward_edge(int rank, size_t edges_count) {
  return rank + 1 <= (int)edges_count / 2;
}

static struct edge rank_to_edge(int rank, size_t edges_count) {
  bool reverse_edge = !is_forward_edge(rank, edges_count);
  int to = reverse_edge ? rank + 1 - (edges_count / 2) : rank + 1;
  int from = get_parent_node(to);
  return reverse_edge ? edge{to, from} : edge{from, to};
}

static struct edge compute_euler_tour(struct edge rev_edge,
                                      size_t nodes_count) {
  int node = rev_edge.from + 1;                    /* Tree index */
  int adj = rev_edge.to + 1;                       /* Tree index */
  int parent = node == 1 /* root */ ? INVALID_NODE /* parent does not exists */
                                    : get_parent_node(rev_edge.from) + 1;
  const int ADJS_N = 4;
  /* Array of adjacents for node: parent, left and right son nodes. Use invalid
   * node as sentinel for simplier logic to find the next adjacent of the node.
   */
  int adjs[ADJS_N] = {parent, 2 * node, 2 * node + 1, INVALID_NODE};

  int next_adj = INVALID_NODE;
  for (int i = 0; i < ADJS_N; ++i) {
    if (adjs[i] ==

            adj &&
        adjs[i + 1] <= (int)nodes_count) {
      next_adj = adjs[i + 1];
      break;
    }
  }

  /* Rooting edge case, stop at the root. */
  if (next_adj == INVALID_NODE && adjs[0] == INVALID_NODE) {
    next_adj = node;
    node = adj;
  }

  /* Next adjacent was not found. Use the first adjacent of the node. */
  if (next_adj == INVALID_NODE)
    next_adj = adjs[0];

  /* Return computed edge */
  return {node - 1, next_adj - 1};
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank;
  int procs;
#ifdef MEASURE_TIME
  double start = 0.0;
#endif
  /* Get the number of processors */
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  /* Get the rank/ID of the current processor */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  size_t edges_count = procs;
  /* Compute the number of nodes */
  size_t nodes_count = (edges_count + 2) / 2;
  byte_t *nodes = (byte_t *)argv[1];
  unsigned levels[nodes_count];

#ifdef MEASURE_TIME
  if (is_root_node(rank))
    start = MPI_Wtime();
#endif

  /* Handle special case when the tree has only root node */
  if (nodes_count == 1) {
    printf("%c:0\n", nodes[0]);
    MPI_Finalize();
    return 0;
  }

  /* Get a edge for this rank */
  struct edge e = rank_to_edge(rank, edges_count);
  /* Get a reversed edge */
  struct edge rev_edge = reverse_edge(e);
  /* Compute euler tour for this edge */
  struct edge next_e = compute_euler_tour(rev_edge, nodes_count);

  /* Get my successor */
  int succ = edge_to_rank(next_e, edges_count);
  /* Adjust succ for rooting edge */
  succ = memcmp(&e, &next_e, sizeof(struct edge)) == 0 ? 0 : succ;
  /* Send my rank to my succ */
  MPI_Send(&rank, 1, MPI_INT, succ, 0, MPI_COMM_WORLD);
  int pred;
  /* Get rank of my pred */
  MPI_Recv(&pred, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, NULL);

  /* Set weights for edges */
  int weight = is_forward_edge(rank, edges_count) ? -1 : 1;

  /* Mark invalid nodes */
  if (succ == 0)
    succ = INVALID_NODE;
  if (is_root_node(rank))
    pred = INVALID_NODE;

  /* Do suffix sum */
  int iter_data[2];
  for (int i = 0, e = ceil(log2(edges_count)); i < e; ++i) {
    if (succ != INVALID_NODE) {
      /* Send my predecessor and rank to my successor (if any) */
      iter_data[0] = pred;
      iter_data[1] = rank;
      MPI_Send(iter_data, 2, MPI_INT, succ, 0, MPI_COMM_WORLD);
    }

    if (pred != INVALID_NODE) {
      /* Receive predecessor and rank from my predecessor (if any) */
      MPI_Recv(iter_data, 2, MPI_INT, pred, 0, MPI_COMM_WORLD, NULL);
      int pred_rank = iter_data[1];
      pred = iter_data[0];
      iter_data[0] = succ;
      iter_data[1] = weight;
      /* Send updated data to predecessor of my predecessor */
      MPI_Send(iter_data, 2, MPI_INT, pred_rank, 0, MPI_COMM_WORLD);
    }

    if (succ != INVALID_NODE) {
      /* Receive predecessor and weight from my succesor (if any) */
      MPI_Recv(iter_data, 2, MPI_INT, succ, 0, MPI_COMM_WORLD, NULL);
      succ = iter_data[0];
      /* Adjust weight */
      weight += iter_data[1];
    }

    // Synchronization barrier
    MPI_Barrier(MPI_COMM_WORLD);
  }

  /* Adjust weights for forward edges to get correct levels */
  if (is_forward_edge(rank, edges_count)) {
    int arr[2] = {weight + 1, e.to /* v */};
    MPI_Send(arr, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  /* Receive computed levels of the nodes */
  if (is_root_node(rank)) {
#ifdef MEASURE_TIME
    double end = MPI_Wtime();
    printf("time: %lu ns\n", (size_t)((end - start) * 1000000000));
#endif
    int arr[2];
    for (size_t i = 0, e = edges_count / 2; i < e; ++i) {
      MPI_Recv(arr, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, NULL);
      levels[arr[1] /* node */] = arr[0] /* level */;
    }
    /* Set level of the root */
    levels[0] = 0;

    /* Print level per node */
    for (size_t i = 0; i < nodes_count; ++i) {
      printf((i < nodes_count - 1) ? "%c:%d," : "%c:%d\n", nodes[i], levels[i]);
    }
  }

  MPI_Finalize();
  return 0;
}