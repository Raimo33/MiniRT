/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minirt.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 17:33:27 by craimond          #+#    #+#             */
/*   Updated: 2024/04/18 13:44:16 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINIRT_H
# define MINIRT_H

# include <stdint.h>
# include <float.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <math.h>
# include <stdint.h>
# include <fcntl.h>
# include <pthread.h>
# include <sys/time.h>
# include "../../minilibx-linux/mlx.h"
# include "../../libft/libft.h"
# include "../headers/get_next_line.h"
# include "../headers/primitives.h"
# include "utils.h"
# include "scene.h"

//valori ideali
# define WIN_SIZE 0.5
# define WORLD_SIZE 1000
# define OCTREE_DEPTH 4
# define N_THREADS 8

# define BOUNCE_ATTENUATION_FACTOR 0.8
# define BACKGROUND_COLOR 0x000000
# define KEY_ESC 65307
# define KEY_SPACE 32

# define CHECKERBOARD_TILE_DENSITY 10
# define CHECKERBOARD_COLOR1 0x000000
# define CHECKERBOARD_COLOR2 0xFFFFFF

static const char		spaces[] = " \t\n\v\f\r";

typedef struct s_mlx_data
{
	void			*win;
	void			*mlx;
	void			**images;
	char			**addrresses;
	double			*viewport_x;
	double			*viewport_y;
	double			aspect_ratio;
	int32_t			win_width;
	int32_t			win_height;
	int32_t			endian;
	int32_t			bits_per_pixel;
	int32_t			line_length;
	int16_t			current_img;
	uint16_t		n_images;
	uint8_t			bytes_per_pixel;
}	t_mlx_data;

typedef struct s_hook_data
{
	t_mlx_data	*win_data;
	t_scene		*scene;
}	t_hook_data;

typedef struct s_thread_data
{
	t_mlx_data	*win_data;
	t_scene		*scene;
	double		*light_ratios;
	uint16_t	start_y;
	uint16_t	end_y;
}	t_thread_data;

//TODO aggiustare i const
void			check_args(const uint16_t argc, char **argv);
void			init_scene(t_scene *scene);
void			parse_scene(const int fd, t_scene *scene);
void			setup_scene(t_scene *scene);
void			init_window(t_mlx_data *win_data, t_scene *scene);
void			init_hooks(t_mlx_data *win_data, t_scene *scene);
void			render_scene(t_mlx_data *win_data, t_scene *scene);
t_hit			*trace_ray(const t_scene *scene, const t_ray ray);
t_color			add_lighting(const t_scene *scene, t_color color, const t_hit *hit_info, const double *light_ratio);
double			intersect_ray_sphere(const t_ray ray, const t_shape *shape);
double			intersect_ray_plane(const t_ray ray, const t_shape *shape);
double			intersect_ray_cylinder(const t_ray ray, const t_shape *shape);
double			intersect_ray_triangle(const t_ray, const t_shape *shape);
double			intersect_ray_cone(const t_ray ray, const t_shape *shape);
bool			ray_intersects_aabb(t_ray ray, t_point bounding_box_max, t_point bounding_box_min);
t_point			ray_point_at_parameter(const t_ray ray, double t);
void			get_uv(const t_hit *hit_info, double *u, double *v);
double			*precompute_ratios(uint16_t n_elems);
void			precompute_viewports(t_mlx_data *win_data);
t_thread_data	**set_threads_data(t_scene *scene, t_mlx_data *win_data, double *light_ratios, uint16_t lines_per_thread, pthread_attr_t *thread_attr);
void			set_thread_attr(pthread_attr_t *thread_attr);
t_color			blend_colors(const t_color color1, const t_color color2, double ratio);
void			setup_camera(t_camera *cam, const t_mlx_data *win_data);
void 			ft_quit(const uint8_t id, const char *msg);
void 			destroy_scene(t_scene *scene);
int 			close_win(t_hook_data *hook_data);
void			my_mlx_pixel_put(const t_mlx_data *data, const uint16_t x, const uint16_t y, const t_color color);

#endif