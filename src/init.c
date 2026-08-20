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

	ping->destination.hostname = argv[dest_index];
	ping->packet.identifier = getpid();
	ping->packet.sequence = 1;

	if (resolve_destination(ping))
		return (1);

	if (create_socket(ping))
		return (1);

	return (0);
}

int	parse_options(t_ping *ping, int argc, char **argv, int *dest_index)
{
	int i = 1;

	while (i < argc)
	{
		if (argv[i][0] != '-')
			break;
		if (strcmp(argv[i], "-v") == 0)
			ping->options.verbose = true;
		else if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			ping->options.help = true;
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

void	print_usage(void)
{
	printf("   Usage:\n");
	printf("   ------\n");
	printf("   ft_ping <destination>\n");
	printf("   ft_ping [-v] <destination>      Verbose output.\n");
	printf("   ft_ping [-?] [-h] [--help]      Show this help message.\n");
}
