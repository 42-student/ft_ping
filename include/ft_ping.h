#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdbool.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netdb.h>
# include <unistd.h>

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

typedef struct s_socket
{
	int		fd;
	bool	is_open;
} t_socket;

typedef struct s_packet
{
	uint16_t		sequence;
	uint16_t		identifier;
	struct timeval	timestamp;
} t_packet;

typedef struct s_ping
{
	t_options		options;
	t_destination	destination;
	t_socket		socket;
	t_packet		packet;
}	t_ping;

int	init_ping(t_ping *ping, int argc, char **argv);
int	resolve_destination(t_ping *ping);
int	create_socket(t_ping *ping);
int	build_echo_request(t_ping *ping, char *buffer);

#endif
