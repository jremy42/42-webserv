/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jremy <jremy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 18:22:08 by deus              #+#    #+#             */
/*   Updated: 2022/11/15 13:10:27 by jremy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
# include <signal.h>
# include "_utils.hpp"
int	g_rv;


/*
int main(int ac, char **av)
{
	Webserv myServ;

	try
	{
		if (ac == 1)
			myServ = Webserv("conf/default.config");
		else
			myServ = Webserv(av[1]);
		myServ.parseRawConfig();
		//myServ.createServerListFromRawConfig();
		myServ.createServerListByPortConfig();
		myServ.execServerLoop();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
*/

int main(int ac, char **av)
{
	Webserv myServ;
	g_rv = 1;
	signal(SIGINT, __signal);
	signal(SIGQUIT, __signal);
	if (ac == 1)
		myServ = Webserv("conf/default.config");
	else
		myServ = Webserv(av[1]);
	myServ.parseRawConfig();
		//myServ.createServerListFromRawConfig();
	myServ.createServerListByPortConfig();
	myServ.execServerLoop();
	return (0);
}
