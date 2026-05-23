#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MAX_INTERSECTIONS 50
#define MAX_VEHICLES 100
#define SIMULATION_STEPS 1000
#define ROAD_LENGTH 200

// ---------------- STRUCTURES ----------------
typedef struct
{
    int id;
    int position;
    int speed;
} Vehicle;

typedef struct
{
    int id;
    Vehicle vehicles[MAX_VEHICLES];
    int vehicle_count;
    int signal_state;
    int signal_timer;
    int congestion_level;
} Intersection;

// Local data (each process works on its own copy)
Intersection intersections[MAX_INTERSECTIONS];

// ---------------- INITIALIZATION ----------------
void initialize()
{
    for (int i = 0; i < MAX_INTERSECTIONS; i++)
    {
        intersections[i].id = i;
        intersections[i].vehicle_count = rand() % MAX_VEHICLES;

        intersections[i].signal_state = rand() % 2;
        intersections[i].signal_timer = 0;

        for (int j = 0; j < intersections[i].vehicle_count; j++)
        {
            intersections[i].vehicles[j].id = j;
            intersections[i].vehicles[j].position = rand() % ROAD_LENGTH;
            intersections[i].vehicles[j].speed = rand() % 5 + 1;
        }
    }
}

// ---------------- LOGIC ----------------
void updateVehicles(Intersection *in)
{
    for (int i = 0; i < in->vehicle_count; i++)
    {
        in->vehicles[i].position += in->vehicles[i].speed;

        if (in->vehicles[i].position > ROAD_LENGTH)
            in->vehicles[i].position = 0;
    }
}

void updateSignal(Intersection *in)
{
    in->signal_timer++;

    if (in->signal_timer >= 10)
    {
        in->signal_state = 1 - in->signal_state;
        in->signal_timer = 0;
    }
}

void calculateCongestion(Intersection *in)
{
    in->congestion_level = in->vehicle_count;
}

// ---------------- MAIN ----------------
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(42 + rank); // FIXED SEED

    // Root initializes full dataset
    if (rank == 0)
        initialize();

    // Broadcast same initial state to all processes
    MPI_Bcast(intersections,
              sizeof(intersections),
              MPI_BYTE,
              0,
              MPI_COMM_WORLD);

    // ---------------- DOMAIN DECOMPOSITION ----------------
    int base = MAX_INTERSECTIONS / size;
    int remainder = MAX_INTERSECTIONS % size;

    int start = rank * base + (rank < remainder ? rank : remainder);
    int count = base + (rank < remainder ? 1 : 0);
    int end = start + count;

    double t1 = MPI_Wtime();

    for (int t = 0; t < SIMULATION_STEPS; t++)
    {
        for (int i = start; i < end; i++)
        {
            updateVehicles(&intersections[i]);
            updateSignal(&intersections[i]);
            calculateCongestion(&intersections[i]);
        }
    }

    double t2 = MPI_Wtime();

    // ---------------- SAFE GATHER ----------------
    MPI_Gather(&intersections[start],
               count * sizeof(Intersection),
               MPI_BYTE,
               intersections,
               count * sizeof(Intersection),
               MPI_BYTE,
               0,
               MPI_COMM_WORLD);

    // ---------------- OUTPUT ----------------
    if (rank == 0)
    {
        printf("MPI Simulation Completed\n");
        printf("Processes: %d\n", size);
        printf("Execution Time: %f sec\n", t2 - t1);

        printf("\nSample Output:\n");
        for (int i = 0; i < 5; i++)
        {
            printf("Intersection %d | Congestion: %d\n",
                   intersections[i].id,
                   intersections[i].congestion_level);
        }
    }

    MPI_Finalize();
    return 0;
}