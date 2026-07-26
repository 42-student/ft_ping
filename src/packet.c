#include "../include/ft_ping.h"

int	build_echo_request(t_ping *ping, unsigned char *buffer)
{
	struct icmphdr *icmp;
	struct timeval tv;

	memset(buffer, 0, PACKET_SIZE);

	icmp = (struct icmphdr *)buffer;

	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = htons(getpid());
	icmp->un.echo.sequence = htons(ping->packet.sequence);

	gettimeofday(&tv, NULL);

	memcpy(buffer + sizeof(struct icmphdr), &tv, sizeof(tv));

	icmp->checksum = 0;
	icmp->checksum = calculate_checksum(buffer, PACKET_SIZE);

	return (0);
}
