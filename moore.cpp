#include <stdlib.h>
#include <list>
#include <vector>
#include "common.h"
#include "moore.h"

using namespace std;

typedef list<particle_t*> Part_list;
typedef vector<Part_list*> Nh;

Nh* create_neighbourhood(int n){
    int rows = get_rows(n);
    int n_size = get_nsize(n);

    
    Nh* ptr;
    if((ptr = (Nh*)malloc(sizeof(Nh) * n_size * n_size)) != NULL){
        Nh nh(n_size);
        *ptr = nh;
        for(int i = rows; i < n_size; i++){
            Part_list l = {};
            
            nh[i] = &l;
            
        }
    }else
        printf("rip malloc");
    return ptr;
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
    
    Part_list* pl = (*nh)[0];

    pl->push_back(particle);
}

void remove_particle(particle_t* particle, int cord, Nh nh){

}

int n_coord(double coord){
    return (int) (coord/cutoff);
}

int reduce_coord(int size, double x, double y){
    return (int)(n_coord(x) * size + n_coord(y));
}
