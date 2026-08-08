#include "../include/ft_ping.h"

int	parse_packet(t_reply *reply)
{
	size_t iph_size;

	if (reply->bytes_received < 20)
	{
		fprintf(stderr, "parse_packet: truncated IPv4 packet (expected at least 20 bytes)\n");
		return (1);
	}

	reply->ip = (struct iphdr *)reply->buffer;

	if (reply->ip->version != 4)
	{
		fprintf(stderr, "parse_packet: unsupported IP version (expected IPv4)\n");
		return (1);
	}

	iph_size = reply->ip->ihl * 4;

	if (iph_size < 20 || iph_size > 60)
	{
		fprintf(stderr, "parse_packet: invalid IPv4 header length\n");
		return (1);
	}

	if (reply->bytes_received < iph_size)
	{
		fprintf(stderr, "parse_packet: truncated IPv4 header\n");
		return (1);
	}

	if ((reply->bytes_received - iph_size) < 8)
	{
		fprintf(stderr, "parse_packet: truncated ICMP header\n");
		return (1);
	}

	reply->icmp = (struct icmphdr *)(reply->buffer + iph_size);

	return (0);
}
