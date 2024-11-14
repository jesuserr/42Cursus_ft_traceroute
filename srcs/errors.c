/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   errors.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jesuserr <jesuserr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 18:19:20 by jesuserr          #+#    #+#             */
/*   Updated: 2024/11/14 10:51:19 by jesuserr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

void	print_error_and_exit(char *str)
{
	printf("ft_traceroute: usage error: %s\n", str);
	printf("Try 'ft_traceroute -h' or 'ft_traceroute -?' for more ");
	printf("information.\n");
	exit (EXIT_FAILURE);
}

// Prints system error message, closes the socket (if ping_data has been passed
// containing an open socket) and then exits with EXIT_FAILURE status.
void	print_strerror_and_exit(char *msg, t_ping_data *ping_data)
{
	printf("%s: %s\n", msg, strerror(errno));
	if (ping_data && ping_data->sockfd > 0)
		close(ping_data->sockfd);
	exit(EXIT_FAILURE);
}
