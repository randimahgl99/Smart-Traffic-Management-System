#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_INTERSECTIONS 50
#define MAX_VEHICLES 100
#define SIMULATION_STEPS 1000
#define NUM_THREADS 4
#define ROAD_LENGTH 200   // ✅ FIXED

typedef struct
{
    int id;
    int position;
    int speed;
} Vehicle;

typedef struct
{
    int state;
    int timer;
} TrafficSignal;

typedef struct
{
    int id;
    Vehicle vehicles[MAX_VEHICLES];
    int vehicle_count;
    TrafficSignal signal;
    int congestion_level;
} Intersection;

Intersection intersections[MAX_INTERSECTIONS];

typedef struct
{
    int start;
    int end;
} ThreadData;

// ---------------- INIT ----------------
void initialize()
{
    for (int i = 0; i < MAX_INTERSECTIONS; i++)
    {
        intersections[i].id = i;
        intersections[i].vehicle_count = rand() % MAX_VEHICLES;

        intersections[i].signal.state = rand() % 2;
        intersections[i].signal.timer = 0;

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

        // ✅ FIX: same as serial model
        if (in->vehicles[i].position > ROAD_LENGTH)
        {
            in->vehicles[i].position = 0;
        }
    }
}

void updateSignal(Intersection *in)
{
    in->signal.timer++;

    if (in->signal.timer >= 10)
    {
        in->signal.state = 1 - in->signal.state;
        in->signal.timer = 0;
    }
}

void calculateCongestion(Intersection *in)
{
    in->congestion_level = in->vehicle_count;
}

// ---------------- THREAD FUNCTION ----------------
void* processIntersections(void* arg)
{
    ThreadData *data = (ThreadData*)arg;

    for (int t = 0; t < SIMULATION_STEPS; t++)
    {
        for (int i = data->start; i < data->end; i++)
        {
            updateVehicles(&intersections[i]);
            updateSignal(&intersections[i]);
            calculateCongestion(&intersections[i]);
        }
    }

    pthread_exit(NULL);
}

// ---------------- MAIN ----------------
int main()
{
    // ✅ FIX: same seed for fair comparison
    srand(42);

    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    initialize();

    int chunk = MAX_INTERSECTIONS / NUM_THREADS;

    clock_t start = clock();

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threadData[i].start = i * chunk;
        threadData[i].end = (i == NUM_THREADS - 1)
                            ? MAX_INTERSECTIONS
                            : threadData[i].start + chunk;

        pthread_create(&threads[i], NULL, processIntersections, &threadData[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    clock_t end = clock();

    printf("Pthreads Simulation Completed\n");
    printf("Execution Time: %f seconds\n",
           (double)(end - start) / CLOCKS_PER_SEC);

    printf("\nSample Output:\n");
    for (int i = 0; i < 5; i++)
    {
        printf("Intersection %d | Congestion: %d | Signal: %s\n",
               intersections[i].id,
               intersections[i].congestion_level,
               intersections[i].signal.state ? "GREEN" : "RED");
    }

    return 0;
}