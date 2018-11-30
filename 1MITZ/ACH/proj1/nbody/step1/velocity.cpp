/**
 * Architektura procesoru (ACH 2018)
 * Projekt c. 1 (nbody)
 * Login: xbolva00
 */

#include <cmath>
#include <cfloat>
#include "velocity.h"

/**
 * @brief   Funkce vypocte rychlost kterou teleso p1 ziska vlivem gravitace p2.
 * @details Viz rovnice 4.4 v souboru zadani.pdf 
 */
void calculate_gravitation_velocity(
  const t_particle &p1,
  const t_particle &p2,
  t_velocity &vel
)
{
    float r, dx, dy, dz;
    float vx, vy, vz;
    
    dx = p2.pos_x - p1.pos_x;
    dy = p2.pos_y - p1.pos_y;
    dz = p2.pos_z - p1.pos_z;

    r = sqrt(dx*dx + dy*dy + dz*dz);

    /* MISTO PRO VAS KOD GRAVITACE */
    float force = (G * p1.weight * p2.weight) / (r*r);
    float velocity = ((force / p1.weight) * DT) / r;
    vx = dx * velocity;
    vy = dy * velocity;
    vz = dz * velocity;

    /* KONEC */

    vel.x += (r > COLLISION_DISTANCE) ? vx : 0.0f;
    vel.y += (r > COLLISION_DISTANCE) ? vy : 0.0f;
    vel.z += (r > COLLISION_DISTANCE) ? vz : 0.0f;
}

/**
 * @brief   Funkce vypocte rozdil mezi rychlostmi pred a po kolizi telesa p1 do telesa p2.
 * @details Viz rovnice 4.8 v souboru zadani.pdf
 */

void calculate_collision_velocity(
  const t_particle &p1,
  const t_particle &p2,
  t_velocity &vel
)
{
    float r, dx, dy, dz;
    float vx, vy, vz;

    dx = p2.pos_x - p1.pos_x;
    dy = p2.pos_y - p1.pos_y;
    dz = p2.pos_z - p1.pos_z;

    r = sqrt(dx*dx + dy*dy + dz*dz);

    /* MISTO PRO VAS KOD KOLIZE */

    float dw = p1.weight - p2.weight;
    float sumw = p1.weight + p2.weight;
    
    vx = ((2 * p2.weight * p2.vel_x + dw * p1.vel_x) / sumw) - p1.vel_x;
    vy = ((2 * p2.weight * p2.vel_y + dw * p1.vel_y) / sumw) - p1.vel_y;
    vz = ((2 * p2.weight * p2.vel_z + dw * p1.vel_z) / sumw) - p1.vel_z;

    /* KONEC */

    // jedna se o rozdilne ale blizke prvky
    if (r > 0.0f && r < COLLISION_DISTANCE) {
        vel.x += vx;
        vel.y += vy;
        vel.z += vz;
    }
}
