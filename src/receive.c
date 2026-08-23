#include "../include/ft_ping.h"

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

int	parse_packet(t_ping *ping, t_reply *reply)
{
	uint16_t ip_tot_len;
	ssize_t iph_size;

	if (reply->bytes_received < 20)
	{
		if (ping->options.verbose)
			fprintf(stderr, "parse_packet: truncated IPv4 packet (expected at least 20 bytes)\n");
		return (1);
	}

	reply->ip = (struct iphdr *)reply->buffer;
	ip_tot_len = ntohs(reply->ip->tot_len);

	if ((size_t)reply->bytes_received < ip_tot_len)
	{
		if (ping->options.verbose)
			fprintf(stderr, "parse_packet: received %ld bytes, expected %d\n", reply->bytes_received, ip_tot_len);
		return (1);
	}

	if (reply->ip->version != 4)
	{
		if (ping->options.verbose)
			fprintf(stderr, "parse_packet: unsupported IP version (expected IPv4)\n");
		return (1);
	}

	if (reply->ip->protocol != IPPROTO_ICMP)
	{
		if (ping->options.verbose)
			fprintf(stderr, "parse_packet: IP protocol must be ICMP");
		return (1);
	}

	iph_size = reply->ip->ihl * 4;

	if (iph_size < 20 || iph_size > 60)
	{
		if (ping->options.verbose)
			fprintf(stderr, "parse_packet: invalid IPv4 header length\n");
		return (1);
	}

	if (reply->bytes_received < iph_size)
	{
		if (ping->options.verbose)
			fprintf(stderr, "parse_packet: truncated IPv4 header\n");
		return (1);
	}

	if ((reply->bytes_received - iph_size) < 8)
	{
		if (ping->options.verbose)
			fprintf(stderr, "parse_packet: truncated ICMP header\n");
		return (1);
	}

	reply->icmp = (struct icmphdr *)(reply->buffer + iph_size);

	if (inet_ntop(AF_INET, &reply->ip->saddr, reply->src_ip, INET_ADDRSTRLEN) == NULL)
	{
		if (ping->options.verbose)
			perror("inet_ntop in parse_packet");
		return (1);
	}

	return (0);
}

t_validate_reply	validate_reply(t_ping *ping, t_reply *reply)
{
	size_t icmp_len;
	uint16_t checksum;
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
			if (ping->options.verbose)
				fprintf(stderr, "validate_reply: invalid checksum\n");
			return (INVALID_CHECKSUM);
		}

		return (VALID_REPLY);
	}

	if (reply->icmp->type == ICMP_DEST_UNREACH || reply->icmp->type == ICMP_TIME_EXCEEDED)
	{
		if (get_inner_icmp(reply, &inner_icmp))
			return (NOT_OUR_REPLY);

		if (inner_icmp->type != ICMP_ECHO)
			return (NOT_OUR_REPLY);

		if (ntohs(inner_icmp->un.echo.id) != ping->packet.identifier)
			return (NOT_OUR_REPLY);

		if (ntohs(inner_icmp->un.echo.sequence) != ping->packet.sequence)
			return (NOT_OUR_REPLY);

		report_icmp_error(reply, inner_icmp);

		return (ICMP_ERROR_PACKET);
	}

	return (NOT_OUR_REPLY);
}

int	get_inner_icmp(t_reply *reply, struct icmphdr **inner_icmp)
{
	struct iphdr *inner_ip;
	size_t outer_ip_len;
	size_t outer_icmp_len;
	size_t inner_ip_len;

	outer_ip_len = reply->ip->ihl * 4;
	outer_icmp_len = reply->bytes_received - outer_ip_len;

	if (outer_icmp_len < sizeof(struct iphdr) + sizeof(struct icmphdr))
		return (1);

	inner_ip = (struct iphdr *)(unsigned char *)reply->icmp + sizeof(struct icmphdr);

	if (inner_ip->version != 4)
		return (1);

	if (inner_ip->protocol != IPPROTO_ICMP)
		return (1);

	inner_ip_len = inner_ip->ihl * 4;

	if (inner_ip_len < 20 || inner_ip_len > 60)
		return (1);

	if (outer_icmp_len < sizeof(struct icmphdr) + inner_ip_len + sizeof(struct icmphdr))
		return (1);

	*inner_icmp = (struct icmphdr *)(unsigned char *)inner_ip + inner_ip_len;

	return (0);
}

void	report_icmp_error(t_reply *reply, struct icmphdr *inner_icmp)
{
	if (reply->icmp->type == ICMP_DEST_UNREACH)
	{
		switch (reply->icmp->code)
		{
			case ICMP_NET_UNREACH:
				printf("From (%s) icmp_seq=%d Destination Network Unreachable\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_HOST_UNREACH:
				printf("From (%s) icmp_seq=%d Destination Host Unreachable\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_PROT_UNREACH:
				printf("From (%s) icmp_seq=%d Destination Protocol Unreachable\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_PORT_UNREACH:
				printf("From (%s) icmp_seq=%d Destination Port Unreachable\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_FRAG_NEEDED:
				printf("From (%s) icmp_seq=%d Destination Fragmentation Needed And DF Set\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_SR_FAILED:
				printf("From (%s) icmp_seq=%d Destination Source Route Failed\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_NET_UNKNOWN:
				printf("From (%s) icmp_seq=%d Destination Network Unknown\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_HOST_UNKNOWN:
				printf("From (%s) icmp_seq=%d Destination Host Unknown\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_HOST_ISOLATED:
				printf("From (%s) icmp_seq=%d Destination Source Host Isolated\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_NET_ANO:
				printf("From (%s) icmp_seq=%d Destination Network Administratively Prohibited\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_HOST_ANO:
				printf("From (%s) icmp_seq=%d Destination Host Administratively Prohibited\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_NET_UNR_TOS:
				printf("From (%s) icmp_seq=%d Destination Network Unreachable For TOS\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_HOST_UNR_TOS:
				printf("From (%s) icmp_seq=%d Destination Host Unreachable For TOS\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_PKT_FILTERED:
				printf("From (%s) icmp_seq=%d Destination Communication Administratively Prohibited\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_PREC_VIOLATION:
				printf("From (%s) icmp_seq=%d Destination Host Precedence Violation\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_PREC_CUTOFF:
				printf("From (%s) icmp_seq=%d Destination Precedence Cutoff In Effect\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			default:
				printf("From (%s) icmp_seq=%d Unknown Destination-Unreachable Code: %d\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence), reply->icmp->code);
				break;
		}
	}
	else if (reply->icmp->type == ICMP_TIME_EXCEEDED)
	{
		switch (reply->icmp->code)
		{
			case ICMP_EXC_TTL:
				printf("From (%s) icmp_seq=%d Time To Live Exceeded In Transit\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			case ICMP_EXC_FRAGTIME:
				printf("From (%s) icmp_seq=%d Fragment Reassembly Time Exceeded\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence));
				break;

			default:
				printf("From (%s) icmp_seq=%d Unknown Time-Exceeded Code: %d\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence), reply->icmp->code);
				break;
		}
	}
	else
	{
		printf("From (%s) icmp_seq=%d Unknown ICMP Type Error; type(%d) code(%d)\n", reply->src_ip, ntohs(inner_icmp->un.echo.sequence), reply->icmp->type, reply->icmp->code);
	}
}
