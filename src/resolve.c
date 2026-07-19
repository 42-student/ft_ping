#include "../include/ft_ping.h"

int	resolve_destination(t_ping *ping)
{
	struct addrinfo hints;
	struct addrinfo *result;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;

	if (getaddrinfo(ping->destination.hostname, NULL, &hints, &result) != 0)
	{
		fprintf(stderr, "Failed to resolve %s\n", ping->destination.hostname);
		return (1);
	}

	ping->destination.addr = *(struct sockaddr_in *)result->ai_addr;

	inet_ntop(AF_INET, &ping->destination.addr.sin_addr, ping->destination.ip, sizeof(ping->destination.ip));

	freeaddrinfo(result);

	return (0);
}
