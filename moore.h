#ifndef __MOORE_H_
#define __MOORE_H_

#include "common.h"

struct part_list
{
	part_list* next;
	particle_t* value;
};

typedef struct part_list part_list_t;

struct Nh
{
	int size;
	part_list_t** neighborhood;
};

typedef struct Nh Nh_t;

void neighborhood_initialize(Nh& nh, int size);

#endif