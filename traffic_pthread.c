#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_INTERSECTIONS 50
#define MAX_VEHICLES 100
#define SIMULATION_STEPS 1000
#define NUM_THREADS 4   // Change this for testing (2, 4, 8 etc.)


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


// Global shared traffic network
Intersection intersections[MAX_INTERSECTIONS];


// Mutex lock for synchronization
pthread_mutex_t lock;


// Thread argument structure
typedef struct
{
    int start;
    int end;
} ThreadData;



// Initialize system
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



// Update vehicles
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
    in->signal.timer++;

    if(in->signal.timer >= 10)
    {
        in->signal.state = 1 - in->signal.state;
        in->signal.timer = 0;
    }
}



// Calculate congestion
void calculateCongestion(Intersection *in)
{
    // Protect shared data using mutex
    pthread_mutex_lock(&lock);

    in->congestion_level = in->vehicle_count;

    pthread_mutex_unlock(&lock);
}



// Thread function
void* processIntersections(void* arg)
{
    ThreadData *data = (ThreadData*)arg;

    for(int t = 0; t < SIMULATION_STEPS; t++)
    {
        for(int i = data->start; i < data->end; i++)
        {
            updateVehicles(&intersections[i]);
            updateSignal(&intersections[i]);
            calculateCongestion(&intersections[i]);
        }
    }

    pthread_exit(NULL);
}



int main()
{
    srand(time(NULL));

    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    pthread_mutex_init(&lock, NULL);

    initialize();

    int intersections_per_thread = MAX_INTERSECTIONS / NUM_THREADS;

    clock_t start, end;

    start = clock();


    // Create threads
    for(int i = 0; i < NUM_THREADS; i++)
    {
        threadData[i].start = i * intersections_per_thread;
        threadData[i].end = threadData[i].start + intersections_per_thread;

        pthread_create(&threads[i],
                       NULL,
                       processIntersections,
                       &threadData[i]);
    }


    // Wait for threads
    for(int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }


    end = clock();

    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Pthread Simulation Completed\n");
    printf("Number of Threads: %d\n", NUM_THREADS);
    printf("Execution Time: %f seconds\n", time_taken);


    printf("\nSample Output:\n");

    for(int i = 0; i < 5; i++)
    {
        printf("Intersection %d Congestion Level: %d\n",
               intersections[i].id,
               intersections[i].congestion_level);
    }


    pthread_mutex_destroy(&lock);

    return 0;
}