/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtok.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/19 16:50:25 by egualand          #+#    #+#             */
/*   Updated: 2024/03/23 22:05:00 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

static uint16_t	ft_strspn(const char *str, const char *sep);
static char		*ft_strpbrk(char *str, const char *sep);

char	*ft_strtok(char *const str, const char *const sep)
{
	char			*token;
	char			*end;
	static char		*current_token = NULL;

	if (str)
		current_token = str;
	else if (!current_token)
		return (NULL);
	current_token += ft_strspn(current_token, sep);
	if (*current_token == '\0')
	{
		current_token = NULL;
		return (NULL);
	}
	end = ft_strpbrk(current_token, sep);
	if (!end)
	{
		token = current_token;
		current_token = NULL;
		return (token);
	}
	*end = '\0';
	token = current_token;
	current_token = end + 1;
	return (token);
}

static uint16_t	ft_strspn(const char *str, const char *sep)
{
	uint16_t	i;

	i = 0;
	while (str[i])
	{
		if (ft_strchr(sep, str[i]) == NULL)
			break ;
		i++;
	}
	return (i);
}

static char	*ft_strpbrk(char *str, const char *sep)
{
	uint16_t	i;

	i = 0;
	while (str[i])
	{
		if (ft_strchr(sep, str[i]) != NULL)
			return (str + i);
		i++;
	}
	return (NULL);
}