/**
 * Architektura procesoru (ACH 2018)
 * Projekt c. 1 (nbody)
 * Login: xbolva00
 */

#include <cmath>
#include "nbody.h"

void particles_simulate(t_particles &p)
{
    
    int i, j, k;

    t_velocities velocities;
    __assume_aligned(&p.pos_x, 64);
    __assume_aligned(&p.pos_y, 64);
    __assume_aligned(&p.pos_z, 64);
    __assume_aligned(&p.vel_x, 64);
    __assume_aligned(&p.vel_y, 64);
    __assume_aligned(&p.vel_z, 64);
    __assume_aligned(&p.weight, 64);
    __assume_aligned(&velocities.x, 64);
    __assume_aligned(&velocities.y, 64);
    __assume_aligned(&velocities.z, 64);

    for (k = 0; k < STEPS; k++)
    {
            //vynulovani mezisouctu
        for (i = 0; i < N; i++)
        {
            velocities.x[i] = 0.0f;
            velocities.y[i] = 0.0f;
            velocities.z[i] = 0.0f;
        }
            //vypocet nove rychlosti
        for (i = 0; i < N; i++)
        {
            __assume_aligned(&p, 64);
	    float r, dx, dy, dz;
	    float vx, vy, vz;
            #pragma omp simd
            for (j = 0; j < N; j++)
            {
    	    dx = p.pos_x[i] - p.pos_x[j];
	    dy = p.pos_y[i] - p.pos_y[j];
	    dz = p.pos_z[i] - p.pos_z[j];

	    r = sqrt(dx*dx + dy*dy + dz*dz);

	    if (r > COLLISION_DISTANCE) {
		  float force = (G * p.weight[j] * p.weight[i]) / (r*r);
		  float velocity = ((force / p.weight[j]) * DT) / r;
		  
		  velocities.x[j] += dx * velocity;
		  velocities.y[j] += dy * velocity;
		  velocities.z[j] += dz * velocity;
	    	
	    } else if (r > 0.0f && r < COLLISION_DISTANCE) {
		  float dw = p.weight[j] - p.weight[i];
		  float sumw = p.weight[j] + p.weight[i];
		    
		  velocities.x[j] += ((2 * p.weight[i] * p.vel_x[i] + dw * p.vel_x[j]) / sumw) - p.vel_x[j];
		  velocities.y[j] += ((2 * p.weight[i] * p.vel_y[i] + dw * p.vel_y[j]) / sumw) - p.vel_y[j];
		  velocities.z[j] += ((2 * p.weight[i] * p.vel_z[i] + dw * p.vel_z[j]) / sumw) - p.vel_z[j];
	    }
            } // inner loop
        }
            //ulozeni rychlosti a posun castic
        for (i = 0; i < N; i++)
        {
            p.vel_x[i] += velocities.x[i];
            p.vel_y[i] += velocities.y[i];
            p.vel_z[i] += velocities.z[i];
            
            p.pos_x[i] += p.vel_x[i] * DT;
            p.pos_y[i] += p.vel_y[i] * DT;
            p.pos_z[i] += p.vel_z[i] * DT;
        }
    }
}


void particles_read(FILE *fp, t_particles &p)
{
    for (int i = 0; i < N; i++)
    {
        fscanf(fp, "%f %f %f %f %f %f %f \n",
            &p.pos_x[i], &p.pos_y[i], &p.pos_z[i],
            &p.vel_x[i], &p.vel_y[i], &p.vel_z[i],
            &p.weight[i]);
    }
}

void particles_write(FILE *fp, t_particles &p)
{
    for (int i = 0; i < N; i++)
    {
        fprintf(fp, "%10.10f %10.10f %10.10f %10.10f %10.10f %10.10f %10.10f \n",
            p.pos_x[i], p.pos_y[i], p.pos_z[i],
            p.vel_x[i], p.vel_y[i], p.vel_z[i],
            p.weight[i]);
    }
}
