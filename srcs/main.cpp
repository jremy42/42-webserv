/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jremy <jremy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 18:22:08 by deus              #+#    #+#             */
/*   Updated: 2022/11/10 12:49:51 by fle-blay         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

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
