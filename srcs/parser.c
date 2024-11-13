/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jesuserr <jesuserr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 18:19:12 by jesuserr          #+#    #+#             */
/*   Updated: 2024/11/13 13:57:59 by jesuserr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

// Mandatory: -h or -? and -V
// Bonus: -f <first-hop>, -m <max-hops>, -q <packets>, -r and -w <timeout> (5)
void	print_usage(void)
{
	printf("Usage\n"
		"  ./ft_traceroute [options] <destination>\n\n"
		"Options:\n"
		"  <destination>      dns name or ip address\n"
		"  -f <first-hop>     set initial hop distance (default: 1)\n"
		"  -h or -?           print help and exit\n"
		"  -m <max-hops>      set maximal hop count (default: 64)\n"
		"  -q <packets>       send probe packets per hop (default: 3)\n"
		"  -r                 resolve hostnames\n"
		"  -V                 print version and exit\n"
		"  -w <timeout>       time to wait for response (default: 3)\n");
	exit(EXIT_SUCCESS);
}

bool	check_if_only_digits(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (false);
		i++;
	}
	return (true);
}

u_int8_t	check_argument_value(int opt, char *opt_arg)
{
	int32_t	value;

	value = ft_atoi(opt_arg);
	if ((ft_strlen(opt_arg) > 3 || !check_if_only_digits(opt_arg) || value == 0 \
	|| value > 255) && opt == 'f')
		print_error_and_exit("impossible initial distance (0 < value < 255)");
	else if ((ft_strlen(opt_arg) > 3 || !check_if_only_digits(opt_arg) || \
	value == 0 || value > 255) && opt == 'm')
		print_error_and_exit("invalid maximal hops value (0 < value < 255)");
	else if ((ft_strlen(opt_arg) > 3 || !check_if_only_digits(opt_arg) || \
	value == 0 || value > 10) && opt == 'q')
		print_error_and_exit("probe packets per hop must be between 1 and 10");
	else if ((ft_strlen(opt_arg) > 3 || !check_if_only_digits(opt_arg) || \
	value == 0 || value > 59) && opt == 'w')
		print_error_and_exit("ridiculous waiting time (0 < value < 60)");
	return ((u_int8_t)value);
}

void	parse_options(int opt, t_arguments *args)
{
	if (opt == 'f')
		args->first_hop = check_argument_value(opt, optarg);
	else if (opt == 'h' || opt == '?')
		print_usage();
	else if (opt == 'm')
		args->max_hops = check_argument_value(opt, optarg);
	else if (opt == 'q')
		args->packets_per_hop = check_argument_value(opt, optarg);
	else if (opt == 'r')
		args->resolve_hostnames = true;
	else if (opt == 'V')
	{
		printf("ft_traceroute 1.0 based on traceroute implementation ");
		printf("from inetutils 2.0\n");
		exit(EXIT_SUCCESS);
	}
	else if (opt == 'w')
		args->timeout = check_argument_value(opt, optarg);
}

void	parse_arguments(int argc, char **argv, t_arguments *args)
{
	int		opt;

	args->first_hop = DEFAULT_FIRST_HOP;
	args->max_hops = DEFAULT_MAX_HOPS;
	args->packets_per_hop = DEFAULT_PACKETS;
	args->timeout = DEFAULT_TIMEOUT;
	opt = getopt(argc, argv, "f:h?m:q:rVw:");
	while (opt != -1)
	{
		parse_options(opt, args);
		opt = getopt(argc, argv, "f:h?m:q:rVw:");
	}
	if (optind >= argc)
		print_error_and_exit("Destination address required");
	if (getuid() != 0)
		print_error_and_exit("Superuser privileges needed to run the program.");
	args->dest = argv[optind];
	return ;
}
