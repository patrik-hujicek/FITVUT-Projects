/**
 * Architektura procesoru (ACH 2018)
 * Projekt c. 1 (nbody)
 * Login: xbolva00
 */

#ifndef __NBODY_H__
#define __NBODY_H__

#include <cstdlib>
#include <cstdio>
#include "velocity.h"

// using t_particles   = t_particle[N];
// using t_velocities  = t_velocity[N];

struct t_particles
{
    __declspec(align(64)) float pos_x[N];
    __declspec(align(64)) float pos_y[N];
    __declspec(align(64)) float pos_z[N];
    __declspec(align(64)) float vel_x[N];
    __declspec(align(64)) float vel_y[N];
    __declspec(align(64)) float vel_z[N];
    __declspec(align(64)) float weight[N];
};

struct t_velocities
{
    __declspec(align(64)) float x[N];
    __declspec(align(64)) float y[N];
    __declspec(align(64)) float z[N];
};

void particles_simulate(t_particles &p);

void particles_read(FILE *fp, t_particles &p);

void particles_write(FILE *fp, t_particles &p);

#endif /* __NBODY_H__ */
