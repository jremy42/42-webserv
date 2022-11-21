#include <iostream>

int main(int ac, char *av[], char *env[])
{
	std::cout << "Number of arg : " << ac << std::endl;
	std::cout << "----------------------------------------" << std::endl;
	for (int i = 0; av[i]; i++)
		std::cout << "arg[" << i << "] : [" << av[i] << "]" << std::endl;
	std::cout << "----------------------------------------" << std::endl;
	for (int i = 0; env[i]; i++)
		std::cout << "env[" << i << "] : [" << env[i] << "]" << std::endl;
	return (0);
}
