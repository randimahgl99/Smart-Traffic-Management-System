#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>

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
    int id;
    int vehicle_count;
    Vehicle vehicles[MAX_VEHICLES];
    int signal_state;
    int signal_timer;
    int congestion_level;
} Intersection;


Intersection intersections[MAX_INTERSECTIONS];


// Initialize system (only root)
void initialize()
{
    for(int i = 0; i < MAX_INTERSECTIONS; i++)
    {
        intersections[i].id = i;
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



// Update vehicle
void updateVehicles(Intersection *in)
{
    for(int i = 0; i < in->vehicle_count; i++)
    {
        in->vehicles[i].position += in->vehicles[i].speed;
    }
}


// Update signal
void updateSignal(Intersection *in)
{
    in->signal_timer++;

    if(in->signal_timer >= 10)
    {
        in->signal_state = 1 - in->signal_state;
        in->signal_timer = 0;
    }
}


// Calculate congestion
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


    if(rank == 0)
    {
        initialize();
    }


    // Broadcast data to all MPI processes
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

        // OpenMP parallel region inside MPI process
        #pragma omp parallel for
        for(int i = start; i < end; i++)
        {
            updateVehicles(&intersections[i]);
            updateSignal(&intersections[i]);
            calculateCongestion(&intersections[i]);
        }

    }


    double end_time = MPI_Wtime();


    // Gather results back to root
    MPI_Gather(&intersections[start],
               intersections_per_process * sizeof(Intersection),
               MPI_BYTE,
               intersections,
               intersections_per_process * sizeof(Intersection),
               MPI_BYTE,
               0,
               MPI_COMM_WORLD);



    if(rank == 0)
    {
        printf("Hybrid MPI + OpenMP Simulation Completed\n");
        printf("MPI Processes: %d\n", size);
        printf("OpenMP Threads per Process: %d\n",
               omp_get_max_threads());

        printf("Execution Time: %f seconds\n",
               end_time - start_time);


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