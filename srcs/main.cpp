/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jremy <jremy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 18:22:08 by deus              #+#    #+#             */
/*   Updated: 2022/11/07 11:22:49 by jremy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

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
