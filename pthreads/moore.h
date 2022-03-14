#ifndef __MOORE_H_
#define __MOORE_H_

#include "common.h"

struct part_list
{
	part_list * next;
	particle_t * value;
};

typedef struct part_list part_list_t;

struct Nh
{
	int size;
	part_list_t ** neighborhood;
	pthread_mutex_t * lock;
};

typedef struct Nh Nh_t;

void neighborhood_initialize(Nh_t & nh, int size);
void particle_add(Nh_t & nh, particle_t * particle);
bool particle_remove(Nh_t & nh, particle_t * p, int coord = -1);
void nh_clear(Nh_t & nh);
int n_coord(double coord);
int reduce_coord(int size, double x, double y);

#endif