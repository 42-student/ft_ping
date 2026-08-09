#include "../include/ft_ping.h"

t_receive_result	receive_packet(t_ping *ping, t_reply *reply)
{
	fd_set readfds;
	struct timeval timeout;
	socklen_t sender_len;

	FD_ZERO(&readfds);
	FD_SET(ping->socket.fd, &readfds);

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	int res = select(ping->socket.fd + 1, &readfds, NULL, NULL, &timeout);
	
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

	reply->bytes_received = recvfrom(ping->socket.fd, reply->buffer, 1500, 0, (struct sockaddr *)&reply->sender, &sender_len);

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
