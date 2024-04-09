/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minirt.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 17:33:27 by craimond          #+#    #+#             */
/*   Updated: 2024/04/09 19:31:10 by craimond         ###   ########.fr       */
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
# include "scene.h"

//valori ideali
# define WIN_WIDTH 680
# define WIN_HEIGHT 400
# define PLANE_SIZE 100
# define RAYS_PER_PIXEL 300
# define BACKGROUND_COLOR 0x000000
# define MAX_BOUNCE 20
# define OCTREE_DEPTH 3
# define N_THREADS 8

//valori di test
// # define WIN_WIDTH 300
// # define WIN_HEIGHT 300
// # define PLANE_SIZE 100
// # define RAYS_PER_PIXEL 3
// # define BACKGROUND_COLOR 0x000000
// # define MAX_BOUNCE 2
// # define OCTREE_DEPTH 3

# ifndef N_THREADS
#  define N_THREADS 8
# endif

# if N_THREADS > RAYS_PER_PIXEL / 2
#  undef N_THREADS
#  define N_THREADS RAYS_PER_PIXEL / 2
# endif
# if N_THREADS < 1
#  undef N_THREADS
#  define N_THREADS 1
# endif

# define KEY_ESC 65307

static const char		spaces[] = " \t\n\v\f\r";

typedef struct s_mlx_data
{
	void			*win;
	void			*mlx;
	int				endian;
	int				bits_per_pixel;
	uint8_t			bytes_per_pixel;
	int				line_length;
	void			*img;
	char			*addr;
}	t_mlx_data;

typedef struct s_hook_data
{
	t_mlx_data	*win_data;
	t_scene		scene;
}	t_hook_data;

typedef struct s_thread_data
{
	t_scene		*scene;
	t_ray		ray;
	t_color		*colors_array;
	uint16_t	start_depth;
	uint16_t	end_depth;
}	t_thread_data;

//TODO aggiustare i const
void			check_args(const uint16_t argc, char **argv);
void			init_scene(t_scene *scene);
void			init_window(t_mlx_data *win_data);
void			init_hooks(t_mlx_data *win_data, t_scene scene);
void			parse_scene(int fd, t_scene *scene);
void			set_bounding_box(t_shape *shape);
void			setup_scene(t_scene *scene);
void			render_scene(t_mlx_data *mlx_data, t_scene *scene);
void 			ft_quit(uint8_t id, char *msg);
int				close_win(t_hook_data *hook_data);
bool			is_empty_line(const char *line);
bool			is_comment(const char *line);
bool			is_space(char c);
float 			ft_atof(const char *str);
uint8_t 		ft_atoui(const char *str);
char 			*ft_strtok(char *const str, const char *const sep);
bool			ray_intersects_aabb(t_ray ray, t_point bounding_box_max, t_point bounding_box_min);
float			intersect_ray_cylinder(const t_ray ray, const t_shape *shape);
float			intersect_ray_sphere(const t_ray ray, const t_shape *shape);
float			intersect_ray_plane(const t_ray ray, const t_shape *shape);
void			my_mlx_pixel_put(const t_mlx_data *data, const uint16_t x, const uint16_t y, const t_color color);
void			my_mlx_stack_image(t_mlx_data *data);
char			*my_mlx_get_data_addr(void *img_ptr, int32_t *bits_per_pixel, int32_t *size_line, int32_t *endian);
float			fclamp(const float value, const float min, const float max);

#endif