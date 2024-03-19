/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minirt.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 17:33:27 by craimond          #+#    #+#             */
/*   Updated: 2024/03/19 17:47:11 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINIRT_H
# define MINIRT_H

# include <stdint.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <math.h>
# include <stdint.h>
# include <fcntl.h>
# include "../minilibx-linux/mlx.h"
# include "../libft/libft.h"
# include "get_next_line/get_next_line.h"

#define WIN_HEIGHT 480
#define WIN_WIDTH 640

# define KEY_ESC 65307

static const char	spaces[] = " \t\n\v\f\r";

typedef struct s_mlx_data
{
	void			*win;
	void			*img;
	void			*mlx;
	int				endian;
	int				bits_per_pixel;
	int				line_length;
	char			*addr;
}	t_mlx_data;

typedef struct s_color
{
	uint8_t		r;
	uint8_t		g;
	uint8_t		b;
}	t_color;

typedef struct s_coord
{
	float		x;
	float		y;
	float		z;
}	t_coord;

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

typedef struct s_sphere
{
	t_coord		center;
	float		radius;
	t_color		color;
}	t_sphere;

typedef struct s_plane
{
	t_coord		point;
	t_coord		normal;
	t_color		color;
}	t_plane;

typedef struct s_cylinder
{
	t_coord		point;
	t_coord		normal;
	float		diameter;
	float		height;
	t_color		color;
}	t_cylinder;

typedef struct	s_scene
{
	t_list		*spheres;
	t_list		*planes;
	t_list		*cylinders;
	t_list		*lights;
	t_camera	camera;
	t_amblight	amblight;
}	t_scene;

typedef struct s_hook_data
{
	t_mlx_data	*win_data;
	t_scene		scene;
}	t_hook_data;

void 	ft_quit(uint8_t id, char *msg);
float 	ft_atof(const char *str);
uint8_t ft_atoui(const char *str);
char 	*ft_strtok(char *const str, const char *const sep);

#endif