#include "../include/ft_ping.h"

struct icmp_error_message
{
	int			type;
	int			code;
	const char 	*message;
};

static const struct icmp_error_message icmp_errors[] = {
	{ ICMP_DEST_UNREACH, ICMP_NET_UNREACH, "Destination Network Unreachable" },
	{ ICMP_DEST_UNREACH, ICMP_HOST_UNREACH, "Destination Host Unreachable" },
	{ ICMP_DEST_UNREACH, ICMP_PROT_UNREACH, "Destination Protocol Unreachable" },
	{ ICMP_DEST_UNREACH, ICMP_PORT_UNREACH, "Destination Port Unreachable" },
	{ ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED, "Destination Fragmentation Needed And DF Set" },
	{ ICMP_DEST_UNREACH, ICMP_SR_FAILED, "Destination Source Route Failed" },
	{ ICMP_DEST_UNREACH, ICMP_NET_UNKNOWN, "Destination Network Unknown" },
	{ ICMP_DEST_UNREACH, ICMP_HOST_UNKNOWN, "Destination Host Unknown" },
	{ ICMP_DEST_UNREACH, ICMP_HOST_ISOLATED, "Destination Source Host Isolated" },
	{ ICMP_DEST_UNREACH, ICMP_NET_ANO, "Destination Network Administratively Prohibited" },
	{ ICMP_DEST_UNREACH, ICMP_HOST_ANO, "Destination Host Administratively Prohibited" },
	{ ICMP_DEST_UNREACH, ICMP_NET_UNR_TOS, "Destination Network Unreachable For TOS" },
	{ ICMP_DEST_UNREACH, ICMP_HOST_UNR_TOS, "Destination Host Unreachable For TOS" },
	{ ICMP_DEST_UNREACH, ICMP_PKT_FILTERED, "Destination Communication Administratively Prohibited" },
	{ ICMP_DEST_UNREACH, ICMP_PREC_VIOLATION, "Destination Host Precedence Violation" },
	{ ICMP_DEST_UNREACH, ICMP_PREC_CUTOFF, "Destination Precedence Cutoff In Effect" },
	/* ... */
	{ ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, "Time To Live Exceeded In Transit" },
	{ ICMP_TIME_EXCEEDED, ICMP_EXC_FRAGTIME, "Fragment Reassembly Time Exceeded" },
};

t_receive_result	receive_packet(t_ping *ping, t_reply *reply, struct timeval *timeout)
{
	fd_set readfds;
	socklen_t sender_len;

	FD_ZERO(&readfds);
	FD_SET(ping->socket.fd, &readfds);

	int res = select(ping->socket.fd + 1, &readfds, NULL, NULL, timeout);

	if (res == -1)
	{
		if (errno == EINTR)
			return (RECEIVE_INTERRUPTED);
		perror("select");
			return (RECEIVE_ERROR);
	}
	else if (res == 0)
		return (RECEIVE_TIMEOUT);

	if (!FD_ISSET(ping->socket.fd, &readfds))
		return (RECEIVE_ERROR);

	sender_len = sizeof(reply->sender);

	reply->bytes_received = recvfrom(ping->socket.fd, reply->buffer, RECV_BUFFER_SIZE, 0, (struct sockaddr *)&reply->sender, &sender_len);

	if (reply->bytes_received < 0)
	{
		if (errno == EINTR)
			return (RECEIVE_INTERRUPTED);

		perror("recvfrom");
			return (RECEIVE_ERROR);
	}

	gettimeofday(&reply->timestamp, NULL);

	return (RECEIVE_OK);
}

int	parse_packet(t_reply *reply)
{
	ssize_t iph_size;
	uint16_t ip_tot_len;

	if (reply->bytes_received < 20)
	{
		fprintf(stderr, "parse_packet: truncated IPv4 packet (expected at least 20 bytes)\n");
		return (1);
	}

	reply->ip = (struct iphdr *)reply->buffer;
	iph_size = reply->ip->ihl * 4;
	ip_tot_len = ntohs(reply->ip->tot_len);

	if (ip_tot_len <= iph_size)
	{
		fprintf(stderr, "parse_packet: IPv4 total length < IPv4 header length\n");
		return (1);
	}

	if ((size_t)reply->bytes_received < ip_tot_len)
	{
		fprintf(stderr, "parse_packet: received %ld bytes, expected %d\n", reply->bytes_received, ip_tot_len);
		return (1);
	}

	if (reply->ip->version != 4)
	{
		fprintf(stderr, "parse_packet: unsupported IP version (expected IPv4)\n");
		return (1);
	}

	if (reply->ip->protocol != IPPROTO_ICMP)
	{
		fprintf(stderr, "parse_packet: IP protocol must be ICMP");
		return (1);
	}

	if (iph_size < 20 || iph_size > 60)
	{
		fprintf(stderr, "parse_packet: invalid IPv4 header length\n");
		return (1);
	}

	if (reply->bytes_received < iph_size)
	{
		fprintf(stderr, "parse_packet: truncated IPv4 header\n");
		return (1);
	}

	if ((reply->bytes_received - iph_size) < 8)
	{
		fprintf(stderr, "parse_packet: truncated ICMP header\n");
		return (1);
	}

	reply->icmp = (struct icmphdr *)(reply->buffer + iph_size);

	if (inet_ntop(AF_INET, &reply->ip->saddr, reply->src_ip, INET_ADDRSTRLEN) == NULL)
	{
		perror("inet_ntop in parse_packet");
		return (1);
	}

	return (0);
}

