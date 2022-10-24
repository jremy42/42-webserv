/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jremy <jremy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 18:22:08 by deus              #+#    #+#             */
/*   Updated: 2022/10/24 16:29:43 by jremy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "Webserv.hpp"

int main (int ac, char** av)
{
	if (ac < 2)
		return (std::cerr << "need config" << std::endl , 1);
	Webserv myServ(av);

	myServ.parseRawConfig();
	myServ.createServerListFromRawConfig();
	myServ.execServerLoop();
}
