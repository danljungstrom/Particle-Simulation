#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "common.h"
#include "moore.h"

void neighborhood_initialize(Nh& nh, int size) {
	nh.size = size;

    if ((nh.neighborhood = (part_list**) malloc(sizeof(part_list*) * size * size)) != NULL)
		memset(nh.neighborhood, 0, sizeof(part_list*) * size * size);
	else
		printf("Neighborhood malloc error");
}

void particle_add(Nh_t& nh, particle_t& p) {
    int coord = reduce_coord(nh.size, p.x, p.y);

    part_list_t* newElement = (part_list_t*)malloc(sizeof(part_list));
    newElement->particle_id = p.id;

    // Beginning of critical section
    newElement->next = nh.neighborhood[coord];

    nh.neighborhood[coord] = newElement;
    // End of critical section
}

bool particle_remove(Nh_t& nh, particle_t& p, int n_coord) {
    if (n_coord == -1)
        n_coord = reduce_coord(nh.size, p.x, p.y);

    if (nh.neighborhood[n_coord] == 0)
        return false;

    part_list_t** particlePointer = &(nh.neighborhood[n_coord]);
    part_list_t* part = nh.neighborhood[n_coord];

    while (part && part->particle_id != p.id) {
        particlePointer = &(part->next);
        part = part->next;
    }

    if (part) {
        *particlePointer = part->next;
        free(part);
    }

    return !!part;
}

void nh_clear(Nh& nh)
{
    for (int i = 0; i < nh.size * nh.size; ++i)
    {
        part_list_t* pl = nh.neighborhood[i];
        while (pl != 0)
        {
            part_list_t* temp = pl->next;
            free(pl);
            pl = temp;
        }
    }
    free(nh.neighborhood);
}
void clear_row(Nh_t& neigh, int i){
    for(int j = 0; j < neigh.size; j++){
        int k = i * neigh.size + j;
        part_list_t* current = neigh.neighborhood[k];
        while(current != 0){
            part_list_t* pl = current->next;
            free(current);
            current = pl;
        }
        memset(neigh.neighborhood + k, 0, sizeof(part_list_t*));
    }
}