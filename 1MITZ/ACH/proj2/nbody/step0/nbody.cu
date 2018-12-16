/*
 * Architektura procesoru (ACH 2018)
 * Projekt c.2 (CUDA)
 * Login: xbolva00
 */

#include <cmath>
#include <cfloat>
#include "nbody.h"

__global__ void calculate_gravitation_velocity(t_particles p, t_velocities tmp_vel, int N, float dt)
{
  int i = blockDim.x * blockIdx.x + threadIdx.x;

  if (i < N) {
		float tx, ty, tz;
		tx = ty = tz = 0.0f;
		float g_dt_inv = G * dt;
	        float p_pos_x_i = p.pos[i].x;
	        float p_pos_y_i = p.pos[i].y;
	        float p_pos_z_i = p.pos[i].z;
		for (int j = 0; j < N; ++j) {
			float dx = p.pos[j].x - p_pos_x_i;
			float dy = p.pos[j].y - p_pos_y_i;
			float dz = p.pos[j].z - p_pos_z_i;
			float s = dx*dx + dy*dy + dz*dz;
			float r = sqrtf(s);
			bool ok = r > COLLISION_DISTANCE;
			float vel = (ok) ? (g_dt_inv * p.pos[j].w) / (s * r)  : 0.0f;
			
			tx += (ok) ? dx * vel : 0.0f;
			ty += (ok) ? dy * vel : 0.0f;
			tz += (ok) ? dz * vel : 0.0f;
		}

		tmp_vel.coord[i].x += tx;
		tmp_vel.coord[i].y += ty;
		tmp_vel.coord[i].z += tz;

   }
}

__global__ void calculate_collision_velocity(t_particles p, t_velocities tmp_vel, int N, float dt)
{
  int i = blockDim.x * blockIdx.x + threadIdx.x;

  if (i < N) {
		float tx, ty, tz;
		tx = ty = tz = 0.0f;
	        float p_w_i = p.pos[i].w;
	        float p_pos_x_i = p.pos[i].x;
	        float p_pos_y_i = p.pos[i].y;
	        float p_pos_z_i = p.pos[i].z;
	        float p_vel_x_i = p.vel[i].x;
	        float p_vel_y_i = p.vel[i].y;
	        float p_vel_z_i = p.vel[i].z;
		for (int j = 0; j < N; ++j) {
			float dx = p_pos_x_i - p.pos[j].x;
			float dy = p_pos_y_i - p.pos[j].y;
			float dz = p_pos_z_i - p.pos[j].z;
			float s = dx*dx + dy*dy + dz*dz;
			float r = sqrtf(s);
			
			float p_w_j = p.pos[j].w;
			float subw = p_w_i - p_w_j;
			float addw = p_w_i + p_w_j;
			float p_2w_j = 2 * p_w_j;
			bool ok = r > 0.0f && r < COLLISION_DISTANCE;

			tx += (ok) ? ((p_2w_j * p.vel[j].x + subw * p_vel_x_i) / addw) - p_vel_x_i : 0.0f;
			ty += (ok) ? ((p_2w_j * p.vel[j].y + subw * p_vel_y_i) / addw) - p_vel_y_i : 0.0f;
			tz += (ok) ? ((p_2w_j * p.vel[j].z + subw * p_vel_z_i) / addw) - p_vel_z_i : 0.0f;
		}

		tmp_vel.coord[i].x += tx;
		tmp_vel.coord[i].y += ty;
		tmp_vel.coord[i].z += tz;

	}
}

__global__ void update_particle(t_particles p, t_velocities tmp_vel, int N, float dt)
{
  int i = blockDim.x * blockIdx.x + threadIdx.x;

  if (i < N) {
		p.vel[i].x += tmp_vel.coord[i].x;
		p.vel[i].y += tmp_vel.coord[i].y;
		p.vel[i].z += tmp_vel.coord[i].z;

		p.pos[i].x += p.vel[i].x * dt;
		p.pos[i].y += p.vel[i].y * dt;
		p.pos[i].z += p.vel[i].z * dt;

		tmp_vel.coord[i].x = tmp_vel.coord[i].y = tmp_vel.coord[i].z = 0.0f;
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
