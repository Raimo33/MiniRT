/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:45:05 by craimond          #+#    #+#             */
/*   Updated: 2024/03/24 16:13:59 by craimond         ###   ########.fr       */
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

typedef struct	s_scene
{
	t_list		*shapes;
	t_list		*lights;
	t_camera	camera;
	t_amblight	amblight;
}	t_scene;

#endif