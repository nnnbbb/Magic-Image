#include <iostream>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <httplib.h>

std::string Post(std::string path, httplib::MultipartFormDataItems formData);

std::string HttpGet(const std::map<std::string, std::string>& params);

std::string GetWordSuggest(std::string word);