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

	//printf("Raw socket created successfully. fd: %d\n", ping.socket.fd);

	memset(&reply, 0, sizeof(reply));

	while (running)
	{
		switch (ping_once(&ping, &reply))
		{
			case PING_REPLY:
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

	printf("\n");
	print_stats(&ping);

	close_socket(&ping);

	return (EXIT_SUCCESS);
}

void	close_socket(t_ping *ping)
{
	if (ping->socket.is_open)
		close(ping->socket.fd);
}

void	print_stats(t_ping *ping)
{
	double min_rtt_ms = (double)ping->stats.min_rtt / 1000.0;
	double max_rtt_ms = (double)ping->stats.max_rtt / 1000.0;
	double total_rtt_ms = (double)ping->stats.total_rtt / 1000.0;
	double loss_percentage = ((double)(ping->stats.transmitted - ping->stats.received) / (double)ping->stats.transmitted) * 100;

	if (ping->stats.received == 0)
		printf("--- %s ping statistics ---\n%" PRIu64 " packets transmitted, %" PRIu64 " received, %.1f%% packet loss\n", 
			ping->destination.hostname, ping->stats.transmitted, ping->stats.received, loss_percentage);
	else
		printf("--- %s ping statistics ---\n%" PRIu64 " packets transmitted, %" PRIu64 " received, %.1f%% packet loss\nrtt min/avg/max = %.3f/%.3f/%.3f ms\n", 
			ping->destination.hostname, ping->stats.transmitted, ping->stats.received, loss_percentage, min_rtt_ms, (total_rtt_ms / ping->stats.received), max_rtt_ms);
}

void	print_header(t_ping *ping)
{
	printf("PING %s (%s) 56(84) bytes of data.\n", ping->destination.hostname, ping->destination.ip);
}
