#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include "common.h"
#include "moore.h"

void neighborhood_initialize(Nh& nh, int size) {
	nh.size = size;

    if ((nh.neighborhood = (part_list**) malloc(sizeof(part_list*) * size * size)) != NULL)
		memset(nh.neighborhood, 0, sizeof(part_list*) * size * size);
	else
		printf("Nh neighborhood malloc error");

    if((nh.lock = (omp_lock_t*) malloc(sizeof(omp_lock_t) * size * size)) != NULL)
        for (int i = 0; i < size * size; ++i)
            omp_init_lock(&nh.lock[i]);
    else
        printf("Nh lock malloc error");
}

void particle_add(Nh_t& nh, particle_t* p) {
    int coord = reduce_coord(nh.size, p->x, p->y);

    part_list_t* newElement = (part_list_t*)malloc(sizeof(part_list));
    newElement->value = p;

    omp_set_lock(&nh.lock[coord]);
    newElement->next = nh.neighborhood[coord];

    nh.neighborhood[coord] = newElement;
    omp_unset_lock(&nh.lock[coord]);
}

bool particle_remove(Nh_t& nh, particle_t* p, int coord) {
    if (coord == -1)
        coord = reduce_coord(nh.size, p->x, p->y);

    if (nh.neighborhood[coord] == 0)
        return false;
    
    omp_set_lock(&nh.lock[coord]);

    part_list_t** particlePointer = &(nh.neighborhood[coord]);
    part_list_t* part = nh.neighborhood[coord];

    while (part && (part->value != p)) {
        particlePointer = &(part->next);
        part = part->next;
    }

    if (part) {
        *particlePointer = part->next;
        free(part);
    }

    omp_unset_lock(&nh.lock[coord]);

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