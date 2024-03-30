/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersections.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/30 13:16:18 by craimond          #+#    #+#             */
/*   Updated: 2024/03/30 13:16:26 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

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
	const float		radius = cylinder.diameter / 2.0f;
	const t_vector	oc = vec_sub(ray.origin, cylinder.center);
	
	// Coefficients for the quadratic equation (Ax^2 + Bx + C = 0)
    const float A = vec_dot(ray.direction, ray.direction) - pow(vec_dot(ray.direction, cylinder.direction), 2);
    const float B = 2 * (vec_dot(ray.direction, oc) - (vec_dot(ray.direction, cylinder.direction) * vec_dot(oc, cylinder.direction)));
    const float C = vec_dot(oc, oc) - pow(vec_dot(oc, cylinder.direction), 2) - radius * radius;

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
		if (t[i] > 0)
		{
			const t_point	point = vec_add(ray.origin, vec_scale(ray.direction, t[i]));
			const t_vector	vec_from_center_to_point = vec_sub(point, cylinder.center);
			const float		projection_lenght = vec_dot(vec_from_center_to_point, cylinder.direction);

			if (fabs(projection_lenght) <= cylinder.height / 2.0f)
				if (valid_t < 0 || t[i] < valid_t)
					valid_t = t[i]; // Update with the smaller positive t	
		}
		i++;
	}
	return (valid_t);
}