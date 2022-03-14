#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "moore.h"
#include <string.h>
//
//  benchmarking program
//
int main(int argc, char **argv)
{
    //
    //  process command line parameters
    //
    if (find_option(argc, argv, "-h") >= 0)
    {
        printf("Options:\n");
        printf("-h to see this help\n");
        printf("-n <int> to set the number of particles\n");
        printf("-o <filename> to specify the output file name\n");
        printf( "-s <int> to set the number of steps in the simulation\n" );
        printf( "-f <int> to set the frequency of saving particle coordinates (e.g. each ten's step)\n" );
        return 0;
    }

    int n = read_int(argc, argv, "-n", 1000);
    int nsteps = read_int(argc, argv, "-s", NSTEPS);
    int savefreq = read_int(argc, argv, "-f", SAVEFREQ);
    char *savename = read_string(argc, argv, "-o", NULL);
    

    //
    //  set up MPI
    //
    int n_proc, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //
    //  allocate generic resources
    //
    FILE *fsave = savename ? fopen(savename, "w") : NULL;
    particle_t *particles = (particle_t *)malloc(n * sizeof(particle_t));
    Nh_t nh;
    

    

    int si = sizeof(int);
    int sd = sizeof(double);
    int ind = -si;
    int blens[] = {1, 1, 1, 1, 1, 1, 1, 1};
    MPI_Aint indices[] = {ind += si,
                          ind += sd, ind += sd, ind += sd,
                          ind += sd, ind += sd, ind += sd, sizeof(particle_t)};
    MPI_Datatype oldtypes[] = {MPI_INTEGER,
                               MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
                               MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_UB};
    MPI_Datatype PARTICLE;
    MPI_Type_struct(8, blens, indices, oldtypes, &PARTICLE);
    MPI_Type_commit(&PARTICLE);

    //
    //  set up the data partitioning across processors
    //
    int particle_per_proc = (n + n_proc - 1) / n_proc;
    int *partition_offsets = (int *)malloc((n_proc + 1) * sizeof(int));
    for (int i = 0; i < n_proc + 1; i++)
        partition_offsets[i] = min(i * particle_per_proc, n);

    int *partition_sizes = (int *)malloc(n_proc * sizeof(int));
    for (int i = 0; i < n_proc; i++)
        partition_sizes[i] = partition_offsets[i + 1] - partition_offsets[i];

    //
    //  allocate storage for local partition
    //
    int first = partition_offsets[rank];
    int last = partition_offsets[rank] + max(0, partition_sizes[rank] - 1);
    //int nlocal = partition_sizes[rank];
    //particle_t *local = (particle_t*) malloc( nlocal * sizeof(particle_t) );

    //
    //  initialize and distribute the particles (that's fine to leave it unoptimized)
    //
    //set_size( n );
    if (rank == 0)
        init_particles(n, particles);
    //MPI_Scatterv( particles, partition_sizes, partition_offsets, PARTICLE, local, nlocal, PARTICLE, 0, MPI_COMM_WORLD );
    MPI_Bcast(particles, n, PARTICLE, 0, MPI_COMM_WORLD);

    double size = set_size(n);
    int nh_size = (size / cutoff) + 1;
    neighborhood_initialize(nh, nh_size);
    for (int i = 0; i < n; i++)
    {
        particle_add(nh, particles[i]);
    }

    int locals[n];
    int ls = 0;
    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer();
    for (int step = 0; step < NSTEPS; step++)
    {
        //
        //  collect all global data locally (not good idea to do)
        //
        //MPI_Allgatherv( local, nlocal, PARTICLE, particles, partition_sizes, partition_offsets, PARTICLE, MPI_COMM_WORLD );

        MPI_Barrier(MPI_COMM_WORLD);

        //
        //  save current step if necessary (slightly different semantics than in other codes)
        //
        if (fsave && (step % SAVEFREQ) == 0)
            save(fsave, rank, n, particles, locals, ls, PARTICLE);

        //
        //  compute all forces
        //
        
        for( int i = first; i < last; i++ )
        {
            for (int j = 0; j < nh.size; j++)
                for (part_list_t *current = nh.neighborhood[i * nh.size + j]; current != 0; current = current->next)
                {
                    int k = current->particle_id;
                    locals[ls++] = k;
                    particles[i].ax = particles[i].ay = 0;
                    int nx = n_coord(particles[i].x);
                    int ny = n_coord(particles[i].y);

                    for (int px = max(nx - 1, 0); px <= min(nx + 1, nh.size - 1); px++)
                        for (int py = max(ny - 1, 0); py <= min(py + 1, nh.size - 1); py++)
                            for (part_list_t *neigh = nh.neighborhood[px * nh.size + py]; neigh != 0; neigh = neigh->next)
                                apply_force(particles[i], particles[neigh->particle_id]);
                }
            
        }

        //clearing rows
        if(first > 0)
            clear_row(nh, first-1);
        if (last < nh.size - 1)
            clear_row(nh, last + 1);
        
        
        //
        //  move particles
        //
        for( int i = 0; i < ls; i++ ){
            int id = locals[i];
            int nc = reduce_coord(nh.size, particles[id].x, particles[id].y);

            move(particles[id]);
            int nx = n_coord(particles[id].x);
            if (nx < first || nx > last)
            {
                if (!particle_remove(nh, particles[id], nc))
                {
                    printf("faield to remove particle");
                }

                particle_add(nh, particles[id]);

                int targ = (nx < first ? rank - 1 : rank + 1);

                MPI_Request request;
                MPI_Isend(particles + id, 1, PARTICLE, targ, targ, MPI_COMM_WORLD, &request);
                continue;
            }
            if (nc != reduce_coord(nh.size, particles[id].x, particles[id].y))
            {
                if (!particle_remove(nh, particles[id], nc))
                    printf("failed to remove particle");
            }
            particle_add(nh, particles[id]);
        }
        MPI_Request request;
        if (first > 0)
            for(int i = 0; i < nh.size; ++i)
                for(part_list_t* current = nh.neighborhood[first * nh.size + i]; current != 0; current = current->next)
                    MPI_Isend(particles+current->particle_id, 1, PARTICLE, rank-1, rank-1, MPI_COMM_WORLD, &request);
        if(last < nh.size-1)
            for(int i = 0; i < nh.size; ++i)
                for(part_list_t* current = nh.neighborhood[last * nh.size + i]; current != 0; current = current->next)
                    MPI_Isend(particles+current->particle_id, 1, PARTICLE, rank+1, rank+1, MPI_COMM_WORLD, &request);
    
        particle_t end;
        end.id = -1;
        if (first > 0)
            MPI_Isend(&end, 1, PARTICLE, rank-1, rank, MPI_COMM_WORLD, &request);
        if (last < nh.size - 1)
            MPI_Isend(&end, 1, PARTICLE, rank+1, rank, MPI_COMM_WORLD, &request);
    
    
        particle_t np;
        int breakc = 0;
        int breaklim = 2;
        if(first == 0)
            breaklim--;
        if(last == nh.size - 1)
            breaklim--;

        while(breakc < breaklim){
            MPI_Status ms;
            MPI_Recv(&np, 1, PARTICLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &ms);

            if(np.id == -1){
                breakc++;
                continue;
            }
            particles[np.id] = np;
            particle_add(nh, particles[np.id]);
        }
    
    }



    

        simulation_time = read_timer() - simulation_time;

    if (rank == 0)
        printf("n = %d, n_procs = %d, simulation time = %g s\n", n, n_proc, simulation_time);

    //
    //  release resources
    //
    free(partition_offsets);
    free(partition_sizes);
    free(particles);
    if (fsave)
        fclose(fsave);

    MPI_Finalize();

    return 0;
}