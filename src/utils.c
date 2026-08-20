#include "../include/ft_ping.h"

void	close_socket(t_ping *ping)
{
	if (ping->socket.is_open)
	close(ping->socket.fd);
}

uint16_t	calculate_checksum(void *data, size_t len)
{
	uint32_t sum;
	uint16_t *ptr;

	sum = 0;
	ptr = (uint16_t *)data;

	while (len > 1)
	{
		sum += *ptr++;
		len -= 2;
	}

	if (len == 1)
		sum += *(uint8_t *)ptr;
	
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ((uint16_t)~sum);
}

int	calculate_remaining_time(struct timeval deadline, struct timeval *remaining)
{
	struct timeval current;

	gettimeofday(&current, NULL);

	if (current.tv_sec > deadline.tv_sec || (current.tv_sec == deadline.tv_sec && current.tv_usec >= deadline.tv_usec))
		return (1);

	remaining->tv_sec = deadline.tv_sec - current.tv_sec;
	remaining->tv_usec = deadline.tv_usec - current.tv_usec;

	if (remaining->tv_usec < 0)
	{
		remaining->tv_sec--;
		remaining->tv_usec += 1000000;
	}

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

void	print_usage(void)
{
	printf("   Usage:\n");
	printf("   ------\n");
	printf("   ft_ping <destination>\n");
	printf("   ft_ping [-v] <destination>      Verbose output.\n");
	printf("   ft_ping [-?] [-h] [--help]      Show this help message.\n");
}

void	print_header(t_ping *ping)
{
	printf("PING %s (%s) 56(84) bytes of data.\n", ping->destination.hostname, ping->destination.ip);
}

void	print_reply(t_ping *ping, t_reply *reply, int64_t rtt_us)
{
	double rtt_ms = (double)rtt_us / 1000.0;

	printf("%zd bytes from %s: icmp_seq=%d ttl=%u time=%.2f ms\n", reply->bytes_received, reply->src_ip, ping->packet.sequence, reply->ip->ttl, rtt_ms);
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
