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

typedef enum e_icmp_result
{
	ICMP_REPLY,
	ICMP_DEST_UNREACHABLE,
	ICMP_TUME_EXCEEDED,
	ICMP_OTHER
}	t_icmp_result;

typedef enum e_validate_reply
{
	VALID_REPLY,
	NOT_OUR_REPLY,
	INVALID_CHECKSUM
}	t_validate_reply;

typedef enum e_receive_result
{
	RECEIVE_OK,
	RECEIVE_TIMEOUT,
	RECEIVE_INTERRUPTED,
	RECEIVE_ERROR
}	t_receive_result;

typedef enum e_ping_reply
{
	PING_REPLY,
	PING_TIMEOUT,
	PING_INTERRUPTED,
	PING_ERROR
}	t_ping_reply;

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
}	t_socket;

typedef struct s_packet
{
	uint16_t		identifier;
	uint16_t		sequence;
	struct timeval	timestamp;
}	t_packet;

typedef struct stats
{
	uint64_t	transmitted;
	uint64_t	received;
	int64_t		min_rtt;
	int64_t		max_rtt;
	int64_t		total_rtt;
}	t_stats;

typedef struct s_ping
{
	t_options		options;
	t_destination	destination;
	t_socket		socket;
	t_packet		packet;
	t_stats			stats;
}	t_ping;

typedef struct s_reply
{
	unsigned char		buffer[1500];
	ssize_t				bytes_received;
	struct iphdr		*ip;
	struct icmphdr		*icmp;
	struct sockaddr_in	sender;
	struct hostent		*src_hostname;
	struct timeval		timestamp;
	char				src_ip[INET_ADDRSTRLEN];
}	t_reply;

//			init.c
int					init_ping(t_ping *ping, int argc, char **argv);
int					parse_options(t_ping *ping, int argc, char **argv, int *dest_index);
int					resolve_destination(t_ping *ping);
int					create_socket(t_ping *ping);

//			ping.c
t_ping_reply		ft_ping(t_ping *ping, t_reply *reply);

//			send.c
int					send_echo_request(t_ping *ping);
int					build_echo_request(t_ping *ping, unsigned char *buffer);

//			receive.c
t_receive_result	receive_packet(t_ping *ping, t_reply *reply, struct timeval *timeout);
int					parse_packet(t_reply *reply);
t_validate_reply	validate_reply(t_ping *ping, t_reply *reply);

//			signals.c
int					setup_signal_handler(void);

//			utils.c
void				close_socket(t_ping *ping);
uint16_t			calculate_checksum(void *data, size_t len);
int					calculate_remaining_time(struct timeval deadline, struct timeval *remaining);
void				update_stats(t_ping *ping, int64_t rtt_us);
void				print_usage(void);
void				print_header(t_ping *ping);
void				print_reply(t_ping *ping, t_reply *reply, int64_t rtt_us);
void				print_stats(t_ping *ping);

#endif
