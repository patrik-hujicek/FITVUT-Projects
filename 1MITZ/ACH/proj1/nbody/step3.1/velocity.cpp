/**
 * Architektura procesoru (ACH 2018)
 * Projekt c. 1 (nbody)
 * Login: xbolva00
 */

#include <cmath>
#include <cfloat>
#include "velocity.h"

void calculate_all_velocities(
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

    if (r > COLLISION_DISTANCE) {
	  float force = (G * p1.weight * p2.weight) / (r*r);
	  float velocity = ((force / p1.weight) * DT) / r;
	  
	  vel.x += dx * velocity;
	  vel.y += dy * velocity;
	  vel.z += dz * velocity;
    	
    } else if (r > 0.0f && r < COLLISION_DISTANCE) {
	  float dw = p1.weight - p2.weight;
	  float sumw = p1.weight + p2.weight;
	    
	  vel.x += ((2 * p2.weight * p2.vel_x + dw * p1.vel_x) / sumw) - p1.vel_x;
	  vel.y += ((2 * p2.weight * p2.vel_y + dw * p1.vel_y) / sumw) - p1.vel_y;
	  vel.z += ((2 * p2.weight * p2.vel_z + dw * p1.vel_z) / sumw) - p1.vel_z;
    }
}
