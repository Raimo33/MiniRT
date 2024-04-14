/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoui.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/19 17:21:29 by egualand          #+#    #+#             */
/*   Updated: 2024/04/14 10:35:54 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

uint8_t	ft_atoui(const char *str)
{
	uint8_t		result;
	uint16_t	i;

	i = 0;
	result = 0;
	if (ft_isalpha(str[i]))
		ft_quit(4, "invalid number format");
	while (ft_isdigit(str[i]))
		result = result * 10 + (str[i++] - '0');
	if (ft_isalpha(str[i]))
		ft_quit(4, "invalid number format");
	return (result);
}