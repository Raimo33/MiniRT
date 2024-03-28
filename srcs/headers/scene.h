/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:45:05 by craimond          #+#    #+#             */
/*   Updated: 2024/03/28 14:50:57 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCENE_H
# define SCENE_H

# include "shapes.h"
# include "../../libft/libft.h"

typedef struct s_camera
{
	t_point		center;
	t_vector	normal;
	uint8_t		fov;
}	t_camera;

typedef struct s_light
{
	t_point		center;
	float		brightness;
	t_color		color;
}	t_light;

typedef struct s_amblight
{
	float		brightness;
	t_color		color;
}	t_amblight;

typedef	struct s_octree
{
	struct s_octree	*children[8];
	t_list			*shapes;
	uint16_t		n_shapes;
	t_vector		box_top;
	t_vector		box_bottom;
	int 			depth;
}	t_octree;

typedef struct	s_scene
{
	t_octree	*octree;
	t_list		*shapes;
	t_list		*lights;
	t_camera	camera;
	t_amblight	amblight;
	t_vector	world_min;
	t_vector	world_max;
}	t_scene;

#endif