/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 13:58:08 by craimond          #+#    #+#             */
/*   Updated: 2024/04/12 15:10:31 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

t_vector	get_rand_in_unit_sphere(void) //metodo di Marsaglia (meno leggibile ma piu' efficiente)
{
    double		u;
	double		v;
	double		s;
    t_vector	p;

    do {
        u = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        v = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        s = u*u + v*v;
    } while (s >= 1 || s == 0);

    double multiplier = 2 * sqrt(1 - s);
    p.x = u * multiplier;
    p.y = v * multiplier;
    p.z = 1 - 2 * s;
    return (p);
}

void	set_thread_attr(pthread_attr_t *thread_attr)
{
	pthread_attr_init(thread_attr);
	pthread_attr_setschedpolicy(thread_attr, SCHED_FIFO);
	pthread_attr_setscope(thread_attr, PTHREAD_SCOPE_PROCESS);
	pthread_attr_setdetachstate(thread_attr, PTHREAD_CREATE_JOINABLE);
}

t_thread_data	**set_threads_data(t_scene *scene, t_mlx_data *win_data, double *light_ratios, double *attenuation_factors, uint16_t lines_per_thread, pthread_attr_t *thread_attr)
{
	uint16_t		i;
	t_thread_data	**threads_data;

	threads_data = (t_thread_data **)malloc((N_THREADS + 1) * sizeof(t_thread_data *));
	i = 0;
	while (i < N_THREADS)
	{
		threads_data[i] = (t_thread_data *)malloc(sizeof(t_thread_data));
		threads_data[i]->scene = scene;
		threads_data[i]->win_data = win_data;
		threads_data[i]->light_ratios = light_ratios;
		threads_data[i]->attenuation_factors = attenuation_factors;
		threads_data[i]->start_y = i * lines_per_thread;
		if (i == N_THREADS - 1)
			threads_data[i]->end_y = WIN_HEIGHT;
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
	
	ratios = (double *)malloc(n_elems * sizeof(double));
	ratios[0] = 1.0f;
	i = 1;
	while (i < n_elems)
	{
		ratios[i] = 1.0f / (i + 1);
		i++;
	}
	return (ratios);
}

double	*precoumpute_attenuation_factors(void)
{
	uint16_t	i;
	double		*attenuation_factors;
	double		safety_limit = EPSILON * 5;

	attenuation_factors = (double *)malloc((MAX_BOUNCE + 1) * sizeof(double));
	attenuation_factors[0] = 1.0f;
	i = 1;
	while (i <= MAX_BOUNCE)
	{
		attenuation_factors[i] = attenuation_factors[i - 1] * ATTENUATION_FACTOR;
		if (attenuation_factors[i] <= safety_limit) //safety measure to avoid division by zero and floating point issues
		{
			while (i <= MAX_BOUNCE)
				attenuation_factors[i++] = safety_limit;
			break ;
		}
		i++;
	}
	return (attenuation_factors);
}

t_color	blend_colors(const t_color color1, const t_color color2, double ratio)
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

void	setup_camera(t_camera *cam)
{
	const t_vector		world_up = {0, 1, 0};
	const double		aspect_ratio = (double)WIN_WIDTH / (double)WIN_HEIGHT;
	const double		rad_fov = cam->fov * M_PI / 180;

	cam->viewport_height = 2 * tan(rad_fov / 2);
	cam->viewport_width = aspect_ratio * cam->viewport_height;
	cam->forward = vec_normalize(cam->normal);
	if (are_vectors_parallel(cam->forward, world_up))
		cam->right = (t_vector){1, 0, 0};
	else
		cam->right = vec_normalize(vec_cross(world_up, cam->forward));
	cam->up = vec_normalize(vec_cross(cam->forward, cam->right));
}