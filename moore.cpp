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