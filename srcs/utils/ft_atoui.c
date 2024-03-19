/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoui.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/19 17:21:29 by egualand          #+#    #+#             */
/*   Updated: 2024/03/19 17:29:57 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minirt.h"

uint8_t	ft_atoui(const char *str)
{
	uint8_t		result;
	uint16_t	i;

	i = 0;
	result = 0;
	while (ft_isdigit(str[i]))
		result = result * 10 + (str[i++] - '0');
	return (result);
}