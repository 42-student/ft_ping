#include "../include/ft_ping.h"

int	receive_packet(t_ping *ping, t_reply *reply)
{
	socklen_t sender_len;

	sender_len = sizeof(reply->sender);

	reply->bytes_received = recvfrom(ping->socket.fd, reply->buffer, 1500, 0, (struct sockaddr *)&reply->sender, &sender_len);

	if (reply->bytes_received < 0)
	{
		perror("recvfrom");
		return (1);
	}

	gettimeofday(&reply->timestamp, NULL);

	return (0);
}
