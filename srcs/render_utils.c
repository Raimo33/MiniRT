/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 13:58:08 by craimond          #+#    #+#             */
/*   Updated: 2024/04/17 15:19:40 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

void	set_thread_attr(pthread_attr_t *thread_attr)
{
	pthread_attr_init(thread_attr);
	pthread_attr_setschedpolicy(thread_attr, SCHED_FIFO);
	pthread_attr_setscope(thread_attr, PTHREAD_SCOPE_PROCESS);
	pthread_attr_setdetachstate(thread_attr, PTHREAD_CREATE_JOINABLE);
}

t_thread_data	**set_threads_data(t_scene *scene, t_mlx_data *win_data, double *light_ratios, uint16_t lines_per_thread, pthread_attr_t *thread_attr)
{
	uint16_t		i;
	t_thread_data	**threads_data;

	threads_data = (t_thread_data **)calloc_p((N_THREADS + 1), sizeof(t_thread_data *));
	i = 0;
	while (i < N_THREADS)
	{
		threads_data[i] = (t_thread_data *)calloc_p(1, sizeof(t_thread_data));
		threads_data[i]->scene = scene;
		threads_data[i]->win_data = win_data;
		threads_data[i]->light_ratios = light_ratios;
		threads_data[i]->start_y = i * lines_per_thread;
		if (i == N_THREADS - 1)
			threads_data[i]->end_y = win_data->win_height;
		else
			threads_data[i]->end_y = (i + 1) * lines_per_thread;
		i++;
	}
	threads_data[N_THREADS] = NULL;
	set_thread_attr(thread_attr);
	return (threads_data);
}

double	*precompute_ratios(uint16_t n_elems)
{
	uint16_t	i;
	double 		*ratios;
	
	ratios = (double *)calloc_p(n_elems, sizeof(double));
	ratios[0] = 1.0f;
	i = 1;
	while (i < n_elems)
	{
		ratios[i] = 1.0f / (i + 1);
		i++;
	}
	return (ratios);
}

void	precompute_viewports(t_mlx_data *win_data)
{
	uint16_t	x;
	uint16_t	y;

	y = 0;
	while (y < win_data->win_height)
	{
		x = 0;
		while (x < win_data->win_width)
		{
			win_data->viewport_x[x] = (x / (double)(win_data->win_width - 1)) * 2 - 1;
			win_data->viewport_y[y] = 1 - (y / (double)(win_data->win_height - 1)) * 2;
			x++;
		}
		y++;
	}
}

inline t_color	blend_colors(const t_color color1, const t_color color2, double ratio)
{
	t_color		result;
	double		negative_ratio;

    ratio = fclamp(ratio, 0.0f, 1.0f);
	negative_ratio = 1.0f - ratio;
	result.r = (color1.r * negative_ratio + color2.r * ratio);
	result.g = (color1.g * negative_ratio + color2.g * ratio);
	result.b = (color1.b * negative_ratio + color2.b * ratio);
	return (result);
}

void	setup_camera(t_camera *cam, const t_mlx_data *win_data)
{
	static const t_vector	world_up = {0, 1, 0};
	const double			rad_fov = cam->fov * M_PI / 180;
	const double 			viewport_height = 2 * tan(rad_fov / 2);
	const double			viewport_width = win_data->aspect_ratio * viewport_height;

	cam->forward = vec_normalize(cam->normal);
	if (are_vectors_parallel(cam->forward, world_up))
		cam->right = (t_vector){1, 0, 0};
	else
		cam->right = vec_normalize(vec_cross(world_up, cam->forward));
	cam->up = vec_normalize(vec_cross(cam->forward, cam->right));
	cam->right_by_half_viewport_width = vec_scale(viewport_width / 2, cam->right);
	cam->up_by_half_viewport_height = vec_scale(viewport_height / 2, cam->up);
}