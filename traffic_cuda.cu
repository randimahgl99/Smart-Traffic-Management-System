#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda.h>

#define MAX_INTERSECTIONS 50
#define MAX_VEHICLES 100
#define SIMULATION_STEPS 1000
#define ROAD_LENGTH 200

// ---------------- Structures ----------------
typedef struct {
    int position;
    int speed;
} Vehicle;

typedef struct {
    int vehicle_count;
    Vehicle vehicles[MAX_VEHICLES];
    int signal_state;
    int signal_timer;
    int congestion_level;
} Intersection;

// ---------------- CUDA Kernel ----------------
__global__ void simulateTraffic(Intersection *intersections, int num_intersections)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx < num_intersections)
    {
        for (int t = 0; t < SIMULATION_STEPS; t++)
        {
            // Update vehicles
            for (int i = 0; i < intersections[idx].vehicle_count; i++)
            {
                intersections[idx].vehicles[i].position +=
                    intersections[idx].vehicles[i].speed;

                // SAME AS SERIAL / OMP / MPI / PTHREAD
                if (intersections[idx].vehicles[i].position > ROAD_LENGTH)
                {
                    intersections[idx].vehicles[i].position = 0;
                }
            }

            // Update signal
            intersections[idx].signal_timer++;

            if (intersections[idx].signal_timer >= 10)
            {
                intersections[idx].signal_state =
                    1 - intersections[idx].signal_state;

                intersections[idx].signal_timer = 0;
            }

            // Congestion
            intersections[idx].congestion_level =
                intersections[idx].vehicle_count;
        }
    }
}

// ---------------- Initialization ----------------
void initialize(Intersection *intersections)
{
    srand(42); // FIXED SEED for fairness

    for (int i = 0; i < MAX_INTERSECTIONS; i++)
    {
        intersections[i].vehicle_count = rand() % MAX_VEHICLES;
        intersections[i].signal_state = rand() % 2;
        intersections[i].signal_timer = 0;

        for (int j = 0; j < intersections[i].vehicle_count; j++)
        {
            intersections[i].vehicles[j].position = rand() % ROAD_LENGTH;
            intersections[i].vehicles[j].speed = rand() % 5 + 1;
        }
    }
}

// ---------------- MAIN ----------------
int main()
{
    Intersection *h_intersections, *d_intersections;
    size_t size = sizeof(Intersection) * MAX_INTERSECTIONS;

    h_intersections = (Intersection*)malloc(size);

    initialize(h_intersections);

    cudaMalloc((void**)&d_intersections, size);

    cudaMemcpy(d_intersections, h_intersections, size, cudaMemcpyHostToDevice);

    // GPU timing (IMPORTANT FIX)
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    cudaEventRecord(start);

    int threadsPerBlock = 256;
    int blocksPerGrid = (MAX_INTERSECTIONS + threadsPerBlock - 1) / threadsPerBlock;

    simulateTraffic<<<blocksPerGrid, threadsPerBlock>>>(d_intersections, MAX_INTERSECTIONS);

    cudaDeviceSynchronize();

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);

    cudaMemcpy(h_intersections, d_intersections, size, cudaMemcpyDeviceToHost);

    printf("CUDA Simulation Completed\n");
    printf("Execution Time: %f ms\n", milliseconds);
    printf("GPU Threads Used: %d\n", blocksPerGrid * threadsPerBlock);

    printf("\nSample Output:\n");

    for (int i = 0; i < 5; i++)
    {
        printf("Intersection %d Congestion Level: %d\n",
               i,
               h_intersections[i].congestion_level);
    }

    cudaFree(d_intersections);
    free(h_intersections);

    return 0;
}