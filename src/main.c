#include "../include/ft_ping.h"

int	main(int argc, char **argv)
{
	t_ping ping;
	t_reply reply;

	if (init_ping(&ping, argc, argv))
		return (EXIT_FAILURE);

	print_header(&ping);

	//printf("Raw socket created successfully. fd: %d\n", ping.socket.fd);

	memset(&reply, 0, sizeof(reply));

	while (1)
	{
		if (ping_once(&ping, &reply))
		{
			fprintf(stderr, "ping_once: failed\n");
			return (EXIT_FAILURE);
		}

		ping.packet.sequence++;

		sleep(1);
	}

	if (ping.socket.is_open)
		close(ping.socket.fd);

	return (EXIT_SUCCESS);
}

void	print_header(t_ping *ping)
{
	printf("PING %s (%s) 56(84) bytes of data.\n", ping->destination.hostname, ping->destination.ip);
}
