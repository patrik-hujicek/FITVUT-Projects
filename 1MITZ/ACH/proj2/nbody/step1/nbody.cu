/*
 * Architektura procesoru (ACH 2018)
 * Projekt c.2 (CUDA)
 * Login: xbolva00
 */

#include <cmath>
#include <cfloat>
#include "nbody.h"

__global__ void calculate_velocity(t_particles p_in, t_particles p_out, const int N, const float dt)
{
	int i = blockDim.x * blockIdx.x + threadIdx.x;

	if (i < N) {
	        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
		float g_dt_inv = G * dt;
	        float p_w_i = p_in.pos[i].w;
	        float p_pos_x_i = p_in.pos[i].x;
	        float p_pos_y_i = p_in.pos[i].y;
	        float p_pos_z_i = p_in.pos[i].z;
	        float p_vel_x_i = p_in.vel[i].x;
	        float p_vel_y_i = p_in.vel[i].y;
	        float p_vel_z_i = p_in.vel[i].z;

		for (int j = 0; j < N; ++j) {
			float dx = p_in.pos[j].x - p_pos_x_i;
			float dy = p_in.pos[j].y - p_pos_y_i;
			float dz = p_in.pos[j].z - p_pos_z_i;

			float s = dx * dx + dy * dy + dz * dz;
			float r = sqrtf(s);

		 	float p_w_j = p_in.pos[j].w;
			float sub_w = p_w_i - p_w_j;
			float add_w = p_w_i + p_w_j;
			float v_j = (r > COLLISION_DISTANCE) ? (g_dt_inv * p_w_j) / (s * r) : 0.0f;

			tx += (r > COLLISION_DISTANCE) ? (dx * v_j) : ((2 * p_w_j * p_in.vel[j].x + sub_w * p_vel_x_i) / add_w) - p_vel_x_i;
			ty += (r > COLLISION_DISTANCE) ? (dy * v_j) : ((2 * p_w_j * p_in.vel[j].y + sub_w * p_vel_y_i) / add_w) - p_vel_y_i;
			tz += (r > COLLISION_DISTANCE) ? (dz * v_j) : ((2 * p_w_j * p_in.vel[j].z + sub_w * p_vel_z_i) / add_w) - p_vel_z_i;

		}

		p_out.vel[i].x = p_vel_x_i + tx;
		p_out.vel[i].y = p_vel_y_i + ty;
		p_out.vel[i].z = p_vel_z_i + tz;

		p_out.pos[i].x = p_out.vel[i].x * dt + p_pos_x_i;
		p_out.pos[i].y = p_out.vel[i].y * dt + p_pos_y_i;
		p_out.pos[i].z = p_out.vel[i].z * dt + p_pos_z_i;

	}
}

__host__ void particles_read(FILE *fp, t_particles &p, int N)
{
  for (int i = 0; i < N; i++)
	{
		fscanf(fp, "%f %f %f %f %f %f %f \n",
				&p.pos[i].x, &p.pos[i].y, &p.pos[i].z,
				&p.vel[i].x, &p.vel[i].y, &p.vel[i].z,
				&p.pos[i].w);
	}
}

__host__  void particles_write(FILE *fp, t_particles &p, int N)
{
  for (int i = 0; i < N; i++)
	{
		fprintf(fp, "%10.10f %10.10f %10.10f %10.10f %10.10f %10.10f %10.10f \n",
				p.pos[i].x, p.pos[i].y, p.pos[i].z,
				p.vel[i].x, p.vel[i].y, p.vel[i].z,
				p.pos[i].w);
	}
}
