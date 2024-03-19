/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atof.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/19 17:09:28 by egualand          #+#    #+#             */
/*   Updated: 2024/03/19 17:29:38 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minirt.h"

float	ft_atof(const char *str)
{
	float		result;
	float		sign;
	float		dec;
	uint16_t	i;

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
	while (ft_isdigit(str[i]))
	{
		dec = dec * 10 + (str[i++] - '0');
		result = result + dec / 10;
	}
	return (result * sign);
}
