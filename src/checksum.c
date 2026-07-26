#include "../include/ft_ping.h"

uint16_t	calculate_checksum(void *data, size_t len)
{
	uint32_t sum;
	uint16_t *ptr;

	sum = 0;
	ptr = (uint16_t *)data;

	while (len > 1)
	{
		sum += *ptr++;
		len -= 2;
	}

	if (len == 1)
		sum += *(uint8_t *)ptr;
	
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ((uint16_t)~sum);
}
