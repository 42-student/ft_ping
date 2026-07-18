#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

int	main(int argc, char **argv)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct sockaddr_in *addr;
	char ip[INET_ADDRSTRLEN];

	if (argc != 2)
	{
		fprintf(stderr, "usage: ft_ping <destination>\n");
		return EXIT_FAILURE;
	}

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;

	if (getaddrinfo(argv[1], NULL, &hints, &result) != 0)
	{
		fprintf(stderr, "Failed to resolve %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	addr = (struct sockaddr_in *)result->ai_addr;

	inet_ntop(
		AF_INET,
		&addr->sin_addr,
		ip,
		sizeof(ip)
	);

	printf("Rezolved address: %s\n", ip);

	freeaddrinfo(result);

	return EXIT_SUCCESS;
}

