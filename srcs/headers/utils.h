/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 14:41:24 by craimond          #+#    #+#             */
/*   Updated: 2024/04/14 10:22:31 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
# define UTILS_H

# include <stdint.h>
# include <stdbool.h>

bool	is_comment(const char *line);
bool	ft_isspace(const char c);
bool 	is_empty_line(const char *line);
double	ft_atof(const char *str);
uint8_t	ft_atoui(const char *str);
char	*ft_strtok(char *const str, const char *const sep);
char	*get_next_line(int fd);
double	fclamp(const double value, const double min, const double max);
int32_t	clamp(const int32_t value, const int32_t min, const int32_t max);
void	ft_freematrix(void **matrix);

#endif