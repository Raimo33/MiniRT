/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector_operations.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:52:38 by craimond          #+#    #+#             */
/*   Updated: 2024/03/30 15:05:26 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

t_vector	vec_add(const t_vector a, const t_vector b) //somma di vettori
{
	t_vector	result;

	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return (result);
}

t_vector	vec_sub(const t_vector a, const t_vector b) //sottrazione di vettori
{
	t_vector	result;

	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return (result);
}

t_vector	vec_mul(const t_vector a, const t_vector b) //moltiplicazione di vettori
{
	t_vector	result;

	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return (result);
}

t_vector	vec_div(const t_vector a, const t_vector b) //divisione di vettori
{
	t_vector	result;

	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	return (result);
}

static inline bool is_vec_equal(const t_vector a, const t_vector b) //confronto di vettori
{
	return (a.x == b.x && a.y == b.y && a.z == b.z);
}

static void vec_disequate(const t_vector a, t_vector *to_disequate)
{
	if (a.x == to_disequate->x)
		to_disequate->x += 0.000001;
	if (a.y == to_disequate->y)
		to_disequate->y += 0.000001;
	if (a.z == to_disequate->z)
		to_disequate->z += 0.000001;
}

t_vector	vec_cross(const t_vector a, t_vector b) //restituisce un vettore perpendicolare ad entrambi i vettori dati
{
	t_vector	result;

	if (is_vec_equal(a, b))
		vec_disequate(a, &b);
		
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	
	return (result);
}

t_vector	vec_normalize(const t_vector a) //rende un vettore lungo 1
{
	float		len;
	t_vector	result;

	len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	result.x = a.x / len;
	result.y = a.y / len;
	result.z = a.z / len;
	return (result);
}

t_vector	vec_negate(const t_vector a) //restituisce il vettore opposto
{
	t_vector	result;

	result.x = -a.x;
	result.y = -a.y;
	result.z = -a.z;
	return (result);
}

t_vector 	vec_scale(const t_vector a, const float scale) //moltiplica un vettore per uno scalare
{
	t_vector	result;

	result.x = a.x * scale;
	result.y = a.y * scale;
	result.z = a.z * scale;
	return (result);
}

float	vec_dot(const t_vector a, const t_vector b) //prodotto scalare
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}
