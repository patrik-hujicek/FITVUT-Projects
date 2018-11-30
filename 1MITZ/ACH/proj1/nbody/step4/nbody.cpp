/**
 * Architektura procesoru (ACH 2018)
 * Projekt c. 1 (nbody)
 * Login: xbolva00
 */

#include "nbody.h"
#include <cmath>

void particles_simulate(t_particles &p) {

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

  for (k = 0; k < STEPS; k++) {
    // vynulovani mezisouctu
    for (i = 0; i < N; i++) {
      velocities.x[i] = 0.0f;
      velocities.y[i] = 0.0f;
      velocities.z[i] = 0.0f;
    }
    // vypocet nove rychlosti
    for (i = 0; i < N; i++) {
      float tx = 0.0f, ty = 0.0f, tz = 0.0f;
      float p_w_i = p.weight[i];
      float p_2w_i = 2 * p_w_i;
      float p_pos_x_i = p.pos_x[i];
      float p_pos_y_i = p.pos_y[i];
      float p_pos_z_i = p.pos_z[i];
      float p_vel_x_i = p.vel_x[i];
      float p_vel_y_i = p.vel_y[i];
      float p_vel_z_i = p.vel_z[i];
#pragma omp simd reduction(+ : tx, ty, tz) 
      for (j = i + 1; j < N; j++) {
        float dx = p_pos_x_i - p.pos_x[j];
        float dy = p_pos_y_i - p.pos_y[j];
        float dz = p_pos_z_i - p.pos_z[j];

        float p_vel_x_j = p.vel_x[j];
        float p_vel_y_j = p.vel_y[j];
        float p_vel_z_j = p.vel_z[j];

        float s = dx * dx + dy * dy + dz * dz;
        float r = sqrt(s);

	float p_w_j = p.weight[j];
        float p_2w_j = 2 * p_w_j;

        float gdt_r3 = (r > COLLISION_DISTANCE) ? (G * DT) / (s * r) : 0.0f;

        float v_i = p_w_i * gdt_r3;
 	float v_j = p_w_j * gdt_r3;

        float sub_w = p_w_j - p_w_i;
        float sub_w_rev = p_w_i - p_w_j;
        float add_w = p_w_j + p_w_i;

        velocities.x[j] += (r > COLLISION_DISTANCE) ? (dx * v_i) : ((p_2w_i * p_vel_x_i + sub_w * p_vel_x_j) / add_w) - p_vel_x_j;
        velocities.y[j] += (r > COLLISION_DISTANCE) ? (dy * v_i) : ((p_2w_i * p_vel_y_i + sub_w * p_vel_y_j) / add_w) - p_vel_y_j;
        velocities.z[j] += (r > COLLISION_DISTANCE) ? (dz * v_i) : ((p_2w_i * p_vel_z_i + sub_w * p_vel_z_j) / add_w) - p_vel_z_j;

        tx += (r > COLLISION_DISTANCE) ? (-dx * v_j) : ((p_2w_j * p_vel_x_j + sub_w_rev * p_vel_x_i) / add_w) - p_vel_x_i;
        ty += (r > COLLISION_DISTANCE) ? (-dy * v_j) : ((p_2w_j * p_vel_y_j + sub_w_rev * p_vel_y_i) / add_w) - p_vel_y_i;
        tz += (r > COLLISION_DISTANCE) ? (-dz * v_j) : ((p_2w_j * p_vel_z_j + sub_w_rev * p_vel_z_i) / add_w) - p_vel_z_i;
      }

      velocities.x[i] += tx;
      velocities.y[i] += ty;
      velocities.z[i] += tz;

    }
      // ulozeni rychlosti a posun castic
    for (i = 0; i < N; i++) {
      p.vel_x[i] += velocities.x[i];
      p.vel_y[i] += velocities.y[i];
      p.vel_z[i] += velocities.z[i];

      p.pos_x[i] += p.vel_x[i] * DT;
      p.pos_y[i] += p.vel_y[i] * DT;
      p.pos_z[i] += p.vel_z[i] * DT;
    }
  }
}

void particles_read(FILE *fp, t_particles &p) {
  for (int i = 0; i < N; i++) {
    fscanf(fp, "%f %f %f %f %f %f %f \n", &p.pos_x[i], &p.pos_y[i], &p.pos_z[i],
           &p.vel_x[i], &p.vel_y[i], &p.vel_z[i], &p.weight[i]);
  }
}

void particles_write(FILE *fp, t_particles &p) {
  for (int i = 0; i < N; i++) {
    fprintf(fp, "%10.10f %10.10f %10.10f %10.10f %10.10f %10.10f %10.10f \n",
            p.pos_x[i], p.pos_y[i], p.pos_z[i], p.vel_x[i], p.vel_y[i],
            p.vel_z[i], p.weight[i]);
  }
}