t_validate_reply	validate_reply(t_ping *ping, t_reply *reply)
{
	size_t icmp_len;
	uint16_t checksum;
	struct iphdr *inner_ip;
	struct icmphdr *inner_icmp;

	if (reply->icmp->type == ICMP_ECHOREPLY)
	{
		if (ntohs(reply->icmp->un.echo.id) != ping->packet.identifier)
			return (NOT_OUR_REPLY);

		if (ntohs(reply->icmp->un.echo.sequence) != ping->packet.sequence)
			return (NOT_OUR_REPLY);
		
		icmp_len = reply->bytes_received - (reply->ip->ihl * 4);
		checksum = calculate_checksum(reply->icmp, icmp_len);

		if (checksum != 0)
		{
			fprintf(stderr, "validate_reply: invalid checksum\n");
			return (INVALID_CHECKSUM);
		}

		return (VALID_REPLY);
	}

	if (reply->icmp->type == ICMP_DEST_UNREACH || reply->icmp->type == ICMP_TIME_EXCEEDED)
	{
		if (get_inner_ip_icmp(reply, &inner_ip, &inner_icmp))
			return (NOT_OUR_REPLY);

		if (inner_icmp->type != ICMP_ECHO)
			return (NOT_OUR_REPLY);

		if (ntohs(inner_icmp->un.echo.id) != ping->packet.identifier)
			return (NOT_OUR_REPLY);

		if (ntohs(inner_icmp->un.echo.sequence) != ping->packet.sequence)
			return (NOT_OUR_REPLY);

		report_icmp_error(ping, reply, inner_ip, inner_icmp);

		return (ICMP_ERROR_PACKET);
	}

	return (NOT_OUR_REPLY);
}

int	get_inner_ip_icmp(t_reply *reply, struct iphdr **inner_ip, struct icmphdr **inner_icmp)
{
	size_t outer_ip_len;
	size_t outer_icmp_len;
	size_t inner_ip_len;

	outer_ip_len = reply->ip->ihl * 4;
	outer_icmp_len = reply->bytes_received - outer_ip_len;

	if (outer_icmp_len < sizeof(struct iphdr) + sizeof(struct icmphdr))
		return (1);

	*inner_ip = (struct iphdr *)(unsigned char *)reply->icmp + sizeof(struct icmphdr);

	if ((*inner_ip)->version != 4)
		return (1);

	if ((*inner_ip)->protocol != IPPROTO_ICMP)
		return (1);

	inner_ip_len = (*inner_ip)->ihl * 4;

	if (inner_ip_len < 20 || inner_ip_len > 60)
		return (1);

	if (outer_icmp_len < sizeof(struct icmphdr) + inner_ip_len + sizeof(struct icmphdr))
		return (1);

	*inner_icmp = (struct icmphdr *)(unsigned char *)inner_ip + inner_ip_len;

	return (0);
}

void	report_icmp_error(t_ping *ping, t_reply *reply, struct iphdr *inner_ip, struct icmphdr *inner_icmp)
{
	int type;
	int code;
	int seq;
	const char *message;

	type = reply->icmp->type;
	code = reply->icmp->code;
	seq = ntohs(inner_icmp->un.echo.sequence);

	message = get_icmp_error_message(type, code);

	if (message)
	{
		printf("From (%s) icmp_seq=%d %s\n", reply->src_ip, seq, message);
		if (ping->options.verbose)
			print_inner_ip_icmp(inner_ip, inner_icmp);
	}
	else if (type == ICMP_DEST_UNREACH)
	{
		printf("From (%s) icmp_seq=%d Unknown Destination-Unreachable Code: %d\n", reply->src_ip, seq, code);
		if (ping->options.verbose)
			print_inner_ip_icmp(inner_ip, inner_icmp);
	}
	else if (type == ICMP_TIME_EXCEEDED)
	{
		printf("From (%s) icmp_seq=%d Unknown Time-Exceeded Code: %d\n", reply->src_ip, seq, code);
		if (ping->options.verbose)
			print_inner_ip_icmp(inner_ip, inner_icmp);
	}
	else
	{
		printf("From (%s) icmp_seq=%d Unknown ICMP Type Error; type(%d) code(%d)\n", reply->src_ip, seq, type, code);
		if (ping->options.verbose)
			print_inner_ip_icmp(inner_ip, inner_icmp);
	}
}

const char	*get_icmp_error_message(int type, int code)
{
	size_t i;

	for (i = 0; i < sizeof(icmp_errors) / sizeof(icmp_errors[0]); i++)
	{
		if (icmp_errors[i].type == type && icmp_errors[i].code == code)
			return icmp_errors[i].message;
	}

	return NULL;
}

void	print_inner_ip_icmp(struct iphdr *inner_ip, struct icmphdr *inner_icmp)
{
	char src_addr[INET_ADDRSTRLEN];
	char dst_addr[INET_ADDRSTRLEN];

	if (inet_ntop(AF_INET, &inner_ip->saddr, src_addr, INET_ADDRSTRLEN) == NULL)
	{
		perror("inet_ntop in print_inner_ip_icmp");
		return;
	}
	
	if (inet_ntop(AF_INET, &inner_ip->daddr, dst_addr, INET_ADDRSTRLEN) == NULL)
	{
		perror("inet_ntop in print_inner_ip_icmp");
		return;
	}

	printf("Original IP >>> source (%s), destination (%s), protocol (%u), ttl (%u)\n", src_addr, dst_addr, inner_ip->protocol, inner_ip->ttl);
	printf("Original ICMP >>> type (%u), code (%u), id (%u), sequence (%u)\n", inner_icmp->type, inner_icmp->code, ntohs(inner_icmp->un.echo.id), ntohs(inner_icmp->un.echo.sequence));
}
