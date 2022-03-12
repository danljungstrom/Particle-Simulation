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
    for(int i = 0; i < n; i++){
        //printf("before %f\n", particles[i].x);
       add_particle(&particles[i], *&nh, n); //maybe *&nh?
    }
    int nsize = get_nsize(n);
    int rows = get_rows(n);
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
            int coord = reduce_coord(rows, particles[i].x, particles[i].y);
            
            //printf("nx %d, ny %d, nx*rows+ny %d, coord %d\n", nx, ny, nx*rows+ny, coord);

            for(int x = max(nx - 1, 0); x <= min(nx + 1, nsize); x++){
                for(int y = max(ny - 1, 0); y <= min(ny + 1, nsize); y++){
                    //printf("x %d, y%d\n", x, y);
                        Part_list* pl = nh->at(coord);
                        //particle_t* p = pl->front();
                        //printf("%ld\n", sizeof(pl->begin()));
                        //iterator it = pl->begin();
                        //if(pl->size() < 2)
                            //printf("%ld\n", pl->size());
                        /*for(particle_t part = pl->begin(); part != pl->end(); ++part){
                            printf("part");
                        }*/
                        
                        int m = 0;
                        /*for (particle_t* part : *pl) {
                            printf("Particle %d inside Part_list %d", m++, nx * rows * ny);
                            if (part == NULL)
                                printf(" is NULL.\n");
                            else
                                printf(".\n");
                        }*/
                }
            }    
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
