#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
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
        printf( "-n <int> to set the number of particles\n" );
        printf( "-o <filename> to specify the output file name\n" );
        return 0;
    }
    
    int n = read_int( argc, argv, "-n", 1000 );

    char *savename = read_string( argc, argv, "-o", NULL );
    
    FILE *fsave = savename ? fopen( savename, "w" ) : NULL;
    particle_t *particles = (particle_t*) malloc( n * sizeof(particle_t) );
    set_size( n );
    
    init_particles( n, particles );
    Nh* nh = create_neighbourhood(n);
    //if((*nh)[0] == NULL)
        printf("wihoo");
    for(int i = 0; i < n; i++){
       add_particle(&particles[i], *&nh, n); //maybe *&nh?
    }
    int nsize = get_nsize(n);
    //test();
    
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

            for(int x = max(nx - 1, 0); x <= min(nx + 1, nsize); x++){
                for(int y = max(ny - 1, 0); y <= min(ny + 1, nsize); y++){
                    //if((*nh).size() > 0){
                        //Part_list* pl = (*nh)[nx * nsize + ny];
                        //particle_t* p = (*pl).front(); 
                        //printf("%f", p->x);
                    //}
                }
            }
            for (int j = 0; j < n; j++ )
                apply_force( particles[i], particles[j] );
        }
        
        //
        //  move particles
        //
        for( int i = 0; i < n; i++ ){ 
            move( particles[i] );
            //printf("(%f, %f), ", particles[i].x, particles[i].y);
        }
        
        //
        //  save if necessary
        //
        if( fsave && (step%SAVEFREQ) == 0 )
            save( fsave, n, particles );
    }
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "n = %d, simulation time = %g seconds\n", n, simulation_time );
    
    free( particles );
    if( fsave )
        fclose( fsave );
    
    return 0;
}
