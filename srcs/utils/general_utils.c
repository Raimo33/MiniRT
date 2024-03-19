/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   general_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/19 15:07:39 by egualand          #+#    #+#             */
/*   Updated: 2024/03/19 15:09:40 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minirt.h"

void ft_quit(uint8_t id, char *msg)
{
	if (errno != 0)
		perror("miniRT");
	else
		printf("miniRT: %s\n", msg);
	exit(id);
}