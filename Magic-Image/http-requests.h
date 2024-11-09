#include <iostream>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <httplib.h>


std::string HttpPost(const std::string& file_path);

std::string HttpGet(const std::map<std::string, std::string>& params);