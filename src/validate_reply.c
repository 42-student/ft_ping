#include "../include/ft_ping.h"

t_validate_reply	validate_reply(t_ping *ping, t_reply *reply)
{
	size_t icmp_len;
	uint16_t checksum;

	if (reply->icmp->type != ICMP_ECHOREPLY)
	{
		fprintf(stderr, "validate_reply: invalid ICMP type\n");
		// later manage this error msg (we don't want to have this printed on screen , maybe with -v)
		return (NOT_OUR_REPLY);
	}

	if (ntohs(reply->icmp->un.echo.id) != ping->packet.identifier)
	{
		fprintf(stderr, "validate_reply: invalid ICMP id\n");
		// later manage this error msg (we don't want to have this printed on screen , maybe with -v)
		return (NOT_OUR_REPLY);
	}

	if (ntohs(reply->icmp->un.echo.sequence) != ping->packet.sequence)
	{
		fprintf(stderr, "validate_reply: invalid ICMP sequence\n");
		// later manage this error msg (we don't want to have this printed on screen , maybe with -v)
		return (NOT_OUR_REPLY);
	}

	icmp_len = reply->bytes_received - (reply->ip->ihl * 4);

	checksum = calculate_checksum(reply->icmp, icmp_len);

	if (checksum != 0)
	{
		fprintf(stderr, "validate_reply: invalid checksum\n");
		// later manage this error msg (we don't want to have this printed on screen , maybe with -v)
		return (INVALID_CHECKSUM);
	}

	return (0);
}
