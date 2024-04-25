/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 13:58:08 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 17:21:36 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

void	setup_camera(t_camera *cam, const t_mlx_data *win_data)
{
	static const t_vector	world_up = {0, 1, 0};
	const double			rad_fov = cam->fov * M_PI / 180;
	const double			viewport_height = 2 * tan(rad_fov / 2);
	const double			viewport_width
		= win_data->aspect_ratio * viewport_height;

	cam->forward = vec_normalize(cam->normal);
	if (are_vectors_parallel(cam->forward, world_up))
		cam->right = (t_vector){1, 0, 0};
	else
		cam->right = vec_normalize(vec_cross(world_up, cam->forward));
	cam->up = vec_normalize(vec_cross(cam->forward, cam->right));
	cam->right_by_half_viewport_width
		= vec_scale(viewport_width / 2, cam->right);
	cam->up_by_half_viewport_height = vec_scale(viewport_height / 2, cam->up);
}
