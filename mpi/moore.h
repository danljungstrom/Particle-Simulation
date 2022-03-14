#ifndef __MOORE_H_
#define __MOORE_H_

#include "common.h"

struct part_list
{
	part_list* next;
	int particle_id;
};

typedef struct part_list part_list_t;

struct Nh
{
	int size;
	part_list_t** neighborhood;
};

typedef struct Nh Nh_t;

void neighborhood_initialize(Nh& nh, int size);
void particle_add(Nh_t& nh, particle_t& p);
bool particle_remove(Nh_t& nh, particle_t& p, int n_coord);
void nh_clear(Nh& nh);
void clear_row(Nh_t& neigh, int i);

#endif