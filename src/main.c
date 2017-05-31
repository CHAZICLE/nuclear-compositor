#include "nuclear_server.h"
#include "nuclear_texture.h"
#include <stdio.h>

#include "main.h"

nuclear_server *default_server = 0;

int main()
{
	init_egl();

	nuclear_server nc;
	default_server = &nc;
	fprintf(stderr, "Initializing nuclear server..\n");
	nuclear_server_init(&nc);
	fprintf(stderr, "Running nuclear server..\n");
	nuclear_server_run(&nc);
	fprintf(stderr, "Destroying nuclear server..\n");
	nuclear_server_destroy(&nc);
	default_server = 0;
	return 0;
}
