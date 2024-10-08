/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routines.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yioffe <yioffe@student.42lisboa.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 19:53:37 by yioffe            #+#    #+#             */
/*   Updated: 2024/09/17 16:21:43 by yioffe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/philo_bonus.h"

static int	death_check(t_philo *philo, t_config *config)
{
	long			time_since_last_eat;
	struct timeval	current_time;
	int				i;

	sem_wait(config->sem_killer);
	if (check_config_stop(config))
		return (EXIT_FAILURE);
	gettimeofday(&current_time, NULL);
	time_since_last_eat = time_diff_in_ms(philo->last_eat_time, current_time);
	i = 0;
	if (time_since_last_eat > config->time_to_die)
	{
		philo_print("died", philo);
		set_config_stop(config, true);
		while (i < config->number_of_philos)
		{
			sem_post(config->sem_fed_up);
			i ++;
		}
		sem_post(config->sem_killer);
		return (EXIT_FAILURE);
	}
	sem_post(config->sem_killer);
	return (EXIT_SUCCESS);
}

void	*monitor_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	while (1)
	{
		if (check_config_stop(philo->config))
			return (NULL);
		if (death_check(philo, philo->config) == EXIT_FAILURE)
		{
			kill_all_philos(philo->config);
			exit(EXIT_FAILURE);
		}
		usleep(MONITOR_FREQUENCY_US);
	}
}

int	start_monitor_thread(t_philo *philo)
{
	pthread_t	monitor_thread;

	if (pthread_create(&monitor_thread, NULL, monitor_routine,
			(void *)philo) != 0)
		return (EXIT_FAILURE);
	if (pthread_detach(monitor_thread) != 0)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

void	philosopher_routine(t_philo *philo)
{
	t_config	*config;

	start_monitor_thread(philo);
	config = philo->config;
	if (!config)
		return ;
	if (philo->id % 2)
		philo_sleep(1, config);
	while (!check_config_stop(config))
	{
		philo_take_forks_and_eat(philo);
		if (check_config_stop(config))
			break ;
		philo_print("is sleeping", philo);
		philo_sleep(config->time_to_sleep, config);
		if (check_config_stop(config))
			break ;
		philo_print("is thinking", philo);
	}
	exit(EXIT_SUCCESS);
}
