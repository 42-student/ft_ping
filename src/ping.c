#include "../include/ft_ping.h"

int	ping_once(t_ping *ping, t_reply *reply)
{
	int64_t rtt_us;

	if (send_echo_request(ping))
	{
		fprintf(stderr, "ping_once: echo request failed\n");
		return (1);
	}

	switch (receive_packet(ping, reply))
	{
		case RECEIVE_OK:
			ping->stats.received++;
			break;
		case RECEIVE_TIMEOUT:
			printf("Request timeout for icmp_seq %d\n", ping->packet.sequence);
			return (0);
		case RECEIVE_INTERRUPTED:
			return (0);
		case RECEIVE_ERROR:
			fprintf(stderr, "ping_once: receive packet failed\n");
			return (1);
	}

	if (parse_packet(reply))
	{
		fprintf(stderr, "ping_once: parse packet failed\n");
		return (1);
	}

	if (validate_reply(ping, reply))
	{
		fprintf(stderr, "ping_once: validate reply failed\n");
		return (1);
	}

	rtt_us = (reply->timestamp.tv_sec - ping->packet.timestamp.tv_sec) * 1000000L + (reply->timestamp.tv_usec - ping->packet.timestamp.tv_usec);

	update_stats(ping, rtt_us);

	print_reply(ping, reply, rtt_us);

	return (0);
}

void	update_stats(t_ping *ping, int64_t rtt_us)
{
	if (ping->stats.min_rtt == 0 || ping->stats.min_rtt > rtt_us)
		ping->stats.min_rtt = rtt_us;

	if (ping->stats.max_rtt == 0 || ping->stats.max_rtt < rtt_us)
		ping->stats.max_rtt = rtt_us;

	ping->stats.total_rtt += rtt_us;
}

void	print_reply(t_ping *ping, t_reply *reply, int64_t rtt_us)
{
	double rtt_ms = (double)rtt_us / 1000.0;

	printf("%zd bytes from %s: icmp_seq=%d ttl=%u time=%.2f ms\n", reply->bytes_received, ping->destination.ip, ping->packet.sequence, reply->ip->ttl, rtt_ms);
}
