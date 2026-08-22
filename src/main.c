#include "../include/ft_ping.h"

int	main(int argc, char **argv)
{
	t_ping ping;
	t_reply reply;
	struct timeval start_time;
	struct timeval end_time;

	if (setup_signal_handler())
		return (EXIT_FAILURE);

	if (init_ping(&ping, argc, argv))
		return (EXIT_FAILURE);

	gettimeofday(&start_time, NULL);

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

	gettimeofday(&end_time, NULL);

	int64_t elapsed_us = (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_usec - start_time.tv_usec);
	int64_t elapsed_ms = elapsed_us / 1000;

	printf("\n");
	print_stats(&ping, elapsed_ms);

	close_socket(&ping);

	return (EXIT_SUCCESS);
}
