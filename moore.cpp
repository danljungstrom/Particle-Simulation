#include <stdlib.h>
#include <list>
#include <vector>
#include "common.h"
#include "moore.h"

using namespace std;

typedef list<particle_t*> Part_list;
typedef vector<Part_list> Nh;

Nh* create_neighbourhood(int n){
    int rows = get_rows(n);
    int n_size = get_nsize(n);

    Nh nh(n_size);
    Nh* ptr = malloc(sizeof(nh));
    for(int i = rows; i < n_size; i++){
            Part_list l = {};
            nh[i] = l;
    }
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


void add_particle(particle_t* particle, Nh nh, int edge_size){
    int x = (int)((particle->ax) * edge_size / cutoff);
    int y = (int)((particle->ay) / cutoff);
    int cord = x + y;


    nh[cord].push_back(particle);
}

void remove_particle(particle_t* particle, int cord, Nh nh){

    nh[cord].remove(particle);
}

int n_coord(double coord){
    return (int) (coord/cutoff);
}
