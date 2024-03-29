/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/03/29 13:11:26 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static t_ray	get_ray(const t_scene scene, const uint16_t x, const uint16_t y);
static t_color	trace_ray(const t_scene scene, const t_ray ray);
static float	intersect_ray_sphere(const t_ray ray, const t_sphere sphere);
static t_point	ray_point_at_parameter(const t_ray ray, float t);
static bool		check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static bool		traverse_octree(const t_octree *node, const t_ray ray, t_hit *closest_hit);
static t_color	compute_color_at_intersection(const t_hit hit, const t_scene scene);

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

static float	intersect_ray_sphere(const t_ray ray, const t_sphere sphere)
{
    t_vector	oc = vec_sub(ray.origin, sphere.center);
    float 		a = vec_dot(ray.direction, ray.direction);
    float 		b = 2.0 * vec_dot(oc, ray.direction);
    float 		c = vec_dot(oc, oc) - sphere.radius * sphere.radius;
    float 		discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
        return (-1);
    else
	{
        float t1 = (-b - sqrt(discriminant)) / (2*a);
        float t2 = (-b + sqrt(discriminant)) / (2*a);
        if (t1 > 0)
			return (t1);
        if (t2 > 0)
			return (t2);
        return (-1);
    }
}

static t_point ray_point_at_parameter(const t_ray ray, float t)
{
    t_point	point;

    point.x = ray.origin.x + t * ray.direction.x;
    point.y = ray.origin.y + t * ray.direction.y;
    point.z = ray.origin.z + t * ray.direction.z;

    return (point);
}

static bool check_shapes_in_node(const t_octree *node, const t_ray ray, t_hit *closest_hit)
{
	t_list	*shapes;
	t_shape	*shape;
	float	t;
	bool	has_hit;

	has_hit = false;
	shapes = node->shapes;
	while (shapes)
	{
		shape = (t_shape *)shapes->content;
		if (shape->type == SPHERE)
		{
            t = intersect_ray_sphere(ray, shape->sphere);
            if (t > 0 && t < closest_hit->distance)
			{
                closest_hit->distance = t;
                closest_hit->point = ray_point_at_parameter(ray, t);
                closest_hit->normal = vec_sub(closest_hit->point, shape->sphere.center);
                closest_hit->normal = vec_normalize(closest_hit->normal);
                closest_hit->material = &shape->material;
				has_hit = true;
            }
        }
		shapes = shapes->next;
	}
	return (has_hit);
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
			if (node->children[i] && traverse_octree(node->children[i], ray, closest_hit) == true)
				return (true); //early stop se trova un hit
			i++;
		}
	}
	return (false);
}

static t_color compute_color_at_intersection(const t_hit hit, const t_scene scene)
{
	(void)scene;
	
	if (hit.material)
		return (hit.material->color);
	
    return ((t_color){.r = 255, .g = 255, .b = 255});
}