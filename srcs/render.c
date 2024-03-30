/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/03/30 15:52:51 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void		setup_camera(t_camera *cam);
static t_ray	get_ray(const t_camera *cam, const uint16_t x, const uint16_t y);
static t_color	trace_ray(const t_scene scene, const t_ray ray);
static t_point	ray_point_at_parameter(const t_ray ray, float t);
static bool		check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static bool		traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static t_color	compute_color_at_intersection(const t_hit hit, const t_scene scene);

void render(const t_mlx_data mlx_data, t_scene scene)
{
	t_ray	ray;
	t_color	color;
	int		x;
	int		y;
	
	setup_camera(&scene.camera);
	y = 0;
	while (y < WIN_HEIGHT)
	{
		x = 0;
		//implementare threads (ognuno fa un tot di righe)
		while (x < WIN_WIDTH)
		{
			ray = get_ray(&scene.camera, x, y);
			color = trace_ray(scene, ray);
			my_mlx_pixel_put(mlx_data, x, y, color);
			x++;
		}
		y++;
	}
	mlx_put_image_to_window(mlx_data.mlx, mlx_data.win, mlx_data.img, 0, 0);
}

static void	setup_camera(t_camera *cam)
{
	const t_vector	world_up = {0, 1, 0};
	const float		aspect_ratio = (float)WIN_WIDTH / (float)WIN_HEIGHT;
	const float		rad_fov = cam->fov * M_PI / 180; //fov da grad in radianti

	cam->viewport_height = 2 * tan(rad_fov / 2);
	cam->viewport_width = aspect_ratio * cam->viewport_height;
	//calcolo dei vettori base della camera
	cam->forward = vec_normalize(cam->normal); //la normale punta verso il punto di interesse
	if (is_vec_equal(cam->forward, world_up)) //se la normale è uguale al vettore up del mondo
		cam->right = (t_vector){1, 0, 0};
	else
		cam->right = vec_normalize(vec_cross(world_up, cam->forward)); //trova il terzo vettore perpendicolare ad entrambi
	cam->up = vec_normalize(vec_cross(cam->forward, cam->right)); //trova il terzo vettore perpendicolare ad entrambi
}

static t_ray	get_ray(const t_camera *cam, const uint16_t x, const uint16_t y)
{
	//centro del viewport: (0, 0)
	//coordinate sul piano del monitor [0, 1]
	const float		screen_viewport_x = x / (float)(WIN_WIDTH - 1);
	const float		screen_viewport_y = y / (float)(WIN_HEIGHT - 1);
	//coordinate sul piano cartesiano vero [-1, 1]
	const float		real_viewport_x = screen_viewport_x * 2 - 1;
	const float		real_viewport_y = 1 - screen_viewport_y * 2;
	//calcolo del vettore che parte dalla camera verso il pixel (sul piano immaginario viewport)
	const t_vector direction = //vettore direzione
	{
		.x = cam->forward.x + (real_viewport_x * cam->right.x * cam->viewport_width / 2) + (real_viewport_y * cam->up.x * cam->viewport_height / 2),
		.y = cam->forward.y + (real_viewport_x * cam->right.y * cam->viewport_width / 2) + (real_viewport_y * cam->up.y * cam->viewport_height / 2),
		.z = cam->forward.z + (real_viewport_x * cam->right.z * cam->viewport_width / 2) + (real_viewport_y * cam->up.z * cam->viewport_height / 2)
	};
	const t_ray	ray_direction = //raggio (vettore direzione spostato per partire dalla camera)
	{
		.origin = cam->center,
		.direction = vec_normalize(direction)
	};
	return (ray_direction);
}

static t_color	trace_ray(const t_scene scene, const t_ray ray)
{
    t_hit			closest_hit = 
	{
		.distance = FLT_MAX,
		.point = {0, 0, 0},
		.normal = {0, 0, 0},
		.material = NULL
	};
	t_color					color;
	static const t_color	bg_color = 
	{
		.r = (BACKGROUND_COLOR >> 16) & 0xFF,
		.g = (BACKGROUND_COLOR >> 8) & 0xFF,
		.b = BACKGROUND_COLOR & 0xFF
	};

    if (!traverse_octree(scene.octree, ray, &closest_hit)) //aka no hit found
		return (bg_color);
    // Calcolare il colore del pixel in base all'intersezione più vicina
	// Senza considerare le luci e i materiali per ora
    color = compute_color_at_intersection(closest_hit, scene);
    return (color);
}

static bool	traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit)
{
	uint8_t	i;

	// Controllo se il raggio interseca il bounding box del nodo
	if (!ray_intersects_aabb(ray, node->box_top, node->box_bottom))
		return (false);

	// Se c'e' solo una shape all'interno o se il nodo è una leaf, controlla le intersezioni con le forme
	if (node->n_shapes == 1 || node->children == NULL)
		return (check_shapes_in_node(node, ray, closest_hit));
	else
	{
		// Altrimenti, continua a scendere nell'albero
		i = 0;
		while (i < 8)
		{
			if (node->children[i])
				traverse_octree(node->children[i], ray, closest_hit);
			i++;
		}
	}
	return (false);
}

static bool check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit)
{
	static float (*const intersect[3])(const t_ray, const t_shape *) = //stesso ordine di enum e struct
	{
		&intersect_ray_sphere,
		&intersect_ray_cylinder,
		&intersect_ray_plane
	};
	t_list	*shapes;
	t_shape	*shape;
	float	t;
	bool	has_hit;

	has_hit = false;
	shapes = node->shapes;
	while (shapes)
	{
		shape = (t_shape *)shapes->content;
		t = intersect[shape->type](ray, shape);
		if (t > 0 && t < closest_hit->distance)
		{
			closest_hit->distance = t;
			closest_hit->point = ray_point_at_parameter(ray, t);
			closest_hit->material = &shape->material;
			switch (shape->type)
			{
				case SPHERE:
				case CYLINDER:
					closest_hit->normal = vec_normalize(vec_sub(closest_hit->point, shape->cylinder.center));
					break;
				case PLANE:
					closest_hit->normal = shape->plane.normal;
					break;
			}
			has_hit = true;
		}
		shapes = shapes->next;
	}
	return (has_hit);
}

static t_point ray_point_at_parameter(const t_ray ray, float t)
{
    t_point	point;

    point.x = ray.origin.x + t * ray.direction.x;
    point.y = ray.origin.y + t * ray.direction.y;
    point.z = ray.origin.z + t * ray.direction.z;

    return (point);
}

static t_color compute_color_at_intersection(const t_hit hit, const t_scene scene)
{
	(void)scene;

	//TODO implementare il calcolo del colore
	return (hit.material->color);
}