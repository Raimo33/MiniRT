/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:45:05 by craimond          #+#    #+#             */
/*   Updated: 2024/03/24 19:48:35 by craimond         ###   ########.fr       */
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

typedef struct	t_grid
{
	t_list		**cells; //un array di liste di shape. Ogni cella contiene le shape che la occupano
	t_point		world_min;
	t_point		world_max;
	t_uint3		n_cells;
}	t_grid;

typedef struct	s_scene
{
	t_grid		grid;
	t_list		*shapes;
	t_list		*lights;
	t_camera	camera;
	t_amblight	amblight;
}	t_scene;

#endif