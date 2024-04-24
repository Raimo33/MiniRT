/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conditionals.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:32:50 by craimond          #+#    #+#             */
/*   Updated: 2024/04/24 21:10:49 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

inline bool	is_comment(const char *line)
{
	return (*line == '#');
}

inline bool	ft_isspace(const char c)
{
	return (ft_strchr(spaces, c) != NULL);
}

bool	is_empty_line(const char *line)
{
	while (*line)
	{
		if (!ft_isspace(*line))
			return (false);
		line++;
	}
	return (true);
}
