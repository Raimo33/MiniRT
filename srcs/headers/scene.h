/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:45:05 by craimond          #+#    #+#             */
/*   Updated: 2024/03/23 21:59:38 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCENE_H
# define SCENE_H

# include "shapes.h"
# include "../../libft/libft.h"

typedef struct s_camera
{
	t_coord		point;
	t_coord		normal;
	uint8_t		fov;
}	t_camera;

typedef struct s_light
{
	t_coord		point;
	float		brightness;
	t_color		color;
}	t_light;

typedef struct s_amblight
{
	float		brightness;
	t_color		color;
}	t_amblight;

typedef struct	s_scene
{
	t_list		*spheres;
	t_list		*planes;
	t_list		*cylinders;
	t_list		*lights;
	t_camera	camera;
	t_amblight	amblight;
}	t_scene;

#endif