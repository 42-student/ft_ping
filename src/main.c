#include "../include/ft_ping.h"

int	main(int argc, char **argv)
{
	t_ping ping;
	t_reply reply;

	if (init_ping(&ping, argc, argv))
		return (EXIT_FAILURE);

	printf("PING %s (%s)\n", ping.destination.hostname, ping.destination.ip);

	//printf("Raw socket created successfully. fd: %d\n", ping.socket.fd);

	memset(&reply, 0, sizeof(reply));

	if (ping_once(&ping, &reply))
	{
		fprintf(stderr, "ping_once: failed\n");
		return (EXIT_FAILURE);
	}

	if (ping.socket.is_open)
		close(ping.socket.fd);

	return (EXIT_SUCCESS);
}
