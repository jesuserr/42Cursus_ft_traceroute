/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   messages.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jesuserr <jesuserr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 18:19:03 by jesuserr          #+#    #+#             */
/*   Updated: 2024/11/13 22:06:24 by jesuserr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

// Although more complex than inet_ntoa(), the function inet_ntop() is better
// choice since it is thread-safe.
const char	*turn_ip_to_str(t_ping_data *ping_data, void *src, char *dst)
{
	if (inet_ntop(AF_INET, src, dst, INET_ADDRSTRLEN) == NULL)
		print_perror_and_exit("inet_ntop", ping_data);
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
// addressed to us, it is discarded.
// 1   172.29.160.1  0.125ms  0.097ms  0.059ms 
// 2   192.168.1.1  0.416ms  0.330ms  0.338ms 
// ping_data->ttl_packets_received++; (needed?)
void	print_response_line(t_ping_data *ping_data, char *buff, \
		struct iphdr *ip_header)
{
	t_icmp_packet	*inner_icmp_packet;
	char			src_addr_str[INET_ADDRSTRLEN];
	struct timeval	tv;
	float			time_ms;

	//struct iphdr	*inner_ip_header;	
	//inner_ip_header = (struct iphdr *)(buff + (ip_header->ihl * 4) + sizeof(struct icmphdr));
	inner_icmp_packet = (t_icmp_packet *)(buff + (ip_header->ihl * 4) + \
	sizeof(struct icmphdr) + sizeof(struct iphdr));
	if (inner_icmp_packet->icmp_header.un.echo.id != \
	ping_data->packet.icmp_header.un.echo.id)
		return ;
	if (gettimeofday(&tv, NULL) == -1)
		print_perror_and_exit("gettimeofday receive packet", ping_data);
	tv.tv_sec = tv.tv_sec - inner_icmp_packet->seconds;
	tv.tv_usec = tv.tv_usec - inner_icmp_packet->microseconds;
	time_ms = tv.tv_sec * 1000 + (float)tv.tv_usec / 1000;
	if (ping_data->packet.icmp_header.un.echo.sequence == 1)
		printf("%s", turn_ip_to_str(ping_data, &(ip_header->saddr), \
		src_addr_str));
	printf("  %.3fms", time_ms);
}
