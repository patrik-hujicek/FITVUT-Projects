/*
 * Architektura procesoru (ACH 2018)
 * Projekt c.2 (CUDA)
 * Login: xbolva00
 */

#ifndef __NBODY_H__
#define __NBODY_H__

#include <cstdlib>
#include <cstdio>

/* gravitacni konstanta */
#define G 6.673840e-11f
#define COLLISION_DISTANCE 0.01f

/* struktura castic */
typedef struct
{
  float4 *pos;
  float4 *vel;
} t_particles;

/* pomocna struktura rychlosti */
typedef struct
{
  float4 *coord;
} t_velocities;

__global__ void calculate_velocity(t_particles p_in, t_particles p_out, const int N, const float dt);

void particles_read(FILE *fp, t_particles &p, int N);

void particles_write(FILE *fp, t_particles &p, int N);

#endif /* __NBODY_H__ */
