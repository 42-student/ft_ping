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

	reply->src_hostname = gethostbyaddr(&reply->sender.sin_addr, sizeof(reply->sender.sin_addr), AF_INET);

	if (reply->src_hostname == NULL)
	{
		if (ping->options.verbose)
			fprintf(stderr, "parse_packet: gethostbyaddr failed with: %s\n", hstrerror(h_errno));
		return (1);
	}

	return (0);
}

t_validate_reply	validate_reply(t_ping *ping, t_reply *reply)
{
	size_t icmp_len;
	uint16_t checksum;

	if (ntohs(reply->icmp->un.echo.id) != ping->packet.identifier)
	{
		if (ping->options.verbose)
			fprintf(stderr, "validate_reply: invalid ICMP id\n");
		return (NOT_OUR_REPLY);
	}

	if (ntohs(reply->icmp->un.echo.sequence) != ping->packet.sequence)
	{
		if (ping->options.verbose)
			fprintf(stderr, "validate_reply: invalid ICMP sequence\n");
		return (NOT_OUR_REPLY);
	}

	icmp_len = reply->bytes_received - (reply->ip->ihl * 4);

	checksum = calculate_checksum(reply->icmp, icmp_len);

	if (checksum != 0)
	{
		if (ping->options.verbose)
			fprintf(stderr, "validate_reply: invalid checksum\n");
		return (INVALID_CHECKSUM);
	}

	if (reply->icmp->type != ICMP_ECHOREPLY)
	{
		if (ping->options.verbose)
			fprintf(stderr, "validate_reply: ICMP error packet\n");
		return (ICMP_ERROR_PACKET);
	}

	return (VALID_REPLY);
}

void	report_icmp_result(t_reply *reply)
{
	if (reply->icmp->code == ICMP_DEST_UNREACH)
		fprintf(stderr, "From %s icmp_seq=%d Destination Host Unreachable", reply->src_ip, reply->icmp->un.echo.sequence);
	else if (reply->icmp->code == ICMP_TIME_EXCEEDED)
		fprintf(stderr, "From %s icmp_seq=%d Time to live exceeded", reply->src_ip, reply->icmp->un.echo.sequence);
	else if (reply->icmp->code == ICMP_PARAMETERPROB)
		fprintf(stderr, "From %s icmp_seq=%d Parameter problem", reply->src_ip, reply->icmp->un.echo.sequence);
	else
		fprintf(stderr, "From %s icmp_seq=%d Unknown ICMP code", reply->src_ip, reply->icmp->un.echo.sequence);
}
