/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector_operations.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:52:38 by craimond          #+#    #+#             */
/*   Updated: 2024/04/01 17:00:29 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

inline t_vector	vec_add(const t_vector a, const t_vector b) //somma di vettori
{
	return ((t_vector){a.x + b.x, a.y + b.y, a.z + b.z});
}

inline t_vector	vec_sub(const t_vector a, const t_vector b) //sottrazione di vettori
{
	return ((t_vector){a.x - b.x, a.y - b.y, a.z - b.z});
}

inline t_vector	vec_mul(const t_vector a, const t_vector b) //moltiplicazione di vettori
{
	return ((t_vector){a.x * b.x, a.y * b.y, a.z * b.z});
}

inline t_vector	vec_div(const t_vector a, const t_vector b) //divisione di vettori
{
	return ((t_vector){a.x / b.x, a.y / b.y, a.z / b.z});
}

inline bool is_vec_equal(const t_vector a, const t_vector b) //confronto di vettori
{
	return (a.x == b.x && a.y == b.y && a.z == b.z);
}

inline t_vector	vec_cross(const t_vector a, t_vector b) //restituisce un vettore perpendicolare ad entrambi i vettori dati
{
	return ((t_vector){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	});
}

t_vector	vec_normalize(const t_vector a) //rende un vettore lungo 1
{
	float		len;

	len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	return ((t_vector){a.x / len, a.y / len, a.z / len});
}

inline t_vector	vec_negate(const t_vector a) //restituisce il vettore opposto
{
	return ((t_vector){-a.x, -a.y, -a.z});
}

inline t_vector 	vec_scale(const t_vector a, const float scale) //moltiplica un vettore per uno scalare
{
	return ((t_vector){a.x * scale, a.y * scale, a.z * scale});
}

inline float	vec_dot(const t_vector a, const t_vector b) //prodotto scalare
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}
