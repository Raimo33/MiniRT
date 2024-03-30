/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:18:00 by craimond          #+#    #+#             */
/*   Updated: 2024/03/30 11:10:52 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void		setup_camera(t_camera *cam);
static t_ray	get_ray(const t_camera *cam, const uint16_t x, const uint16_t y);
static t_color	trace_ray(const t_scene scene, const t_ray ray);
static float	intersect_ray_sphere(const t_ray ray, const t_sphere sphere);
static float	intersect_ray_plane(const t_ray ray, const t_plane plane);
static float	intersect_ray_cylinder(const t_ray ray, const t_cylinder cylinder);
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
			if (node->children[i] && traverse_octree(node->children[i], ray, closest_hit) == true)
				return (true); //early stop se trova un hit
			i++;
		}
	}
	return (false);
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
		else if (shape->type == PLANE)
		{
			t = intersect_ray_plane(ray, shape->plane);
			if (t > 0 && t < closest_hit->distance)
			{
				closest_hit->distance = t;
				closest_hit->point = ray_point_at_parameter(ray, t);
				closest_hit->normal = shape->plane.normal;
				closest_hit->material = &shape->material;
				has_hit = true;
			}
		}
		else if (shape->type == CYLINDER)
		{
			t = intersect_ray_cylinder(ray, shape->cylinder);
			if (t > 0 && t < closest_hit->distance)
			{
				closest_hit->distance = t;
				closest_hit->point = ray_point_at_parameter(ray, t);
				closest_hit->normal = vec_sub(closest_hit->point, shape->cylinder.center);
				closest_hit->normal = vec_normalize(closest_hit->normal);
				closest_hit->material = &shape->material;
				has_hit = true;
			}
		}
		shapes = shapes->next;
	}
	return (has_hit);
}

static float	intersect_ray_sphere(const t_ray ray, const t_sphere sphere)
{
    const t_vector	oc = vec_sub(ray.origin, sphere.center);
    const float		a = vec_dot(ray.direction, ray.direction);
    const float		b = 2.0 * vec_dot(oc, ray.direction);
    const float		c = vec_dot(oc, oc) - sphere.radius * sphere.radius;
    const float		discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
        return (-1); // No intersection
    else
	{
		const float sqrt_discriminant = sqrt(discriminant);
        const float t1 = (-b - sqrt_discriminant) / (2 * a);
        const float t2 = (-b + sqrt_discriminant) / (2 * a);
        
        // Both t1 and t2 are behind the ray
		//TODO t1 e t2 sono negativi per qualche motivo
        if (t1 < 0 && t2 < 0)
            return (-1);

        // Return the smallest positive t (closest intersection)
        if (t1 > 0 && t2 > 0)
            return (t1 < t2 ? t1 : t2); // Return the smaller of the two
        else if (t1 > 0)
            return (t1); // Only t1 is positive
        else if (t2 > 0)
            return (t2); // Only t2 is positive (t1 < 0)
        // This handles an edge case where both t1 and t2 are 0, which is very rare
        // and would mean the ray origin is on the sphere's surface.
        return (0);
    }
}

static float	intersect_ray_plane(const t_ray ray, const t_plane plane)
{
	float	denom;
	float	t;

	denom = vec_dot(plane.normal, ray.direction);
	if (fabs(denom) > 0.0001)
	{
		t = vec_dot(vec_sub(plane.center, ray.origin), plane.normal) / denom;
		if (t >= 0)
			return (t);
	}
	return (-1);
}

static float	intersect_ray_cylinder(const t_ray ray, const t_cylinder cylinder)
{
	const t_vector	oc = vec_sub(ray.origin, cylinder.center);
	const 
	
	// Coefficients for the quadratic equation (Ax^2 + Bx + C = 0)
    const float A = vec_dot(ray.direction, ray.direction) - pow(vec_dot(ray.direction, cylinder.direction), 2);
    const float B = 2 * (vec_dot(ray.direction, oc) - (vec_dot(ray.direction, cylinder.direction) * vec_dot(oc, cylinder.direction)));
    const float C = vec_dot(oc, oc) - pow(vec_dot(oc, cylinder.direction), 2) - cylinder.radius * cylinder.radius;

	const float discriminant = B * B - 4 * A * C;
	if (discriminant < 0)
		return (-1); // No intersection

	const float sqrt_discriminant = sqrt(discriminant);
	const float t[2] = {
		(-B - sqrt_discriminant) / (2 * A),
		(-B + sqrt_discriminant) / (2 * A)
	};
	float	valid_t = -1;

	uint8_t i = 0;
	while (i < 2)
	{
		if (t[i] <= 0)
			continue ;
		const t_point	point = vec_add(ray.origin, vec_mul(ray.direction, t[i]));
		const t_vector	vec_from_center_to_point = vec_sub(point, cylinder.center);
		const float		projection_lenght = vec_dot(vec_from_center_to_point, cylinder.direction);

		if (fabs(projection_lenght) <= cylinder.height / 2.0f)
			if (valid_t < 0 || t[i] < valid_t)
				valid_t = t[i]; // Update with the smaller positive t
		i++;
	}
	return (valid_t);
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