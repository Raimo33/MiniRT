/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:45:05 by craimond          #+#    #+#             */
/*   Updated: 2024/04/04 15:33:27 by craimond         ###   ########.fr       */
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
	float		viewport_height;
	float		viewport_width;
	t_vector	forward;
	t_vector	right;
	t_vector	up;
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
	struct s_octree	**children;
	t_list			*shapes;
	uint16_t		n_shapes;
	t_vector		box_top;
	t_vector		box_bottom;
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