#include "../include/ft_ping.h"

int	init_ping(t_ping *ping, int argc, char **argv)
{
	memset(ping, 0, sizeof(*ping));

	if (argc != 2)
	{
		fprintf(stderr, "usage: ft_ping <destination>\n");
		return (1);
	}

	ping->destination.hostname = argv[1];
	ping->packet.identifier = getpid();

	if (resolve_destination(ping))
		return (1);

	if (create_socket(ping))
		return (1);

	return (0);
}
