#include <stdlib.h>
#include <list>
#include <vector>
#include "common.h"
#include "moore.h"

using namespace std;

typedef vector<particle_t*> Part_list;
typedef vector<Part_list*> Nh;

Nh* create_neighbourhood(int n){
    int rows = get_rows(n);
    int n_size = get_nsize(n);
    //printf("%d\n", rows);
    //printf("%d\n", n_size);

    
    Nh* ptr;
    if((ptr = (Nh*)malloc(sizeof(Part_list*)* n_size * sizeof(particle_t*) * n)) != NULL){
        Nh nh(n_size);
        
        //for(int i = rows; i < n_size; i++){
            Part_list l(sizeof(particle_t*) * n);
            fill(nh.begin(), nh.end(), (Part_list*)&l);
            //printf("size %ld, max%ld\n", l.size(), l.max_size());
            //nh[i] = (Part_list*)&l;
            
        //}
        *ptr = nh;
    }else
        printf("rip malloc");
    //printf("%lu", (sizeof(Part_list*)* n_size * sizeof(particle_t*) * n));
    return ptr;
}

void test(){
    particle_t *parts = (particle_t*) malloc( 1 * sizeof(particle_t) );
    init_particles(1, parts);
    Part_list l;
    l.push_back((particle_t*)&parts[0]);
    printf("%d", (int)(*l.begin())->x);
    //l.pop_front();
}

int get_rows(int n){
    double size = set_size(n);
    int rows = n_coord(size) + 1;
    return rows;
}

int get_nsize(int n){
    int rows = get_rows(n);
    return rows * rows + rows;
}


void add_particle(particle_t* particle, Nh* nh, int n){
    
    int cord = reduce_coord(get_rows(n), particle->x, particle->y);
    //printf("in func %f\n", particle->x);
    Part_list* pl = nh->at(cord);
    //printf("size %ld, max%ld\n", pl->size(), pl->max_size());
    //printf("%f\n", pl->back()->x);
    pl->push_back(particle);
    //printf("after push %f\n", pl->back()->x);
}

void remove_particle(particle_t* particle, int cord, Nh nh){

}

int n_coord(double coord){
    return (int) (coord/cutoff/10);
}

int reduce_coord(int size, double x, double y){
    return (int)(n_coord(x) * size + n_coord(y));
}
