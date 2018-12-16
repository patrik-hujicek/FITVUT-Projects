/*
 * Architektura procesoru (ACH 2018)
 * Projekt c.2 (CUDA)
 * Login: xbolva00
 */

#include <sys/time.h>
#include <cstdio>
#include <cmath>
#include <algorithm>

#include "nbody.h"

int main(int argc, char **argv)
{
    FILE *fp;
    struct timeval t1, t2;
    int N;
    float dt;
    int steps;
    int thr_blc;

    // parametry
    if (argc != 7)
    {
        printf("Usage: nbody <N> <dt> <steps> <thr/blc> <input> <output>\n");
        exit(1);
    }
    N = atoi(argv[1]);
    dt = atof(argv[2]);
    steps = atoi(argv[3]);
    thr_blc = atoi(argv[4]);

    printf("N: %d\n", N);
    printf("dt: %f\n", dt);
    printf("steps: %d\n", steps);
    printf("threads/block: %d\n", thr_blc);

    // alokace pameti na CPU
    t_particles particles_cpu;
    const size_t N_float4 = N * sizeof(float4);
    cudaHostAlloc(&particles_cpu.pos, N_float4, cudaHostAllocDefault);
    cudaHostAlloc(&particles_cpu.vel, N_float4, cudaHostAllocDefault);

    // nacteni castic ze souboru
    fp = fopen(argv[5], "r");
    if (fp == NULL)
    {
        printf("Can't open file %s!\n", argv[5]);
        exit(1);
    }
    particles_read(fp, particles_cpu, N);
    fclose(fp);

    t_particles particles_gpu;
    t_particles particles2_gpu;

    cudaMalloc(&particles_gpu.pos, N_float4);
    cudaMalloc(&particles_gpu.vel, N_float4);
    cudaMalloc(&particles2_gpu.pos, N_float4);
    cudaMalloc(&particles2_gpu.vel, N_float4);

    cudaMemcpy(particles_gpu.pos, particles_cpu.pos, N_float4, cudaMemcpyHostToDevice);
    cudaMemcpy(particles_gpu.vel, particles_cpu.vel, N_float4, cudaMemcpyHostToDevice);
    cudaMemcpy(particles2_gpu.pos, particles_cpu.pos, N_float4, cudaMemcpyHostToDevice);
    cudaMemcpy(particles2_gpu.vel, particles_cpu.vel, N_float4, cudaMemcpyHostToDevice);

    size_t grid = (N + thr_blc - 1) / thr_blc;
    
    // vypocet
    gettimeofday(&t1, 0);

    for (int s = 0; s < steps; ++s)
    {
        calculate_velocity<<<grid, thr_blc>>>(particles_gpu, particles2_gpu, N, dt);
	std::swap(particles_gpu, particles2_gpu);
    }
    cudaDeviceSynchronize();
    gettimeofday(&t2, 0);

    // cas
    double t = (1000000.0 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec) / 1000000.0;
    printf("Time: %f s\n", t);

    cudaMemcpy(particles_cpu.pos, particles_gpu.pos, N_float4, cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.vel, particles_gpu.vel, N_float4, cudaMemcpyDeviceToHost);

    // ulozeni castic do souboru
    fp = fopen(argv[6], "w");
    if (fp == NULL)
    {
        printf("Can't open file %s!\n", argv[6]);
        exit(1);
    }
    particles_write(fp, particles_cpu, N);
    fclose(fp);

    return 0;
}
