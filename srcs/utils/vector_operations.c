/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector_operations.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/24 14:52:38 by craimond          #+#    #+#             */
/*   Updated: 2024/03/26 19:38:07 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minirt.h"

t_vector	vec_add(t_vector a, t_vector b) //somma di vettori
{
	t_vector	result;

	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return (result);
}

t_vector	vec_sub(t_vector a, t_vector b) //sottrazione di vettori
{
	t_vector	result;

	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return (result);
}

t_vector	vec_mul(t_vector a, t_vector b) //moltiplicazione di vettori
{
	t_vector	result;

	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return (result);
}

t_vector	vec_div(t_vector a, t_vector b) //divisione di vettori
{
	t_vector	result;

	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	return (result);
}

t_vector	vec_cross(t_vector a, t_vector b) //restituisce un vettore perpendicolare ad entrambi i vettori dati
{
	t_vector	result;

	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return (result);
}

t_vector	vec_normalize(t_vector a) //rende un vettore lungo 1
{
	float		len;
	t_vector	result;

	len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	result.x = a.x / len;
	result.y = a.y / len;
	result.z = a.z / len;
	return (result);
}

t_vector	vec_negate(t_vector a) //restituisce il vettore opposto
{
	t_vector	result;

	result.x = -a.x;
	result.y = -a.y;
	result.z = -a.z;
	return (result);
}

float	vec_dot(t_vector a, t_vector b) //prodotto scalare
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}
