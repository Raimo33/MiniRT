/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoui.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/19 17:21:29 by egualand          #+#    #+#             */
/*   Updated: 2024/04/24 21:10:58 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

uint8_t	ft_atoui(const char *str)
{
	uint8_t		result;
	uint16_t	i;

	i = 0;
	result = 0;
	if (!str)
		return (0);
	while (ft_isdigit(str[i]))
		result = result * 10 + (str[i++] - '0');
	return (result);
}
