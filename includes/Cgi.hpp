#ifndef CGI_HPP
#define CGI_HPP

#include <string>

class Cgi{
private:
	std::string extractQuery(std::string &path, size_t pos_query);
	std::string extractPathInfo(std::string &path, size_t pos_query ,size_t pos_py);
	std::string extractScriptPath(std::string &path, size_t pos_query, size_t pos_py);
	std::string query_string;
	std::string path_info;
	std::string script_path;
public:
	void extractEnv(std::string &path,size_t pos_query ,size_t pos_py);
	std::string getQuery(void);
	std::string getPath(void);
	std::string getScript(void);

};


#endif