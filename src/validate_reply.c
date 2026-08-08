#include "../include/ft_ping.h"

int	validate_reply(t_ping *ping, t_reply *reply)
{
	if (reply->icmp->type != ICMP_ECHOREPLY)
	{
		return (1);
	}

	if (ntohs(reply->icmp->un.echo.id) != ping->packet.identifier)
	{
		return (1);
	}

	if (ntohs(reply->icmp->un.echo.sequence) != ping->packet.sequence)
	{
		return (1);
	}

	return (0);
}
