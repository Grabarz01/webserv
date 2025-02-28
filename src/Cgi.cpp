#include "Cgi.hpp"
#include <iostream>
#include <string>

std::string Cgi::extractQuery(std::string& path, size_t pos_query) {
  std::string result = "QUERY_STRING=";
  if (pos_query == std::string::npos)
    return result;
  result += path.substr(pos_query + 1, std::string::npos);
  return result;
}
std::string Cgi::extractPathInfo(std::string& path,
                                 size_t pos_query,
                                 size_t pos_py) {
  std::string result = "PATH_INFO=";
  if (pos_py == std::string::npos)
    return result;
  else if (pos_query != std::string::npos)
    result += path.substr(pos_py + 3, pos_query - pos_py - 3);
  else
    result += path.substr(pos_py + 3, std::string::npos);
  return result;
}
std::string Cgi::extractScriptPath(std::string& path,
                                   size_t pos_query,
                                   size_t pos_py) {
  std::string result;
  result = path.substr(0, pos_py + 3);
  return result;
}
std::string Cgi::getQuery() {
  return query_string;
}
std::string Cgi::getPath() {
  return path_info;
}

std::string Cgi::getScript(void) {
  return script_path;
}
void Cgi::extractEnv(std::string& path, size_t pos_query, size_t pos_py) {
  this->query_string = extractQuery(path, pos_query);
  this->path_info = extractPathInfo(path, pos_query, pos_py);
  this->script_path = extractScriptPath(path, pos_query, pos_py);
  std::cout << query_string << std::endl;  // test
  std::cout << path_info << std::endl;  // test
  std::cout << script_path << std::endl;  // test

}