#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX_INTERSECTIONS 50
#define MAX_VEHICLES 100
#define SIMULATION_STEPS 1000

// Vehicle structure represents a single vehicle in the system
typedef struct
{
    int id;         // Unique vehicle ID
    int position;   // Current position on the road
    int speed;      // Vehicle speed
} Vehicle;

// Traffic signal structure
typedef struct
{
    int state;  // 0 = RED, 1 = GREEN
    int timer;  // Timer to track signal duration
} TrafficSignal;

// Intersection structure representing each road junction
typedef struct
{
    int id;                              // Intersection ID
    Vehicle vehicles[MAX_VEHICLES];      // Vehicles at the intersection
    int vehicle_count;                   // Number of vehicles present
    TrafficSignal signal;                // Traffic signal at this intersection
    int congestion_level;                // Calculated congestion level
} Intersection;


// Shared traffic network (global memory)
Intersection intersections[MAX_INTERSECTIONS];


// Initialize traffic system with random vehicles and signal states
void initialize()
{
    for(int i = 0; i < MAX_INTERSECTIONS; i++)
    {
        intersections[i].id = i;

        // Random number of vehicles at each intersection
        intersections[i].vehicle_count = rand() % MAX_VEHICLES;

        // Random initial signal state
        intersections[i].signal.state = rand() % 2;
        intersections[i].signal.timer = 0;

        // Initialize vehicles
        for(int j = 0; j < intersections[i].vehicle_count; j++)
        {
            intersections[i].vehicles[j].id = j;
            intersections[i].vehicles[j].position = rand() % 100;
            intersections[i].vehicles[j].speed = rand() % 5 + 1;
        }
    }
}


// Update vehicle positions based on their speed
void updateVehicles(Intersection *in)
{
    for(int i = 0; i < in->vehicle_count; i++)
    {
        in->vehicles[i].position += in->vehicles[i].speed;
    }
}


// Update traffic signal state periodically
void updateSignal(Intersection *in)
{
    in->signal.timer++;

    // Change signal every 10 simulation steps
    if(in->signal.timer >= 10)
    {
        in->signal.state = 1 - in->signal.state; // Toggle signal
        in->signal.timer = 0;
    }
}


// Calculate congestion level based on number of vehicles
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

    // Main simulation loop
    for(int t = 0; t < SIMULATION_STEPS; t++)
    {

        // Parallel processing of intersections using OpenMP
        #pragma omp parallel for shared(intersections)
        for(int i = 0; i < MAX_INTERSECTIONS; i++)
        {
            updateVehicles(&intersections[i]);
            updateSignal(&intersections[i]);
            calculateCongestion(&intersections[i]);
        }

    }

    end = omp_get_wtime();

    printf("OpenMP Traffic Simulation Completed\n");
    printf("Execution Time: %f seconds\n", end - start);

    printf("\nNumber of Threads Used: %d\n", omp_get_max_threads());

    // Display sample results
    printf("\nSample Output:\n");

    for(int i = 0; i < 5; i++)
    {
        printf("Intersection %d | Congestion Level: %d | Signal: %s\n",
               intersections[i].id,
               intersections[i].congestion_level,
               intersections[i].signal.state ? "GREEN" : "RED");
    }

    return 0;
}