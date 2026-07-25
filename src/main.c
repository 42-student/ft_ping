#include "../include/ft_ping.h"

int	main(int argc, char **argv)
{
	t_ping ping;

	if (init_ping(&ping, argc, argv))
		return (EXIT_FAILURE);

	printf("PING %s (%s)\n", ping.destination.hostname, ping.destination.ip);

	printf("Raw socket created successfully. fd: %d\n", ping.socket.fd);

	if (ping.socket.is_open)
		close(ping.socket.fd);

	return (EXIT_SUCCESS);
}
