#include "../include/ft_ping.h"

int	resolve_destination(t_ping *ping)
{
	struct addrinfo hints;
	struct addrinfo *result;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	int ret = getaddrinfo(ping->destination.hostname, NULL, &hints, &result);

	if (ret != 0)
	{
		fprintf(stderr, "%s: %s\n", ping->destination.hostname, gai_strerror(ret));
		return (1);
	}

	ping->destination.addr = *(struct sockaddr_in *)result->ai_addr;

	if (inet_ntop(AF_INET, &ping->destination.addr.sin_addr, ping->destination.ip, sizeof(ping->destination.ip)) == NULL)
	{
		perror("inet_ntop");
		freeaddrinfo(result);
		return (1);
	}

	freeaddrinfo(result);

	return (0);
}
