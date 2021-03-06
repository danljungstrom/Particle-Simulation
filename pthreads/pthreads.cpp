#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <pthread.h>

#include "common.h"
#include "moore.h"

#define DEBUG 1
using namespace std;

//
//  global variables
int n, n_threads;
particle_t* particles;
FILE* fsave;
pthread_barrier_t barrier;
Nh_t nh;

//
//  check that pthreads routine call was successful
//
#define P( condition ) {if( (condition) != 0 ) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}

pthread_mutex_t task_lock = PTHREAD_MUTEX_INITIALIZER;

//
//  This is where the action happens
//
void* thread_routine(void* pthread_id)
{
	int thread_id = *(int*)pthread_id;

	int particles_per_thread = (n + n_threads - 1) / n_threads;
	int first = min(thread_id * particles_per_thread, n);
	int last = min((thread_id + 1) * particles_per_thread, n);

	//
	//  simulate a number of time steps
	//
	for (int step = 0; step < NSTEPS; step++)
	{
		//
		//  compute forces
		//
		for (int i = first; i < last; ++i)
		{
			particles[i].ax = particles[i].ay = 0;

			int nx = n_coord(particles[i].x);
			int ny = n_coord(particles[i].y);

			for (int x = max(nx - 1, 0); x <= min(nx + 1, nh.size - 1); x++)
				for (int y = max(ny - 1, 0); y <= min(ny + 1, nh.size - 1); y++) {
					part_list_t* pl = nh.neighborhood[x * nh.size + y];
					while (pl != NULL)
					{
						apply_force(particles[i], *(pl->value));
						pl = pl->next;
					}
				}
		}

		pthread_barrier_wait(&barrier);

		//  Move particles
		for (int i = first; i < last; i++)
		{
			int gc = reduce_coord(nh.size, particles[i].x, particles[i].y);

			move(particles[i]);

			if (gc != reduce_coord(nh.size, particles[i].x, particles[i].y))
			{
				if (!particle_remove(nh, &particles[i], gc))
					printf("Could not remove particle.\n");
				particle_add(nh, &particles[i]);
			}
		}

		pthread_barrier_wait(&barrier);
	}

	return NULL;
}

//
//  benchmarking program
int main(int argc, char** argv)
{
	//
	//  process command line
	//
	if (find_option(argc, argv, "-h") >= 0)
	{
		printf("Options:\n");
		printf("-h to see this help\n");
		printf("-n <int> to set the number of particles\n");
		printf("-p <int> to set the number of threads\n");
		printf("-o <filename> to specify the output file name\n");
		return 0;
	}

	n = read_int(argc, argv, "-n", 1000);
	n_threads = read_int(argc, argv, "-p", 2);
	char* savename = read_string(argc, argv, "-o", NULL);

	//
	//  allocate resources
	//
	fsave = savename ? fopen(savename, "w") : NULL;

	particles = (particle_t*)malloc(n * sizeof(particle_t));
	double size = set_size(n);
	init_particles(n, particles);

	int nh_size = (size / cutoff) + 1;
	neighborhood_initialize(nh, nh_size);
	for (int i = 0; i < n; ++i)
		particle_add(nh, &particles[i]);

	pthread_attr_t attr;
	P(pthread_attr_init(&attr));
	P(pthread_barrier_init(&barrier, NULL, n_threads));

	int* thread_ids = (int*)malloc(n_threads * sizeof(int));
	for (int i = 0; i < n_threads; i++)
		thread_ids[i] = i;

	pthread_t* threads = (pthread_t*)malloc(n_threads * sizeof(pthread_t));

	//
	//  do the parallel work
	//
	double simulation_time = read_timer();


	for (int i = 1; i < n_threads; i++)
		P(pthread_create(&threads[i], &attr, thread_routine, &thread_ids[i]));

	thread_routine(&thread_ids[0]);

	for (int i = 1; i < n_threads; i++)
		P(pthread_join(threads[i], NULL));
	simulation_time = read_timer() - simulation_time;

	printf("n = %d, n_threads = %d, simulation time = %g seconds\n", n, n_threads, simulation_time);

	//
	//  release resources
	//
	P(pthread_barrier_destroy(&barrier));
	P(pthread_attr_destroy(&attr));
	free(thread_ids);
	free(threads);
	nh_clear(nh);
	free(particles);
	if (fsave)
		fclose(fsave);

	return 0;
}
