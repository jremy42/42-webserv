/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jremy <jremy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 18:22:08 by deus              #+#    #+#             */
/*   Updated: 2022/12/16 09:56:44 by fle-blay         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
# include <signal.h>
# include "_utils.hpp"
int	g_rv;

int main(int ac, char **av)
{
	Webserv myServ;
	g_rv = 1;
	signal(SIGINT, __signal);
	signal(SIGQUIT, __signal);
	srand(0);
	try
	{
		if (ac == 1)
			myServ = Webserv("./conf/default.conf");
		else
			myServ = Webserv(av[1]);
		myServ.parseRawConfig();
		myServ.createServerListByPortConfig();
		myServ.execServerLoop();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Unrecoverable error : " << e.what() << std::endl;
		return 1;
	}
	return (0);
}
