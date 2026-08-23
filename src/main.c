#include "../include/ft_ping.h"

int	main(int argc, char **argv)
{
	t_ping ping;
	t_reply reply;

	if (setup_signal_handler())
		return (EXIT_FAILURE);

	if (init_ping(&ping, argc, argv))
		return (EXIT_FAILURE);

	print_header(&ping);

	memset(&reply, 0, sizeof(reply));

	while (running)
	{
		switch (ft_ping(&ping, &reply))
		{
			case PING_SUCCESS:
				break;
			case PING_TIMEOUT:
				break;
			case PING_INTERRUPTED:
				break;
			case PING_ERROR:
				close_socket(&ping);
				return (EXIT_FAILURE);
		}

		ping.packet.sequence++;

		sleep(1);
	}

	print_stats(&ping);

	close_socket(&ping);

	return (EXIT_SUCCESS);
}
