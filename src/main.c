#include "nuclear_server.h"
#include "nuclear_texture.h"
#include <stdio.h>
#include <unistd.h>

#include "main.h"

nuclear_server *default_server = 0;

int main(int argc, char **argv)
{
	bool enableOSVR = true;
	char c;
	while ((c = getopt(argc, argv, "d")) != -1)
		switch(c)
		{
			case 'd':
				enableOSVR = false;
				break;
		}

	init_egl();

	nuclear_server nc;
	default_server = &nc;
	fprintf(stderr, "Initializing nuclear server..\n");
	nuclear_server_init(&nc, enableOSVR);
	fprintf(stderr, "Running nuclear server..\n");
	nuclear_server_run(&nc);
	fprintf(stderr, "Destroying nuclear server..\n");
	nuclear_server_destroy(&nc);
	default_server = 0;
	return 0;
}
