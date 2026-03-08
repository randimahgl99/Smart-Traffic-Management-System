#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

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
    int state;
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


// Global traffic network
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



int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL) + rank);

    // Initialize only in root process
    if(rank == 0)
    {
        initialize();
    }

    // Broadcast full intersection data to all processes
    MPI_Bcast(intersections,
              sizeof(intersections),
              MPI_BYTE,
              0,
              MPI_COMM_WORLD);


    int intersections_per_process = MAX_INTERSECTIONS / size;

    int start = rank * intersections_per_process;
    int end = start + intersections_per_process;

    double start_time = MPI_Wtime();

    // Simulation loop
    for(int t = 0; t < SIMULATION_STEPS; t++)
    {
        for(int i = start; i < end; i++)
        {
            updateVehicles(&intersections[i]);
            updateSignal(&intersections[i]);
            calculateCongestion(&intersections[i]);
        }
    }

    double end_time = MPI_Wtime();

    // Gather results back to root process
    MPI_Gather(&intersections[start],
               intersections_per_process * sizeof(Intersection),
               MPI_BYTE,
               intersections,
               intersections_per_process * sizeof(Intersection),
               MPI_BYTE,
               0,
               MPI_COMM_WORLD);


    // Root prints results
    if(rank == 0)
    {
        printf("MPI Simulation Completed\n");
        printf("Number of Processes: %d\n", size);
        printf("Execution Time: %f seconds\n", end_time - start_time);

        printf("\nSample Output:\n");

        for(int i = 0; i < 5; i++)
        {
            printf("Intersection %d Congestion Level: %d\n",
                   intersections[i].id,
                   intersections[i].congestion_level);
        }
    }

    MPI_Finalize();

    return 0;
}