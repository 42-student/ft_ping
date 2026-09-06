#include "../include/ft_ping.h"

int	init_ping(t_ping *ping, int argc, char **argv)
{
	int dest_index;

	memset(ping, 0, sizeof(*ping));

	if (parse_options(ping, argc, argv, &dest_index))
		return (1);

	if (ping->options.help)
	{
		print_usage();
		exit(EXIT_SUCCESS);
	}

	if (argc < 2)
	{
		fprintf(stderr, "help menu: ft_ping [-?]\n");
		return (1);
	}

	if (dest_index >= argc)
	{
		fprintf(stderr, "ft_ping: missing host operand\n");
		fprintf(stderr, "Try 'ping --help' for more information.\n");
		return (1);
	}

	ping->destination.hostname = argv[dest_index];
	ping->packet.identifier = getpid();

	if (resolve_destination(ping))
		return (1);

	if (create_socket(ping))
		return (1);

	return (0);
}

int	parse_options(t_ping *ping, int argc, char **argv, int *dest_index)
{
	int i = 1;
	char *value;
	char *end;
	long ttl;

	while (i < argc)
	{
		if (argv[i][0] != '-')
			break;

		if (strcmp(argv[i], "-v") == 0)
			ping->options.verbose = true;
		else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0)
			ping->options.quiet = true;
		else if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			ping->options.help = true;
		else if (strncmp(argv[i], "--ttl=", 6) == 0)
		{
			value = argv[i] + 6;

			if (*value == '\0')
			{
				fprintf(stderr, "ft_ping: invalid TTL: %s\n", value);
				return (1);
			}

			errno = 0;
			ttl = strtol(value, &end, 10);

			if (errno == ERANGE || *end != '\0' || ttl < 1	|| ttl > 255)
			{
				fprintf(stderr, "ft_ping: invalid TTL: %s\n", value);
				return (1);
			}

			ping->options.ttl = (int)ttl;
		}
		else
		{
			fprintf(stderr, "ft_ping: unknown option: %s\n", argv[i]);
			return (1);
		}

		i++;
	}

	*dest_index = i;

	return (0);
}

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
		perror("inet_ntop in resolve_destination");
		freeaddrinfo(result);
		return (1);
	}

	freeaddrinfo(result);

	return (0);
}

int	create_socket(t_ping *ping)
{
	ping->socket.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if (ping->socket.fd == -1)
	{
		perror("socket");
		return (1);
	}

	int broadcast = 1;

	if (setsockopt(ping->socket.fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
	{
		perror("setsockopt SO_BROADCAST");
		close(ping->socket.fd);
		ping->socket.is_open = false;
		return (1);
	}

	if (ping->options.ttl)
	{
		if (setsockopt(ping->socket.fd, IPPROTO_IP, IP_TTL, &ping->options.ttl, sizeof(ping->options.ttl)) == -1)
		{
			perror("setsockopt IP_TTL");
			close(ping->socket.fd);
			return (1);
		}
	}

	ping->socket.is_open = true;

	return (0);
}
