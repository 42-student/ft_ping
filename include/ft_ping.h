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
# include <netinet/ip_icmp.h>
# include <sys/time.h>
# include <stdint.h>
# include <inttypes.h>
# include <signal.h>
# include <errno.h>
# include <sys/select.h>

# define PACKET_SIZE 64
# define PAYLOAD_SIZE 56

extern volatile sig_atomic_t running;

typedef enum e_receive_result
{
	RECEIVE_OK,
	RECEIVE_TIMEOUT,
	RECEIVE_INTERRUPTED,
	RECEIVE_ERROR
}	t_receive_result;

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
	uint16_t		identifier;
	uint16_t		sequence;
	struct timeval	timestamp;
} t_packet;

typedef struct s_ping
{
	t_options		options;
	t_destination	destination;
	t_socket		socket;
	t_packet		packet;
}	t_ping;

typedef struct s_reply
{
	unsigned char		buffer[1500];
	ssize_t				bytes_received;
	struct iphdr		*ip;
	struct icmphdr		*icmp;
	struct sockaddr_in	sender;
	struct timeval		timestamp;
	
}	t_reply;

int					init_ping(t_ping *ping, int argc, char **argv);
int					resolve_destination(t_ping *ping);
int					create_socket(t_ping *ping);
int					build_echo_request(t_ping *ping, unsigned char *buffer);
uint16_t			calculate_checksum(void *data, size_t len);
int					send_echo_request(t_ping *ping);
t_receive_result	receive_packet(t_ping *ping, t_reply *reply);
int					parse_packet(t_reply *reply);
int					validate_reply(t_ping *ping, t_reply *reply);
int					ping_once(t_ping *ping, t_reply *reply);
void				print_reply(t_ping *ping, t_reply *reply, int64_t rtt_us);
void				print_header(t_ping *ping);
int					setup_signal_handler(void);

#endif
