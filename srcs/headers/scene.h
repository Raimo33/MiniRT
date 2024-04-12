/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:45:05 by craimond          #+#    #+#             */
/*   Updated: 2024/04/12 20:28:49 by craimond         ###   ########.fr       */
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
	t_vector	forward;
	t_vector	right;
	t_vector	up;
	t_vector	right_by_half_viewport_width;
	t_vector	up_by_half_viewport_height;
}	t_camera;

typedef struct s_light
{
	t_point		center;
	double		brightness;
	t_color		color;
}	t_light;

typedef struct s_amblight
{
	double		brightness;
	t_color		color;
	t_color		ambient;
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
	t_list		*cameras;
	t_camera	*current_camera;
	t_vector	world_min;
	t_vector	world_max;
	t_amblight	amblight;
	uint16_t	n_lights;
}	t_scene;

#endif