#include "../include/ft_ping.h"

t_receive_result	receive_packet(t_ping *ping, t_reply *reply)
{
	socklen_t sender_len;

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
