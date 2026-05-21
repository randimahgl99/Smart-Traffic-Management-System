#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda.h>

#define MAX_INTERSECTIONS 50
#define MAX_VEHICLES 100
#define SIMULATION_STEPS 1000


// Vehicle structure
typedef struct
{
    int position;
    int speed;
} Vehicle;


// Intersection structure
typedef struct
{
    int vehicle_count;
    Vehicle vehicles[MAX_VEHICLES];
    int signal_state;
    int signal_timer;
    int congestion_level;
} Intersection;



// CUDA Kernel function
__global__ void simulateTraffic(Intersection *intersections, int num_intersections)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    if(idx < num_intersections)
    {
        for(int t = 0; t < SIMULATION_STEPS; t++)
        {
            // Update vehicles
            for(int i = 0; i < intersections[idx].vehicle_count; i++)
            {
                intersections[idx].vehicles[i].position +=
                intersections[idx].vehicles[i].speed;
            }

            // Update signal
            intersections[idx].signal_timer++;

            if(intersections[idx].signal_timer >= 10)
            {
                intersections[idx].signal_state =
                1 - intersections[idx].signal_state;

                intersections[idx].signal_timer = 0;
            }

            // Calculate congestion
            intersections[idx].congestion_level =
            intersections[idx].vehicle_count;
        }
    }
}



// Initialize system
void initialize(Intersection *intersections)
{
    for(int i = 0; i < MAX_INTERSECTIONS; i++)
    {
        intersections[i].vehicle_count = rand() % MAX_VEHICLES;

        intersections[i].signal_state = rand() % 2;
        intersections[i].signal_timer = 0;

        for(int j = 0; j < intersections[i].vehicle_count; j++)
        {
            intersections[i].vehicles[j].position = rand() % 100;
            intersections[i].vehicles[j].speed = rand() % 5 + 1;
        }
    }
}



int main()
{
    srand(time(NULL));

    Intersection *h_intersections;
    Intersection *d_intersections;

    size_t size = sizeof(Intersection) * MAX_INTERSECTIONS;

    // Allocate host memory
    h_intersections = (Intersection*)malloc(size);

    initialize(h_intersections);

    // Allocate GPU memory
    cudaMalloc((void**)&d_intersections, size);

    // Copy data from CPU to GPU
    cudaMemcpy(d_intersections,
               h_intersections,
               size,
               cudaMemcpyHostToDevice);


    clock_t start, end;

    start = clock();

    // Launch kernel
    int threadsPerBlock = 256;
    int blocksPerGrid =
    (MAX_INTERSECTIONS + threadsPerBlock - 1) / threadsPerBlock;

    simulateTraffic<<<blocksPerGrid, threadsPerBlock>>>
    (d_intersections, MAX_INTERSECTIONS);

    // Wait for GPU to finish
    cudaDeviceSynchronize();

    end = clock();


    // Copy results back to CPU
    cudaMemcpy(h_intersections,
               d_intersections,
               size,
               cudaMemcpyDeviceToHost);


    double time_taken =
    (double)(end - start) / CLOCKS_PER_SEC;


    printf("CUDA Simulation Completed\n");
    printf("Execution Time: %f seconds\n", time_taken);
    printf("GPU Threads Used: %d\n",
           blocksPerGrid * threadsPerBlock);


    printf("\nSample Output:\n");

    for(int i = 0; i < 5; i++)
    {
        printf("Intersection %d Congestion Level: %d\n",
               i,
               h_intersections[i].congestion_level);
    }


    // Free memory
    cudaFree(d_intersections);
    free(h_intersections);

    return 0;
}
