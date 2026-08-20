#include "../include/ft_ping.h"

int	send_echo_request(t_ping *ping)
{
	unsigned char buffer[PACKET_SIZE];

	if (build_echo_request(ping, buffer))
	{
		fprintf(stderr, "send_echo_request: build_echo_request faild\n");
		return (1);
	}

	if (sendto(ping->socket.fd, buffer, PACKET_SIZE, 0, (struct sockaddr *)&ping->destination.addr, sizeof(ping->destination.addr)) == -1)
	{
		perror("sendto");
		return (1);
	}

	ping->stats.transmitted++;

	return (0);
}

int	build_echo_request(t_ping *ping, unsigned char *buffer)
{
	struct icmphdr *icmp;

	memset(buffer, 0, PACKET_SIZE);

	icmp = (struct icmphdr *)buffer;

	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = htons(ping->packet.identifier);
	icmp->un.echo.sequence = htons(ping->packet.sequence);

	gettimeofday(&ping->packet.timestamp, NULL);

	memcpy(buffer + sizeof(struct icmphdr), &ping->packet.timestamp, sizeof(struct timeval));

	icmp->checksum = 0;
	icmp->checksum = calculate_checksum(buffer, PACKET_SIZE);

	return (0);
}
