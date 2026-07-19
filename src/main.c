#include "../include/ft_ping.h"

int	main(int argc, char **argv)
{
	t_ping ping;

	if (init_ping(&ping, argc, argv))
		return (EXIT_FAILURE);

	printf("PING %s (%s)\n", ping.destination.hostname, ping.destination.ip);

	return (EXIT_SUCCESS);
}
