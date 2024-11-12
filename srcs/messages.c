/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   messages.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jesuserr <jesuserr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 18:19:03 by jesuserr          #+#    #+#             */
/*   Updated: 2024/11/12 18:19:04 by jesuserr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

// PING ivoice.synology.me (79.154.85.235): 56 data bytes
// PING ivoice.synology.me (79.154.85.235): 56 data bytes, id 0x0f90 = 3984
void	print_header(t_ping_data *ping_data)
{
	turn_ip_to_str(ping_data, &(ping_data->dest_addr.sin_addr), \
	ping_data->ip_str);
	printf("PING %s (%s): ", ping_data->args.dest, ping_data->ip_str);
	printf("%ld data bytes", sizeof(t_icmp_packet) - sizeof(struct icmphdr));
	if (ping_data->args.verbose_mode)
	{
		printf(", id 0x%04x =", ping_data->packet.icmp_header.un.echo.id);
		printf(" %d", ping_data->packet.icmp_header.un.echo.id);
	}
	printf("\n");
}

// 64 bytes from 79.154.85.235: icmp_seq=0 ttl=165 time=0.914 ms
// Uses Welford's algorithm to calculate the population standard deviation on
// the fly (just in one single pass).
void	print_response_line(t_ping_data *ping_data, t_icmp_packet packet, \
		uint8_t ttl)
{
	struct timeval	tv;	
	float			time_ms;
	float			delta_time;

	ping_data->packets_received++;
	if (gettimeofday(&tv, NULL) == -1)
		print_perror_and_exit("gettimeofday receive packet", ping_data);
	if (ping_data->args.print_timestamps && !ping_data->args.quiet_mode)
		printf("[%ld.%ld] ", tv.tv_sec, tv.tv_usec);
	tv.tv_usec = tv.tv_usec - packet.microseconds;
	time_ms = (tv.tv_sec - packet.seconds) * 1000 + (float)tv.tv_usec / 1000;
	if (time_ms > ping_data->timings.max_time)
		ping_data->timings.max_time = time_ms;
	if (time_ms < ping_data->timings.min_time)
		ping_data->timings.min_time = time_ms;
	ping_data->timings.sum_times += time_ms;
	delta_time = time_ms - ping_data->timings.mean_time;
	ping_data->timings.mean_time += delta_time / ping_data->packets_received;
	ping_data->timings.square_dist += delta_time * \
	(time_ms - ping_data->timings.mean_time);
	if (ping_data->args.quiet_mode)
		return ;
	printf("%ld bytes from %s: ", sizeof(t_icmp_packet), ping_data->ip_str);
	printf("icmp_seq=%d ", packet.icmp_header.un.echo.sequence);
	printf("ttl=%d time=%.3f ms \n", ttl, time_ms);
}

//IP Hdr Dump:
// 4500 0054 0b66 4000 0101 9bde c0a8 01ad 0808 0808 
//Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src	Dst	Data
// 4  5  00 0054 0b66   2 0000  01  01 9bde 192.168.1.173  8.8.8.8 
//ICMP: type 8, code 0, size 64, id 0x13ea, seq 0x0000
void	print_verbose_ttl(struct iphdr *ip_hdr, struct icmphdr *icmp_hdr, \
		t_ping_data *ping_data)
{
	char	ip_str[INET_ADDRSTRLEN];
	uint8_t	*byte_ptr;

	byte_ptr = (uint8_t *)ip_hdr;
	printf("IP Hdr Dump: \n ");
	while (byte_ptr < (uint8_t *)icmp_hdr)
	{
		printf("%02x%02x ", *byte_ptr, *(byte_ptr + 1));
		byte_ptr = byte_ptr + 2;
	}
	printf("\nVr HL TOS  Len   ID Flg  off TTL Pro  cks      Src	Dst	Data");
	printf("\n %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x ", \
	ip_hdr->version, ip_hdr->ihl, ip_hdr->tos, ntohs(ip_hdr->tot_len), \
	ntohs(ip_hdr->id), (ntohs (ip_hdr->frag_off) & 0xe000) >> 13, \
	ntohs (ip_hdr->frag_off) & 0x1fff, ip_hdr->ttl, ip_hdr->protocol, \
	ntohs (ip_hdr->check));
	printf(" %s", turn_ip_to_str(ping_data, &(ip_hdr->saddr), ip_str));
	printf(" %s", turn_ip_to_str(ping_data, &(ip_hdr->daddr), ip_str));
	printf("\nICMP: type %d, code %d, size %ld, id 0x%04x, seq 0x%04x\n", \
	icmp_hdr->type, icmp_hdr->code, sizeof(t_icmp_packet), \
	icmp_hdr->un.echo.id, icmp_hdr->un.echo.sequence);
}

// buff contains the time exceeded received packet, which consists of the source
// IP address (20 bytes) + ICMP header (8 bytes) + original IP header (20 bytes)
// + original echo request packet (64 bytes). Values between () are not taken by
// granted, provided just for reference. Access to originals ICMP and IP packets
// is needed for print_verbose_ttl info. If the packet is not addressed to us,
// it is discarded.
// XX bytes from xxx.xxx.xxx.xxx (xxx.xxx.xxx.xxx): Time to live exceeded
void	print_ttl_exceeded_line(t_ping_data *ping_data, char *buff, \
		struct iphdr *ip_header)
{
	struct icmphdr	*inner_icmp_header;
	struct iphdr	*inner_ip_header;
	char			src_addr_str[INET_ADDRSTRLEN];
	struct timeval	tv;

	inner_icmp_header = (struct icmphdr *)(buff + (ip_header->ihl * 4) + \
	sizeof(struct icmphdr) + sizeof(struct iphdr));
	inner_ip_header = (struct iphdr *)(buff + (ip_header->ihl * 4) + \
	sizeof(struct icmphdr));
	if (inner_icmp_header->un.echo.id != \
	ping_data->packet.icmp_header.un.echo.id)
		return ;
	ping_data->ttl_packets_received++;
	if (ping_data->args.print_timestamps)
	{
		if (gettimeofday(&tv, NULL) == -1)
			print_perror_and_exit("gettimeofday exceeded ttl", ping_data);
		printf("[%ld.%ld] ", tv.tv_sec, tv.tv_usec);
	}
	printf("%d bytes from ", ntohs(ip_header->tot_len) - (ip_header->ihl * 4));
	printf("%s", turn_ip_to_str(ping_data, &(ip_header->saddr), src_addr_str));
	printf(" (%s): Time to live exceeded\n", src_addr_str);
	if (ping_data->args.verbose_mode)
		print_verbose_ttl(inner_ip_header, inner_icmp_header, ping_data);
}

// --- ivoice.synology.me ping statistics ---
// 5 packets transmitted, 5 packets received, 0% packet loss
// round-trip min/avg/max/stddev = 0.457/0.886/1.284/0.294 ms
void	print_summary(t_ping_data *ping_data)
{
	float	loss;

	loss = 100 - (((float)ping_data->packets_received / \
	(float)ping_data->packet.icmp_header.un.echo.sequence) * 100);
	printf("--- %s ping statistics ---\n", ping_data->args.dest);
	printf("%d ", ping_data->packet.icmp_header.un.echo.sequence);
	printf("packets transmitted, %d ", ping_data->packets_received);
	printf("packets received, %.0f%% packet loss\n", loss);
	if (ping_data->packets_received == 0)
		return ;
	printf("round-trip min/avg/max/stddev = %.3f", ping_data->timings.min_time);
	printf("/%.3f", ping_data->timings.sum_times / ping_data->packets_received);
	printf("/%.3f/%.3f ms\n", ping_data->timings.max_time, \
	sqrt(ping_data->timings.square_dist / ping_data->packets_received));
}
