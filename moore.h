#ifndef __MOORE_H__
#define __MOORE_H__

#include <stdlib.h>
#include <list>
#include <vector>
#include "common.h"

using namespace std;

typedef vector<particle_t*> Part_list;
typedef vector<Part_list*> Nh;

Nh* create_neighbourhood(int n);
int get_rows(int n);
int get_nsize(int n);
void add_particle(particle_t* particle, Nh* nh, int n);
void remove_particle(particle_t* particle, int cord, Nh nh);
int reduce_coord(int size, double x, double y);
int n_coord(double coord);
void test();

#endif