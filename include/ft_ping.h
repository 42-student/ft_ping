#ifndef FT_PING_H
# define FT_PING_H

# include <stdlib.h>
# include <stdbool.h>
# include <arpa/inet.h>
# include <netinet/in.h>

typedef struct s_options
{
	bool	verbose;
	bool	help;
}	t_options;

typedef struct s_destination
{
	char				*hostname;
	char				ip[INET_ADDRSTRLEN];
	struct sockaddr_in	addr;
}	t_destination;

typedef struct s_ping
{
	t_options		options;
	t_destination	destination;

	int				sockfd;
}	t_ping;

int	init_ping(t_ping *ping, int argc, char **argv);
int	resolve_destination(t_ping *ping);

#endif
