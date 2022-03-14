#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <omp.h>
#include "common.h"
#include "moore.h"

//
//  benchmarking program
//
int main( int argc, char **argv )
{    
    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set number of particles\n" );
        printf( "-o <filename> to specify the output file name\n" );
        return 0;
    }

    int n = read_int( argc, argv, "-n", 1000 );
    char *savename = read_string( argc, argv, "-o", NULL );
    int n_threads = read_int(argc, argv, "-p", 2);
    FILE *fsave = savename ? fopen( savename, "w" ) : NULL;

    particle_t *particles = (particle_t*) malloc( n * sizeof(particle_t) );
    double size = set_size( n );
    init_particles( n, particles );

    int nh_size = (size / cutoff) + 1;
    Nh_t nh;
    neighborhood_initialize(nh, nh_size);

    for (int i = 0; i < n; ++i)
        particle_add(nh, &particles[i]);

    omp_set_num_threads(n_threads);

    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );

    #pragma omp parallel
    for( int step = 0; step < 1000; step++ )
    {
        //
        //  compute all forces
        //
        #pragma omp for
        for (int i = 0; i < n; i++)
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
        
        //
        //  move particles
        //
        #pragma omp for
        for (int i = 0; i < n; i++) {
            int prev_coord = reduce_coord(nh.size, particles[i].x, particles[i].y);
            move(particles[i]);
            int new_coord = reduce_coord(nh.size, particles[i].x, particles[i].y);

            if (prev_coord != new_coord) {
                if (!particle_remove(nh, &particles[i], prev_coord))
                    printf("Could not remove particle %d that moved from %d to %d.\n", i, prev_coord, new_coord);
                particle_add(nh, &particles[i]);
            }
        }
        
        //
        //  save if necessary
        //
        #pragma omp master
        if( fsave && (step%SAVEFREQ) == 0 )
            save( fsave, n, particles );
    }
    simulation_time = read_timer( ) - simulation_time;
    
    printf("n = %d, n_threads = %d, simulation time = %g seconds\n", n, n_threads, simulation_time);
    
    free( particles );
    if( fsave )
        fclose( fsave );
    
    return 0;
}