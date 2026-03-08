#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX_INTERSECTIONS 50
#define MAX_VEHICLES 100
#define SIMULATION_STEPS 1000

// Vehicle structure
typedef struct
{
    int id;
    int position;
    int speed;
} Vehicle;

// Traffic signal structure
typedef struct
{
    int state; // 0 = RED, 1 = GREEN
    int timer;
} TrafficSignal;

// Intersection structure
typedef struct
{
    int id;
    Vehicle vehicles[MAX_VEHICLES];
    int vehicle_count;
    TrafficSignal signal;
    int congestion_level;
} Intersection;


// Global traffic network (shared memory)
Intersection intersections[MAX_INTERSECTIONS];


// Initialize traffic system
void initialize()
{
    for(int i = 0; i < MAX_INTERSECTIONS; i++)
    {
        intersections[i].id = i;
        intersections[i].vehicle_count = rand() % MAX_VEHICLES;

        intersections[i].signal.state = rand() % 2;
        intersections[i].signal.timer = 0;

        for(int j = 0; j < intersections[i].vehicle_count; j++)
        {
            intersections[i].vehicles[j].id = j;
            intersections[i].vehicles[j].position = rand() % 100;
            intersections[i].vehicles[j].speed = rand() % 5 + 1;
        }
    }
}


// Update vehicle positions
void updateVehicles(Intersection *in)
{
    for(int i = 0; i < in->vehicle_count; i++)
    {
        in->vehicles[i].position += in->vehicles[i].speed;
    }
}


// Update traffic signal
void updateSignal(Intersection *in)
{
    in->signal.timer++;

    if(in->signal.timer >= 10)
    {
        in->signal.state = 1 - in->signal.state;
        in->signal.timer = 0;
    }
}


// Calculate congestion level
void calculateCongestion(Intersection *in)
{
    in->congestion_level = in->vehicle_count;
}



int main()
{
    srand(time(NULL));

    initialize();

    double start, end;

    start = omp_get_wtime();

    // Simulation loop
    for(int t = 0; t < SIMULATION_STEPS; t++)
    {

        // Parallel loop using OpenMP
        #pragma omp parallel for shared(intersections)
        for(int i = 0; i < MAX_INTERSECTIONS; i++)
        {
            updateVehicles(&intersections[i]);
            updateSignal(&intersections[i]);
            calculateCongestion(&intersections[i]);
        }

    }

    end = omp_get_wtime();

    printf("OpenMP Simulation Completed\n");
    printf("Execution Time: %f seconds\n", end - start);

    printf("\nNumber of Threads Used: %d\n", omp_get_max_threads());


    // Print sample output
    printf("\nSample Output:\n");

    for(int i = 0; i < 5; i++)
    {
        printf("Intersection %d Congestion Level: %d\n",
               intersections[i].id,
               intersections[i].congestion_level);
    }

    return 0;
}