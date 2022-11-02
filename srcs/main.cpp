/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jremy <jremy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 18:22:08 by deus              #+#    #+#             */
/*   Updated: 2022/10/26 18:50:19 by fle-blay         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "Webserv.hpp"

int main (int ac, char** av)
{
	if (ac == 1)
		return (std::cerr << "Webserv : Fatal Error : At least 1 config file is needed to start" << std::endl , 1);
	try {
		Webserv myServ(av);
		myServ.parseRawConfig();
		myServ.createServerListFromRawConfig();
		myServ.execServerLoop();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
