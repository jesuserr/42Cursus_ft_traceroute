/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jesuserr <jesuserr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 18:17:00 by jesuserr          #+#    #+#             */
/*   Updated: 2024/11/14 10:51:19 by jesuserr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

void	set_socket_ttl(t_ping_data *ping_data, u_int8_t ttl)
{
	int				ret;

	ret = setsockopt(ping_data->sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
	if (ret == -1)
		print_strerror_and_exit("setsockopt ttl", ping_data);
}

void	set_socket_timeout(t_ping_data *ping_data)
{
	int				ret;
	struct timeval	timeout;

	timeout.tv_sec = ping_data->args.timeout;
	timeout.tv_usec = 0;
	ret = setsockopt(ping_data->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, \
	sizeof(timeout));
	if (ret == -1)
		print_strerror_and_exit("setsockopt timeout", ping_data);
}

// Uses getaddrinfo to obtain the destination address and then is stored in
// ping_data->dest_addr.sin_addr. The ICMP packet is initialized with the
// necessary common values and socket is created.
void	init_ping_data_and_socket(t_ping_data *ping_data)
{
	struct addrinfo	*result;
	int				ret;

	ping_data->hints.ai_family = SOCKET_DOMAIN;
	ping_data->hints.ai_socktype = SOCKET_TYPE;
	ping_data->hints.ai_protocol = SOCKET_PROTOCOL;
	ret = getaddrinfo(ping_data->args.dest, NULL, &ping_data->hints, &result);
	if (ret != 0)
	{
		fprintf(stderr, "ft_ping: %s: %s\n", ping_data->args.dest, \
		gai_strerror(ret));
		exit(EXIT_FAILURE);
	}
	ping_data->dest_addr.sin_family = SOCKET_DOMAIN;
	ping_data->dest_addr.sin_addr = ((struct sockaddr_in *)result->ai_addr) \
	->sin_addr;
	freeaddrinfo(result);
	ping_data->packet.icmp_header.type = ICMP_ECHO;
	ping_data->packet.icmp_header.code = 0;
	ping_data->packet.icmp_header.un.echo.id = getpid() & 0xFFFF;
	ping_data->packet.icmp_header.un.echo.sequence = 0;
	ft_memcpy(ping_data->packet.payload, PAYLOAD_56_B, ft_strlen(PAYLOAD_56_B));
	ping_data->sockfd = socket(SOCKET_DOMAIN, SOCKET_TYPE, SOCKET_PROTOCOL);
	if (ping_data->sockfd == -1)
		print_strerror_and_exit("socket", ping_data);
}

int	main(int argc, char **argv)
{
	t_ping_data	ping_data;

	if (argc < 2)
		print_error_and_exit("Destination address required");
	ft_bzero(&ping_data, sizeof(t_ping_data));
	parse_arguments(argc, argv, &ping_data.args);
	init_ping_data_and_socket(&ping_data);
	set_socket_timeout(&ping_data);
	traceroute(&ping_data);
	return (EXIT_SUCCESS);
}
