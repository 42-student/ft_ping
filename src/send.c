#include "../include/ft_ping.h"

int	send_echo_request(t_ping *ping)
{
	unsigned char buffer[PACKET_SIZE];

	build_echo_request(ping, buffer);

	if (sendto(ping->socket.fd, buffer, PACKET_SIZE, 0, (struct sockaddr *)&ping->destination.addr, sizeof(ping->destination.addr)) == -1)
	{
		perror("sendto");
		return (1);
	}

	return (0);
}
