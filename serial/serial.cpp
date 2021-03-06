#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <vector>
#include "common.h"
#include "moore.h"

#define DEBUG 0
using namespace std;

//
//  benchmarking program
//
int main( int argc, char **argv )
{    
    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set the number of particles\n" );
        printf( "-o <filename> to specify the output file name\n" );
        return 0;
    }
    
    int n = read_int( argc, argv, "-n", 1000 );

    char *savename = read_string( argc, argv, "-o", NULL );
    
    FILE *fsave = savename ? fopen( savename, "w" ) : NULL;
    particle_t *particles = (particle_t*) malloc( n * sizeof(particle_t) );
    double size = set_size( n );
    init_particles( n, particles );
    
    int nh_size = (size / cutoff) + 1;
    Nh_t nh;
    neighborhood_initialize(nh, nh_size);

    for (int i = 0; i < n; ++i)
    {
        particle_add(nh, &particles[i]);
    }

    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );
    for( int step = 0; step < NSTEPS; step++ )
    {
        //
        //  compute forces
        //
        for( int i = 0; i < n; i++ )
        {
            particles[i].ax = particles[i].ay = 0;

            int nx = n_coord(particles[i].x);
            int ny = n_coord(particles[i].y);

            for(int x = max(nx - 1, 0); x <= min(nx + 1, nh.size - 1); x++)
                for (int y = max(ny - 1, 0); y <= min(ny + 1, nh.size - 1); y++) {
                    part_list_t* pl = nh.neighborhood[x * nh.size + y];
                    while (pl != NULL){
                        apply_force(particles[i], *(pl->value));
                        pl = pl->next;
                    }
                }
        }
        
        //
        //  move particles
        //
        for (int i = 0; i < n; i++) {
            int prev_coord = reduce_coord(nh_size, particles[i].x, particles[i].y);
            move(particles[i]);
            int new_coord = reduce_coord(nh_size, particles[i].x, particles[i].y);

            if (prev_coord != new_coord) {
                if (!particle_remove(nh, &particles[i], prev_coord))
                    printf("Could not remove particle.\n");
                particle_add(nh, &particles[i]);
            }
        }
        
        //
        //  save if necessary
        //
        if( fsave && (step%SAVEFREQ) == 0 )
            save( fsave, n, particles );
    }
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "n = %d, simulation time = %g seconds\n", n, simulation_time );
    
    nh_clear(nh);
    free( particles );
    if( fsave )
        fclose( fsave );
    
    return 0;
}