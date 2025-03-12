#ifndef CGI_HPP
#define CGI_HPP

#include <map>
#include <string>
#include <vector>

class Cgi {
 private:
  std::string extractQuery(std::string& path, size_t pos_query);
  std::string extractPathInfo(std::string& path,
                              size_t pos_query,
                              size_t pos_py);
  std::string extractScriptPath(std::string& path,
                                size_t pos_query,
                                size_t pos_py);
  std::string query_string;
  std::string path_info;
  std::string script_path;
  char* cgi_envp[7];
  char* cgi_path;
  char* cgi_script_path;

 public:
  ~Cgi();
  Cgi();
  void runCgi();
  char* getScriptPath();
  void extractEnv(std::string& path, size_t pos_query, size_t pos_py);
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