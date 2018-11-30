/**
 * Architektura procesoru (ACH 2018)
 * Projekt c. 1 (nbody)
 * Login: xbolva00
 */

#include <cmath>
#include <cfloat>
#include "velocity.h"

void calculate_all_velocities(float p1_pos_x, float p1_pos_y, float p1_pos_z, float p1_vel_x, float p1_vel_y, float p1_vel_z, float p1_weight, float p2_pos_x, float p2_pos_y, float p2_pos_z, float p2_vel_x, float p2_vel_y, float p2_vel_z, float p2_weight, float &velocity_x, float &velocity_y, float &velocity_z)
{
    float r, dx, dy, dz;
    float vx, vy, vz;
    
    dx = p2_pos_x - p1_pos_x;
    dy = p2_pos_y - p1_pos_y;
    dz = p2_pos_z - p1_pos_z;

    r = sqrt(dx*dx + dy*dy + dz*dz);

    if (r > COLLISION_DISTANCE) {
	  float force = (G * p1_weight * p2_weight) / (r*r);
	  float velocity = ((force / p1_weight) * DT) / r;
	  
	  velocity_x += dx * velocity;
	  velocity_y += dy * velocity;
	  velocity_z += dz * velocity;
    	
    } else if (r > 0.0f && r < COLLISION_DISTANCE) {
	  float dw = p1_weight - p2_weight;
	  float sumw = p1_weight + p2_weight;
	    
	  velocity_x += ((2 * p2_weight * p2_vel_x + dw * p1_vel_x) / sumw) - p1_vel_x;
	  velocity_y += ((2 * p2_weight * p2_vel_y + dw * p1_vel_y) / sumw) - p1_vel_y;
	  velocity_z += ((2 * p2_weight * p2_vel_z + dw * p1_vel_z) / sumw) - p1_vel_z;
    }
}
