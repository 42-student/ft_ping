#include "../include/ft_ping.h"

t_ping_reply	ft_ping(t_ping *ping, t_reply *reply)
{
	int64_t rtt_us;
	struct timeval deadline, remaining;

	if (send_echo_request(ping))
	{
		fprintf(stderr, "ping_once: echo request failed\n");
		return (PING_ERROR);
	}

	deadline.tv_sec = ping->packet.timestamp.tv_sec + 1;
	deadline.tv_usec = ping->packet.timestamp.tv_usec;

	while (true)
	{
		if (calculate_remaining_time(deadline, &remaining))
		{
			if (ping->options.verbose)
				printf("Request timeout for icmp_seq %d\n", ping->packet.sequence);
			return (PING_TIMEOUT);
		}

		switch (receive_packet(ping, reply, &remaining))
		{
			case RECEIVE_OK:
				break;
			case RECEIVE_TIMEOUT:
				if (ping->options.verbose)
					printf("Request timeout for icmp_seq %d\n", ping->packet.sequence);
				return (PING_TIMEOUT);
			case RECEIVE_INTERRUPTED:
				return (PING_INTERRUPTED);
			case RECEIVE_ERROR:
				if (ping->options.verbose)
					fprintf(stderr, "ping_once: receive packet failed\n");
				return (PING_ERROR);
		}

		if (parse_packet(ping, reply))
			continue;

		switch (validate_reply(ping, reply))
		{
			case VALID_REPLY:
				break;
			case NOT_OUR_REPLY:
				continue;
			case INVALID_CHECKSUM:
				continue;
			case ICMP_ERROR_PACKET:
				break;
		}

		break;
	}

	ping->stats.received++;

	rtt_us = (reply->timestamp.tv_sec - ping->packet.timestamp.tv_sec) * 1000000L + (reply->timestamp.tv_usec - ping->packet.timestamp.tv_usec);

	update_stats(ping, rtt_us);

	get_src_hostname(ping, reply);

	print_reply(ping, reply, rtt_us);

	return (0);
}
