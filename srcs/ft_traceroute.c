/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jesuserr <jesuserr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 18:18:46 by jesuserr          #+#    #+#             */
/*   Updated: 2024/11/14 09:40:27 by jesuserr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

// Calculates the checksum of the whole ICMP packet treating it as a sequence of
// 16-bit words. After the sum it is verified if there is a carry, and if so, it
// is added to the sum (for this reason sum is a 32-bit integer and not 16-bit).
// The final checksum is the 16-bit one's complement of the sum.
// The size of the packet is assumed to be an even number of bytes.
uint16_t	calc_checksum(t_icmp_packet *ptr)
{
	uint32_t	sum;
	uint16_t	*address;
	uint8_t		len;

	sum = 0;
	address = (uint16_t *)ptr;
	len = sizeof(t_icmp_packet);
	while (len > 1)
	{
		sum += *address++;
		len -= sizeof(uint16_t);
	}
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);
	return ((uint16_t)(~sum));
}

// Fills the ICMP packet with the necessary data and sends it. The payload
// includes the time the packet is sent, so the RTT can be calculated when the
// packet is received.
void	fill_and_send_icmp_packet(t_ping_data *ping_data)
{
	struct timeval	tv;
	ssize_t			bytes_sent;

	ping_data->packet.icmp_header.checksum = 0;
	if (gettimeofday(&tv, NULL) == -1)
		print_perror_and_exit("gettimeofday send packet", ping_data);
	ping_data->packet.seconds = tv.tv_sec;
	ping_data->packet.microseconds = tv.tv_usec;
	ping_data->packet.icmp_header.checksum = calc_checksum(&ping_data->packet);
	bytes_sent = sendto(ping_data->sockfd, &ping_data->packet, \
	sizeof(ping_data->packet), 0, (struct sockaddr *)&ping_data->dest_addr, \
	sizeof(ping_data->dest_addr));
	if (bytes_sent == -1)
		print_perror_and_exit("sendto", ping_data);
	ping_data->packet.icmp_header.un.echo.sequence++;
}

// Since recvfrom() is blocking, conditions EWOULDBLOCK / EAGAIN are checked to
// see if recvfrom() failed because no data was available to read within the
// specified timeout period and therefore print "*" and return.
// If correct data is received, it's a TIME_EXCEEDED or ECHO_REPLY packet and
// is addressed to our process, the response line is printed and the loop is 
// broken. Otherwise, the loop continues.
// When the packet is received, it contains the IP header of the sender (usually
// 20 bytes, but calculated anyways), which is casted to a struct iphdr in order
// to get access to the received ICMP packet to verify its type (TIME_EXCEEDED
// or ECHO_REPLY).
void	receive_packet(t_ping_data *ping_data)
{
	struct iphdr	*ip_header;
	char			buff[BUFFER_LEN];
	t_icmp_packet	packet;

	while (1)
	{
		if (recvfrom(ping_data->sockfd, buff, BUFFER_LEN, 0, NULL, NULL) == -1)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				ft_putstr_fd("*  ", 1);
				return ;
			}
			print_perror_and_exit("recvfrom", ping_data);
		}
		ip_header = (struct iphdr *)buff;
		ft_memcpy(&packet, buff + (ip_header->ihl * 4), sizeof(t_icmp_packet));
		if ((packet.icmp_header.type == ICMP_TIME_EXCEEDED && \
		print_response_ttl_exceeded(ping_data, buff, ip_header)) || \
		(packet.icmp_header.type == ICMP_ECHOREPLY && \
		print_response_echo_reply(ping_data, packet, ip_header)))
			break ;
	}
}

// Send packets with increasing TTL until the destination is reached or the
// maximum number of hops is reached.
void	traceroute(t_ping_data *ping_data)
{
	int	i;
	int	j;

	print_header(ping_data);
	i = 1;
	while (i <= ping_data->args.max_hops && !ping_data->destiny_reached)
	{
		printf("%3d   ", i++);
		fflush(stdout);
		j = 1;
		set_socket_ttl(ping_data, ping_data->args.first_hop++);
		while (j++ <= ping_data->args.packets_per_hop)
		{
			fill_and_send_icmp_packet(ping_data);
			receive_packet(ping_data);
		}
		ping_data->packet.icmp_header.un.echo.sequence = 0;
		ping_data->printed_ip = false;
		printf("\n");
	}
	close(ping_data->sockfd);
	return ;
}
