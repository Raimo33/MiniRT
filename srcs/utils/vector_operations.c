/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector_operations.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:52:38 by craimond          #+#    #+#             */
/*   Updated: 2024/04/11 19:22:59 by craimond         ###   ########.fr       */
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

bool	are_vectors_parallel(t_vector v1, t_vector v2)
{
	double dot;
	
    v1 = vec_normalize(v1);
    v2 = vec_normalize(v2);
    dot = vec_dot(v1, v2);
    return (fabs(dot - 1.0) < EPSILON) || (fabs(dot + 1.0) < EPSILON);
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
	double		len;

	len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	return ((t_vector){a.x / len, a.y / len, a.z / len});
}

inline t_vector	vec_negate(const t_vector a) //restituisce il vettore opposto
{
	return ((t_vector){-a.x, -a.y, -a.z});
}

inline t_vector 	vec_scale(const double scale, const t_vector a) //moltiplica un vettore per uno scalare
{
	return ((t_vector){a.x * scale, a.y * scale, a.z * scale});
}

inline double	vec_dot(const t_vector a, const t_vector b) //prodotto scalare
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

inline double	vec_length(const t_vector a) //lunghezza di un vettore
{
	return (sqrt(a.x * a.x + a.y * a.y + a.z * a.z));
}
