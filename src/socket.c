#include "../include/ft_ping.h"

int	create_socket(t_ping *ping)
{
	ping->socket.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if (ping->socket.fd == -1)
	{
		perror("socket");
		return (1);
	}

	ping->socket.is_open = true;

	return (0);
}
