/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse4.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:33:22 by craimond          #+#    #+#             */
/*   Updated: 2024/05/01 15:23:15 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

t_color	parse_color(char *str)
{
	t_color	color;

	if (!str)
		ft_quit(5, "invalid color syntax");
	color.r = ft_atoui(str);
	str = skip_commas(str);
	color.g = ft_atoui(str);
	str = skip_commas(str);
	color.b = ft_atoui(str);
	return (color);
}

char	*skip_commas(char *str)
{
	while (*str && *str != ',')
		str++;
	if (*str == '\0')
		ft_quit(5, "invalid syntax: missing comma");
	return (str + 1);
}

char	*get_word(char *line)
{
	char	*end;

	if (!line)
		return (NULL);
	end = line;
	while (*end != '\0' && ft_isspace(*end) == false)
		end++;
	*end = '\0';
	return (line);
}
