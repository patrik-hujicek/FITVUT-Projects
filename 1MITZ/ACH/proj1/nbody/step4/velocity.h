/**
 * Architektura procesoru (ACH 2018)
 * Projekt c. 1 (nbody)
 * Login: xbolva00
 */

#ifndef __VELOCITY_H__
#define __VELOCITY_H__

#include <cstdlib>
#include <cstdio>

/* gravitacni konstanta */
constexpr float G = 6.67384e-11f;

/* collision threshold */
constexpr float COLLISION_DISTANCE = 0.01f;

/* struktura castice (hmotneho bodu) */
struct t_particle
{
    float pos_x;
    float pos_y;
    float pos_z;
    float vel_x;
    float vel_y;
    float vel_z;
    float weight;
};

/* vektor zmeny rychlosti */
struct t_velocity
{
    float x;
    float y;
    float z;
};

#pragma omp declare simd linear(velocity_x, velocity_y, velocity_z) uniform (p2_pos_x, p2_pos_y, p2_pos_z, p2_vel_x, p2_vel_y, p2_vel_z, p2_weight)
void calculate_all_velocities(float p1_pos_x, float p1_pos_y, float p1_pos_z, float p1_vel_x, float p1_vel_y, float p1_vel_z, float p1_weight, float p2_pos_x, float p2_pos_y, float p2_pos_z, float p2_vel_x, float p2_vel_y, float p2_vel_z, float p2_weight, float &velocity_x, float &velocity_y, float &velocity_z);
// #pragma omp declare simd linear(p1, vel) uniform(p2)
// void calculate_collision_velocity(const t_particle &p1, const t_particle &p2, t_velocity &vel);

#endif /* __VELOCITY_H__ */
