#include "../include/ft_ping.h"

int	build_echo_request(t_ping *ping, unsigned char *buffer)
{
	struct icmphdr *icmp;

	memset(buffer, 0, 64);

	icmp = (struct icmphdr *)buffer;

	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = htons(getpid());
	icmp->un.echo.sequence = htons(ping->packet.sequence);

	/*
		payload here

		memcpy(buffer + sizeof(struct icmphdr), "ft_ping", 7);
		// or we can store timestamps in here
	*/

	icmp->checksum = checksum(buffer, 64);

	return (0);
}
