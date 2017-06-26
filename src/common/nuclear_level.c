#include "nuclear_surface.h"

#include "nuclear_level.h"

void nuclear_level_init(nuclear_level *lvl, nuclear_atom *atm, float radius)
{
	lvl->atom = atm;
	lvl->radius = radius;

	wl_list_init(&lvl->surfaces);
}
void nuclear_level_destroy(nuclear_level *lvl)
{
	
}
