#include <stdlib.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include "common.cpp"

using namespace std;

typedef list<particle_t*> Part_list;
typedef vector<Part_list> Nh;

void* create_neighbourhood(int n){
    set_size(n);
    int rows = (int) (size/cutoff + 1);
    int grid_size = rows * rows + rows;
    Nh nh(grid_size);
    void* ptr = malloc(sizeof(nh));
    for(int i = rows; i < grid_size; i++){
            Part_list l = {};
            nh[i] = l;
    }
    return ptr;
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
