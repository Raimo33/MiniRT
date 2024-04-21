/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   singleton.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/21 14:47:54 by craimond          #+#    #+#             */
/*   Updated: 2024/04/21 14:53:28 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

t_scene *get_scene(t_scene *_scene)
{
	static t_scene	*scene = NULL;

	if (_scene)
		scene = _scene;
	return (scene);
}