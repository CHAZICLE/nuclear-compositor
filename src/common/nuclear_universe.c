#include "nuclear_atom.h"
#include <stdlib.h>
#include <stdio.h>
#include "nuclear_utils.h"
#include <string.h>
#include <math.h>

#include "nuclear_universe.h"

void nuclear_universe_init(nuclear_universe *unv, nuclear_server *srv)
{
	unv->server = srv;
	wl_list_init(&unv->atoms);

	nuclear_atom *atoms = malloc(sizeof(nuclear_atom)*3);
	for(int i=0;i<3;i++)
	{
		nuclear_atom_init(&atoms[i], unv, 0.f, 100.f*(i+2), 0.f);
		wl_list_insert(&unv->atoms, &atoms[i].next);
	}
	atoms[2].position[0] = 100.f;
}
void nuclear_universe_destroy(nuclear_universe *unv)
{
	nuclear_atom *atom;
	wl_list_for_each_reverse(atom, &unv->atoms, next) {
		nuclear_atom_destroy(atom);
	}
}
