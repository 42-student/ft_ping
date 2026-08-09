#include "../include/ft_ping.h"

volatile sig_atomic_t running = 1;

static void	sigint_handler(int sig)
{
	(void)sig;
	running = 0;
}

int	setup_signal_handler(void)
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction");
		return (1);
	}

	return (0);
}
