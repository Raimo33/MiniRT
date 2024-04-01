/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atof.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/19 17:09:28 by egualand          #+#    #+#             */
/*   Updated: 2024/04/01 23:06:49 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

float	ft_atof(const char *str)
{
	float		result;
	float		sign;
	float		divisor;
	float		dec;
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
	while (ft_isdigit(str[i]))
	{
		dec = dec * 10 + (str[i++] - '0');
		divisor *= 10;
	}
	result += dec / divisor;
	return (result * sign);
}
