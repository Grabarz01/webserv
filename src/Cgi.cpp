#include "Cgi.hpp"
#include <string>
#include <iostream>

std::string Cgi::extractQuery(std::string &path, size_t pos_query )
{
	std::string result;
	if(pos_query == std::string::npos)
		return result;
	result = "PATH_INFO=" + path.substr(pos_query + 1, std::string::npos);
	std::cout << result << std::endl;//test
	return result;
}
std::string Cgi::extractPathInfo(std::string &path, size_t pos_query, size_t pos_py)
{
	std::string result;
	if (pos_py == std::string::npos)
		return result;
	else if (pos_query != std::string::npos)
		result = path.substr(pos_py + 3, pos_query);
	else
		result = path.substr(pos_py + 3, std::string::npos);
	std::cout << result << std::endl;//test
	return result;
}

void Cgi::extractEnv(std::string &path, size_t pos_query ,size_t pos_py)
{
	this->path_info = extractQuery(path, pos_query);
	this->path_info = extractPathInfo(path, pos_query, pos_py);
}