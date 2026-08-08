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

	return (0);
}
