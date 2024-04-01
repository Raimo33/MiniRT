/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersections.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/30 13:16:18 by craimond          #+#    #+#             */
/*   Updated: 2024/04/01 17:10:25 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static float	intersect_cylinder_side(const t_ray ray, const t_cylinder *cylinder);
static float	intersect_cylinder_caps(const t_ray ray, const t_cylinder *cylinder);
static float	intersect_cylinder_cap(const t_ray ray, const t_cylinder *cylinder, const t_vector extreme_center);

float	intersect_ray_sphere(const t_ray ray, const t_shape *shape)
{
	const t_sphere	sphere = shape->sphere;
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

float	intersect_ray_plane(const t_ray ray, const t_shape *shape)
{
	const t_plane	plane = shape->plane;
	float	denom;
	float	t;

	denom = vec_dot(plane.normal, ray.direction);
	if (fabs(denom) > EPSILON)
	{
		t = vec_dot(vec_sub(plane.center, ray.origin), plane.normal) / denom;
		if (t >= 0)
			return (t);
	}
	return (-1);
}

float	intersect_ray_cylinder(const t_ray ray, const t_shape *shape)
{
	const float	t1 = intersect_cylinder_side(ray, &shape->cylinder);
	const float	t2 = intersect_cylinder_caps(ray, &shape->cylinder);

	if (t2 > 0 && (t1 < 0 || t2 < t1))
		return (t2);
	return (t1);
}

static float	intersect_cylinder_caps(const t_ray ray, const t_cylinder *cylinder)
{
    const float t1 = intersect_cylinder_cap(ray, cylinder, cylinder->top_cap_center);
    const float t2 = intersect_cylinder_cap(ray, cylinder, cylinder->bottom_cap_center);
	if (t1 > 0 && (t1 < t2 || t2 < 0))
		return (t1);
	return (t2);
}

static float	intersect_cylinder_cap(const t_ray ray, const t_cylinder *cylinder, const t_vector extreme_center)
{
    const float denom = vec_dot(ray.direction, cylinder->direction);
	float t = -1;

    if (fabs(denom) > EPSILON)
	{
        const float 	t_cap = vec_dot(vec_sub(extreme_center, ray.origin), cylinder->direction) / denom;
        const t_vector	p_cap = vec_add(ray.origin, vec_scale(ray.direction, t_cap));
        if (vec_dot(vec_sub(p_cap, extreme_center), vec_sub(p_cap, extreme_center)) <= cylinder->radius * cylinder->radius && t_cap > 0)
            t = t_cap;
    }
	return (t);
}

static float	intersect_cylinder_side(const t_ray ray, const t_cylinder *cylinder)
{
	const t_vector		oc = vec_sub(ray.origin, cylinder->center);

	// Coefficients for the quadratic equation (Ax^2 + Bx + C = 0)
    const float A = vec_dot(ray.direction, ray.direction) - pow(vec_dot(ray.direction, cylinder->direction), 2);
    const float B = 2 * (vec_dot(ray.direction, oc) - (vec_dot(ray.direction, cylinder->direction) * vec_dot(oc, cylinder->direction)));
    const float C = vec_dot(oc, oc) - pow(vec_dot(oc, cylinder->direction), 2) - cylinder->sqr_radius;

	const float discriminant = B * B - 4 * A * C;
	if (discriminant < 0)
		return (-1); // No intersection

	const float sqrt_discriminant = sqrt(discriminant);
	const float two_times_a = 2 * A;
	const float t[2] = {
		(-B - sqrt_discriminant) / two_times_a,
		(-B + sqrt_discriminant) / two_times_a
	};
	float	valid_t = -1;

	uint8_t i = 0;
	while (i < 2)
	{
		if (t[i] > 0)
		{
			const t_point	point = vec_add(ray.origin, vec_scale(ray.direction, t[i]));
			const t_vector	vec_from_center_to_point = vec_sub(point, cylinder->center);
			const float		projection_lenght = vec_dot(vec_from_center_to_point, cylinder->direction);

			if (fabs(projection_lenght) <= cylinder->height / 2.0f)
			{
				valid_t = t[i];
				break ;
			}
				//if (valid_t < 0 || t[i] < valid_t)
					//valid_t = t[i]; // Update with the smaller positive t	
		}
		i++;
	}
	return (valid_t);
}

bool	ray_intersects_aabb(t_ray ray, t_point bounding_box_max, t_point bounding_box_min)
{
    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;
	float ray_direction[3] = {ray.direction.x, ray.direction.y, ray.direction.z};
	float bb_max[3] = {bounding_box_max.x, bounding_box_max.y, bounding_box_max.z};
	float bb_min[3] = {bounding_box_min.x, bounding_box_min.y, bounding_box_min.z};
	float ray_origin[3] = {ray.origin.x, ray.origin.y, ray.origin.z};

    for (int i = 0; i < 3; i++)
	{
        float invD = 1.0f / ray_direction[i];
        float t0 = (bb_min[i] - ray_origin[i]) * invD;
        float t1 = (bb_max[i] - ray_origin[i]) * invD;
        if (invD < 0.0f)
		{
            float temp = t0;
            t0 = t1;
            t1 = temp;
        }
        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;
        if (tmax <= tmin)
            return (false);
    }
    return (true);
}