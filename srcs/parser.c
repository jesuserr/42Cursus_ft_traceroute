/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jesuserr <jesuserr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 18:19:12 by jesuserr          #+#    #+#             */
/*   Updated: 2024/11/12 18:19:13 by jesuserr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

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

u_int8_t	check_argument_value_ttl(char *opt_arg)
{
	int32_t	value;

	value = ft_atoi(opt_arg);
	if (ft_strlen(opt_arg) > 3 || !check_if_only_digits(opt_arg) || value == 0 \
	|| value > 255)
		print_error_and_exit("Out of range: 0 < integer value < 255");
	return ((u_int8_t)value);
}

int32_t	check_argument_value(char *opt_arg)
{
	int32_t	value;

	value = ft_atoi(opt_arg);
	if (ft_strlen(opt_arg) > 9 || !check_if_only_digits(opt_arg) || value == 0)
		print_error_and_exit("Out of range: 0 < integer value < 999999999");
	return (value);
}

void	parse_options(int opt, t_arguments *args)
{
	if (opt == 'c')
		args->count = check_argument_value(optarg);
	else if (opt == 'D')
		args->print_timestamps = true;
	else if (opt == 'h' || opt == '?')
		print_usage();
	else if (opt == 'i')
	{
		args->interval = true;
		args->interval_seconds = check_argument_value(optarg);
	}
	else if (opt == 'q')
		args->quiet_mode = true;
	else if (opt == 't')
		args->ttl = check_argument_value_ttl(optarg);
	else if (opt == 'v')
		args->verbose_mode = true;
	else if (opt == 'V')
	{
		printf("ft_ping 1.0 based on ping build from inetutils 2.0\n");
		exit(EXIT_SUCCESS);
	}
	else if (opt == 'W')
		args->timeout = check_argument_value(optarg);
}

void	parse_arguments(int argc, char **argv, t_arguments *args)
{
	int		opt;

	args->interval_seconds = DEFAULT_INTERVAL;
	args->count = INT32_MAX;
	args->ttl = DEFAULT_TTL;
	args->timeout = DEFAULT_TIMEOUT;
	opt = getopt(argc, argv, "c:Dh?i:qt:vVW:");
	while (opt != -1)
	{
		parse_options(opt, args);
		opt = getopt(argc, argv, "c:Dh?i:qt:vVW:");
	}
	if (optind >= argc)
		print_error_and_exit("Destination address required");
	if (getuid() != 0)
		print_error_and_exit("Superuser privileges needed to run the program.");
	args->dest = argv[optind];
	return ;
}
