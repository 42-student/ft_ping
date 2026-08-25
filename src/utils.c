#include "../include/ft_ping.h"

void	close_socket(t_ping *ping)
{
	if (ping->socket.is_open)
	{
		close(ping->socket.fd);
		ping->socket.is_open = false;
	}
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
	printf("   ft_ping [-v] <destination>                 Verbose output.\n");
	printf("   ft_ping [-?] [-h] [--help]                 Show this help message.\n");
	printf("   ft_ping [--ttl=<value>] <destination>      Set IP time-to-live (1-255).\n");
}

void	print_header(t_ping *ping)
{
	printf("PING %s (%s): 56 data bytes", ping->destination.hostname, ping->destination.ip);

	if (ping->options.verbose)
		printf(", id 0x%04x = %u", ping->packet.identifier & 0xffff, ping->packet.identifier & 0xffff);

	printf("\n");
}

void	print_reply(t_ping *ping, t_reply *reply, int64_t rtt_us)
{
	double rtt_ms = (double)rtt_us / 1000.0;

	printf("%zd bytes from %s: icmp_seq=%d ttl=%u time=%.3f ms\n", reply->bytes_received - (reply->ip->ihl * 4), reply->src_ip, ping->packet.sequence, reply->ip->ttl, rtt_ms);
}

void	print_stats(t_ping *ping)
{
	double min_rtt_ms = (double)ping->stats.min_rtt / 1000.0;
	double max_rtt_ms = (double)ping->stats.max_rtt / 1000.0;
	double total_rtt_ms = (double)ping->stats.total_rtt / 1000.0;
	double loss_percentage = ((double)(ping->stats.transmitted - ping->stats.received) / (double)ping->stats.transmitted) * 100;

	if (ping->stats.received == 0)
		printf("--- %s ping statistics ---\n%" PRIu64 " packets transmitted, %" PRIu64 " packets received, %.f%% packet loss\n",
			ping->destination.hostname, ping->stats.transmitted, ping->stats.received, loss_percentage);
	else
		printf("--- %s ping statistics ---\n%" PRIu64 " packets transmitted, %" PRIu64 " packets received, %.f%% packet loss\nround-trip min/avg/max = %.3f/%.3f/%.3f ms\n",
			ping->destination.hostname, ping->stats.transmitted, ping->stats.received, loss_percentage, min_rtt_ms, (total_rtt_ms / ping->stats.received), max_rtt_ms);
}

void	print_verbose_icmp_error(struct iphdr *ip, struct icmphdr *icmp)
{
	unsigned char *raw;
	int header_len;
	size_t icmp_size;
	char src_addr[INET_ADDRSTRLEN];
	char dst_addr[INET_ADDRSTRLEN];

	if (inet_ntop(AF_INET, &ip->saddr, src_addr, INET_ADDRSTRLEN) == NULL)
	{
		perror("inet_ntop in print_verbose_icmp_error");
		return;
	}

	if (inet_ntop(AF_INET, &ip->daddr, dst_addr, INET_ADDRSTRLEN) == NULL)
	{
		perror("inet_ntop in print_verbose_icmp_error");
		return;
	}

	printf("IP Hdr Dump:\n");

	raw = (unsigned char *)ip;
	header_len = ip->ihl * 4;

	printf(" ");
	for (int i = 0; i < header_len; i += 2)
	{
		printf("%02x%02x", raw[i], raw[i + 1]);

		if (i + 2 < header_len)
			printf(" ");
	}
	printf("\n");

	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data\n");

	printf(" %u  %u  %02x %04x %04x   %u %04x  %02x  %02x %04x %s  %s\n",
		ip->version, ip->ihl, ip->tos, ntohs(ip->tot_len), ntohs(ip->id), ntohs(ip->frag_off) >> 13, ntohs(ip->frag_off) & 0x1fff, ip->ttl, ip->protocol, ntohs(ip->check), src_addr, dst_addr);

	icmp_size = ntohs(ip->tot_len) - (ip->ihl * 4);

	printf("ICMP: type %u, code %u, size %zd, id 0x%04x, seq 0x%04x\n",
		icmp->type, icmp->code, icmp_size, ntohs(icmp->un.echo.id), ntohs(icmp->un.echo.sequence));
}
