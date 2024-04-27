/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:24:04 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 16:10:19 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

void	render_scene(t_mlx_data *w, t_scene *scene)
{
	t_thread_data	**threads_data;
	pthread_attr_t	thread_attr;
	t_list			*cameras;
	double			*light_ratios;
	const uint16_t	lines_per_thread
		= roundf((float)w->win_height / (float)N_THREADS);

	cameras = scene->cameras;
	light_ratios = precompute_ratios(ft_lstsize(scene->lights));
	threads_data = set_threads_data((t_scene_windata){scene, w},
			light_ratios, lines_per_thread, &thread_attr);
	while (cameras)
	{
		printf("\nRendering camera %d\n", w->current_img);
		scene->current_camera = cameras->content;
		setup_camera(scene->current_camera, w);
		fill_image(threads_data, &thread_attr);
		cameras = cameras->next;
		w->current_img++;
	}
	w->current_img = w->n_images - 1;
	mlx_put_image_to_window(w->mlx, w->win, w->images[w->current_img], 0, 0);
	pthread_attr_destroy(&thread_attr);
	free(light_ratios);
	ft_freematrix((void **)threads_data);
}

void	fill_image(t_thread_data **threads_data,
	pthread_attr_t *thread_attr)
{
	pthread_t	thread_ids[N_THREADS];
	uint16_t	i;

	i = 0;
	while (i < N_THREADS)
	{
		pthread_create(&thread_ids[i], thread_attr,
			render_segment, threads_data[i]);
		i++;
	}
	while (i > 0)
		pthread_join(thread_ids[--i], NULL);
}

void	render_row(t_thread_data *td, const uint16_t y)
{
	uint16_t	x;
	t_color		c;
	t_hit		*first_hit;

	x = 0;
	while (x < td->win_data->win_width)
	{
		first_hit = trace_ray(td->scene,
				get_ray(td->scene->current_camera, x, y, td->win_data));
		if (first_hit)
		{
			c = add_texture(first_hit);
			c = add_lighting(td->scene, c, first_hit, td->light_ratios);
		}
		else
			c = (t_color){BG_C >> 16 & 255, BG_C >> 8 & 255, BG_C & 255};
		my_mlx_pixel_put(td->win_data, x, y, c);
		free(first_hit);
		x++;
	}
}

void	*render_segment(void *data)
{
	t_thread_data	*td;
	uint16_t		y;

	td = (t_thread_data *)data;
	y = td->start_y;
	while (y < td->end_y)
	{
		write(1, ".", 1);
		render_row(td, y);
		y++;
	}
	return (NULL);
}

t_ray	get_ray(const t_camera *cam, const uint16_t x,
					const uint16_t y, const t_mlx_data *win_data)
{
	const t_vector	direction
		= {
		.x = cam->forward.x + (win_data->viewport_x[x]
			* cam->right_by_half_viewport_width.x)
		+ (win_data->viewport_y[y]
			* cam->up_by_half_viewport_height.x),
		.y = cam->forward.y + (win_data->viewport_x[x]
			* cam->right_by_half_viewport_width.y)
		+ (win_data->viewport_y[y]
			* cam->up_by_half_viewport_height.y),
		.z = cam->forward.z + (win_data->viewport_x[x]
			* cam->right_by_half_viewport_width.z)
		+ (win_data->viewport_y[y]
			* cam->up_by_half_viewport_height.z)
	};

	return ((t_ray){cam->center, vec_normalize(direction)});
}
