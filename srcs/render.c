/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/03/28 13:59:04 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static t_ray	get_ray(t_scene scene, uint16_t x, uint16_t y);
static t_color	trace_ray(t_scene scene, t_ray ray);

void render(const t_mlx_data mlx_data, const t_scene scene)
{
	t_ray	ray;
	t_color	color;
	int		x;
	int		y;

	y = 0;
	while (y < WIN_HEIGHT)
	{
		x = 0;
		//implementare threads (ognuno fa un tot di righe)
		while (x < WIN_WIDTH)
		{
			ray = get_ray(scene, x, y);
			color = trace_ray(scene, ray);
			my_mlx_pixel_put(mlx_data, x, y, color);
			x++;
		}
		y++;
	}
	mlx_put_image_to_window(mlx_data.mlx, mlx_data.win, mlx_data.img, 0, 0);
}

static t_ray	get_ray(const t_scene scene, const uint16_t x, const uint16_t y)
{
	const t_vector	world_up = {0, 1, 0};
	const float		aspect_ratio = (float)WIN_WIDTH / (float)WIN_HEIGHT;
	const float		fov = scene.camera.fov * M_PI / 180; //fov da grad in radianti
	const float		viewport_height = 2 * tan(fov / 2);
	const float		viewport_width = aspect_ratio * viewport_height;

	//calcolo dei vettori base della camera
	const t_vector	forward = vec_normalize(vec_negate(scene.camera.normal)); //opposto della normale (la normale punta verso l'osservatore)
	const t_vector	right = vec_normalize(vec_cross(world_up, forward)); //trova il terzo vettore perpendicolare ad entrambi
	const t_vector	up = vec_normalize(vec_cross(forward, right)); //trova il terzo vettore perpendicolare ad entrambi

	//calcolo della posizione del viewport
	//centro del viewport: (0, 0)
	//angolo in alto sx del viewport: (-1, 1), angolo alto dx del viewport: (1, 1), angolo basso sx del viewport: (-1, -1), angolo basso dx del viewport: (1, -1)

	//coordinate sul piano del monitor [0, 1]
	const float		screen_viewport_x = x / (float)(WIN_WIDTH - 1);
	const float		screen_viewport_y = y / (float)(WIN_HEIGHT - 1);
	//coordinate sul piano cartesiano vero [-1, 1]
	const float		real_viewport_x = screen_viewport_x * 2 - 1;
	const float		real_viewport_y = 1 - screen_viewport_y * 2;
	
	//calcolo del vettore che parte dalla camera verso il pixel (sul piano immaginario viewport)
	const t_vector direction = //vettore direzione
	{
		.x = forward.x + (real_viewport_x * right.x * viewport_width / 2) + (real_viewport_y * up.x * viewport_height / 2),
		.y = forward.y + (real_viewport_x * right.y * viewport_width / 2) + (real_viewport_y * up.y * viewport_height / 2),
		.z = forward.z + (real_viewport_x * right.z * viewport_width / 2) + (real_viewport_y * up.z * viewport_height / 2)
	};
	const t_ray	ray_direction = //raggio (vettore direzione spostato per partire dalla camera)
	{
		.origin = scene.camera.center,
		.direction = vec_normalize(direction)
	};
	return (ray_direction);
}

static t_color	trace_ray(const t_scene scene, const t_ray ray)
{
	(void)scene;
	(void)ray;
	//TODO implementare il tracciamento dei raggi
	
	return ((t_color){0, 255, 0});
}