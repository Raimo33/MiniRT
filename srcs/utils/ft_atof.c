/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atof.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/19 17:09:28 by egualand          #+#    #+#             */
/*   Updated: 2024/04/14 10:35:33 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

double	ft_atof(const char *str)
{
	double		result;
	double		sign;
	double		divisor;
	double		dec;
	uint16_t	i;

	if (!str)
		return (0);
	i = 0;
	result = 0;
	sign = 1;
	dec = 0;
	if (str[i] == '-')
	{
		sign = -1;
		i++;
	}
	while (ft_isdigit(str[i]))
		result = result * 10 + (str[i++] - '0');
	if (str[i] == '.')
		i++;
	divisor = 1;
	if (ft_isalpha(str[i]))
		ft_quit(4, "invalid number format");
	while (ft_isdigit(str[i]))
	{
		dec = dec * 10 + (str[i++] - '0');
		divisor *= 10;
	}
	if (ft_isalpha(str[i]))
		ft_quit(4, "invalid number format");
	result += dec / divisor;
	return (result * sign);
}
