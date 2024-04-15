/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   protected_methods.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 18:38:50 by craimond          #+#    #+#             */
/*   Updated: 2024/04/15 18:46:50 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

void	*malloc_p(size_t size)
{
	void	*ptr;

	ptr = malloc(size);
	if (!ptr)
		ft_quit(1, "fatal: failed to allocate memory");
	return (ptr);
}