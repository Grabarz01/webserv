#ifndef CGI_HPP
#define CGI_HPP

#include <map>
#include <string>
#include <vector>

enum cgiExt{
	CGI_PHP,
	CGI_PYTHON
};

class Cgi {
 private:
  std::string extractQuery(std::string& path);
  std::string extractPathInfo(std::string& path, size_t size);
  std::string extractScriptPath(std::string& path, size_t size);
  std::string queryString;
  std::string pathInfo;
  std::string scriptPath;
  char* cgiEnvp[7];
  char* cgiPath;
  char* cgiScriptPath;
  size_t posPython;
  size_t posPhp;
  size_t posQuery;
  size_t posCgiExt;
  cgiExt cgiExtension;

 public:
  ~Cgi();
  Cgi();
  int checkCgi(std::string pathWithRoot,
               std::string cgiPathPython,
               std::string cgiPathPhp,
               std::vector<std::string> ext);
  void runCgi();
  char* getScriptPath();
  void extractEnvFromPath(std::string& path);
  std::string getQuery(void);
  std::string getPath(void);
  std::string getScript(void);
  char* getCgiPath(void);
  char** getCgiEnvp(void);
  void readResponse(std::string& responseContent,
                    unsigned int& responseStatus,
                    int* pipe_response);
  void setCgiPath(std::string cgi_path);
  void setCgiScriptPath();
  void setEnvp(std::string& method,
               std::map<std::string, std::string>& headers);
};

#endif