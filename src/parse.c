#include "../include/ft_ping.h"

int	parse_packet(t_reply *reply)
{
	int iph_size;

	reply->ip = (struct iphdr *)reply->buffer;

	iph_size = reply->ip->ihl * 4;

	reply->icmp = (struct icmphdr *)(reply->buffer + iph_size);

	return (0);
}
