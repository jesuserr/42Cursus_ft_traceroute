/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   messages.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jesuserr <jesuserr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 18:19:03 by jesuserr          #+#    #+#             */
/*   Updated: 2024/11/14 10:51:19 by jesuserr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

// Although more complex than inet_ntoa(), the function inet_ntop() is better
// choice since it is thread-safe.
const char	*turn_ip_to_str(t_ping_data *ping_data, void *src, char *dst)
{
	if (inet_ntop(AF_INET, src, dst, INET_ADDRSTRLEN) == NULL)
		print_strerror_and_exit("inet_ntop", ping_data);
	return (dst);
}

// traceroute to www.google.com (216.58.215.164), 64 hops max
void	print_header(t_ping_data *ping_data)
{
	turn_ip_to_str(ping_data, &(ping_data->dest_addr.sin_addr), \
	ping_data->ip_str);
	printf("traceroute to %s (%s), ", ping_data->args.dest, ping_data->ip_str);
	printf("%d hops max\n", ping_data->args.max_hops);
}

// buff contains the time exceeded received packet, which consists of the source
// IP address (20 bytes) + ICMP header (8 bytes) + original IP header (20 bytes)
// + original echo request packet (64 bytes). Values between () are not taken by
// granted, provided just for reference. Access to original ICMP packet is
// needed to access original timestamp and calculate RTT. If the packet is not
// addressed to us, return false to keep receiving packets.
// 1   172.29.160.1  0.125ms  0.097ms  0.059ms 
// 2   192.168.1.1  0.416ms  0.330ms  0.338ms
bool	print_response_ttl_exceeded(t_ping_data *ping_data, char *buff, \
		struct iphdr *ip_header)
{
	t_icmp_packet	*inner_icmp_packet;
	char			src_addr_str[INET_ADDRSTRLEN];
	struct timeval	tv;
	float			time_ms;

	inner_icmp_packet = (t_icmp_packet *)(buff + (ip_header->ihl * 4) + \
	sizeof(struct icmphdr) + sizeof(struct iphdr));
	if (inner_icmp_packet->icmp_header.un.echo.id != \
	ping_data->packet.icmp_header.un.echo.id)
		return (false);
	if (gettimeofday(&tv, NULL) == -1)
		print_strerror_and_exit("gettimeofday receive packet", ping_data);
	tv.tv_sec = tv.tv_sec - ping_data->seconds;
	tv.tv_usec = tv.tv_usec - ping_data->microseconds;
	time_ms = tv.tv_sec * 1000 + (float)tv.tv_usec / 1000;
	if (!ping_data->printed_ip)
	{
		printf("%s", turn_ip_to_str(ping_data, &(ip_header->saddr), \
		src_addr_str));
		ping_data->printed_ip = true;
	}
	printf("  %.3fms", time_ms);
	return (true);
}

// If the packet is not addressed to us, return false to keep receiving packets.
// If the packet is addressed to us, it means that traceroute reached its
// destination so the flag 'destiny_reached' is set to true to break the main
// loop and end the program.
bool	print_response_echo_reply(t_ping_data *ping_data, u_int16_t id, \
		struct iphdr *ip_header)
{
	char			src_addr_str[INET_ADDRSTRLEN];
	struct timeval	tv;
	float			time_ms;

	if (id != ping_data->packet.icmp_header.un.echo.id)
		return (false);
	ping_data->destiny_reached = true;
	if (gettimeofday(&tv, NULL) == -1)
		print_strerror_and_exit("gettimeofday receive packet", ping_data);
	tv.tv_sec = tv.tv_sec - ping_data->seconds;
	tv.tv_usec = tv.tv_usec - ping_data->microseconds;
	time_ms = tv.tv_sec * 1000 + (float)tv.tv_usec / 1000;
	if (!ping_data->printed_ip)
	{
		printf("%s", turn_ip_to_str(ping_data, &(ip_header->saddr), \
		src_addr_str));
		ping_data->printed_ip = true;
	}
	printf("  %.3fms", time_ms);
	return (true);
}
