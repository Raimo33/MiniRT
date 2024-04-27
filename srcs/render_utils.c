/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 13:58:08 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 13:42:06 by egualand         ###   ########.fr       */
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

t_thread_data	**set_threads_data(t_scene_windata *scene_windata,
	double *light_ratios, uint16_t lines_per_thread,
	pthread_attr_t *thread_attr)
{
	uint16_t		i;
	t_thread_data	**threads_data;

	threads_data = (t_thread_data **)calloc_p((N_THREADS + 1),
			sizeof(t_thread_data *));
	i = 0;
	while (i < N_THREADS)
	{
		threads_data[i] = (t_thread_data *)calloc_p(1, sizeof(t_thread_data));
		threads_data[i]->scene = scene_windata->scene;
		threads_data[i]->win_data = scene_windata->win_data;
		threads_data[i]->light_ratios = light_ratios;
		threads_data[i]->start_y = i * lines_per_thread;
		if (i == N_THREADS - 1)
			threads_data[i]->end_y = scene_windata->win_data->win_height;
		else
			threads_data[i]->end_y = (i + 1) * lines_per_thread;
		i++;
	}
	threads_data[N_THREADS] = NULL;
	set_thread_attr(thread_attr);
	return (threads_data);
}

double	*precompute_ratios(const uint16_t n_elems)
{
	uint16_t	i;
	double		*ratios;

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
			win_data->viewport_x[x]
				= (x / (double)(win_data->win_width - 1)) * 2 - 1;
			win_data->viewport_y[y]
				= 1 - (y / (double)(win_data->win_height - 1)) * 2;
			x++;
		}
		y++;
	}
}

inline t_color	blend_colors(const t_color color1,
	const t_color color2, double ratio)
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
