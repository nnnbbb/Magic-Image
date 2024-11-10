#include "http-requests.h"
#include "utils.h"


String ParseJson(std::string& jsonString) {
    Json::CharReaderBuilder readerBuilder;
    Json::Value jsonData;
    std::string errs;

    std::istringstream s(jsonString);
    if (!Json::parseFromStream(readerBuilder, s, &jsonData, &errs)) {
        std::cerr << "解析 JSON 失败: " << errs << std::endl;
    }

    const Json::Value& dataArray = jsonData["data"];
    std::string combinedText;

    for (const auto& item : dataArray) {
        if (item.isMember("text")) {
            combinedText += item["text"].asString();
        }
    }
    String str = Utf8ToLocalCP(combinedText);
    std::cout << "combinedText: " << str << std::endl;

    return str;
}

String HttpPost(
  const std::string& filePath,
  bool noCache
) {
    httplib::Client cli("http://127.0.0.1:5100");

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string fileContent = oss.str();

    httplib::MultipartFormDataItems items = {
      {"img", fileContent, "image.jpg", "image/jpeg"},
      {"no_cache", (noCache ? "True" : "False"), "", "application/json"}
    };

    auto res = cli.Post("/ocr", items);

    if (res) {
        if (res->status == 200) {
            // std::cout << "Response body: " << res->body << std::endl;
            return res->body;
        } else {
            std::cout << "Request failed, status code: " << res->status << std::endl;
        }
    } else {
        auto err = res.error();
        std::cout << "Request failed, error code: " << err << std::endl;
    }

    return String();
}


String HttpGet(const std::map<std::string, std::string>& params) {
    std::string url = "http://127.0.0.1:5100";
    httplib::Client client(url);

    client.set_connection_timeout(2);
    client.set_read_timeout(2);

    std::string path = "/?";
    for (const auto& param : params) {
        path += param.first + "=" + param.second + "&";
    }
    if (!params.empty()) {
        path.pop_back();
    }

    auto res = client.Get(path);

    if (res) {
        if (res->status == 200) {
            // std::cout << "Response body: " << res->body << std::endl;
            return res->body;
        } else {
            std::cout << "Request failed, status code: " << res->status << std::endl;
        }
    } else {
        auto err = res.error();
        std::cout << "Request failed, error code: " << err << std::endl;
    }

    return params.at("content");
}