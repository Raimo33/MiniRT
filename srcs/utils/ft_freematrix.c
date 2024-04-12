/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_freematrix.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 15:07:44 by craimond          #+#    #+#             */
/*   Updated: 2024/04/12 15:08:11 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

void	ft_freematrix(void **matrix)
{
	void	**tmp;

	tmp = matrix;
	while (*tmp)
	{
		free(*tmp);
		tmp++;
	}
	free(matrix);
}
