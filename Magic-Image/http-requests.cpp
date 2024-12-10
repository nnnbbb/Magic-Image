#include "http-requests.h"
#include "utils.h"


class CURLplusplus {
   private:
    CURL* curl;
    std::stringstream ss;
    long http_code;

   public:
    CURLplusplus()
        : curl(curl_easy_init()), http_code(0) {
    }
    ~CURLplusplus() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }
    std::string Get(const std::string& url) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

        ss.str("");
        http_code = 0;
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            throw std::runtime_error(curl_easy_strerror(res));
        }
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        return ss.str();
    }
    long GetHttpCode() {
        return http_code;
    }

   private:
    static size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp) {
        return static_cast<CURLplusplus*>(userp)->Write(buffer, size, nmemb);
    }
    size_t Write(void* buffer, size_t size, size_t nmemb) {
        ss.write((const char*)buffer, size * nmemb);
        return size * nmemb;
    }
};

String ParseJson(std::string& jsonString) {
    Json::CharReaderBuilder readerBuilder;
    Json::Value jsonData;
    std::string errs;

    std::istringstream s(jsonString);
    if (!Json::parseFromStream(readerBuilder, s, &jsonData, &errs)) {
        std::cerr << "解析 JSON 失败: " << errs << std::endl;
    }

    const Json::Value& dataArray = jsonData["data"]["entries"];
    std::string combinedText;

    for (const auto& item : dataArray) {
        if (item.isMember("explain")) {
            combinedText += item["explain"].asString() + "\r\n";
        }
    }
    String str = Utf8ToLocalCP(combinedText);
    return str;
}

String Post(String path, httplib::MultipartFormDataItems formData) {
    httplib::Client client("http://127.0.0.1:5100");
    client.set_connection_timeout(2);
    client.set_read_timeout(2);

    auto res = client.Post(path, formData);

    if (res) {
        if (res->status == 200) {
            std::string body = Utf8ToLocalCP(res->body);
            // std::cout << "Response body: " << body << std::endl;
            return body;
        } else {
            std::cout << "Request failed, status code: " << res->status << std::endl;
        }
    } else {
        auto err = res.error();
        std::cout << "Request failed, error code: " << err << std::endl;
    }

    return formData.at(0).content;
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

    return "";
}

String GetWordSuggest(String word) {
    String url = "http://dict.youdao.com/suggest?doctype=json";
    url = url + "&q=" + LocalCPToUtf8(word);
    CURLplusplus client;
    String x = client.Get(url);
    String explain = ParseJson(x);
    std::cout << "word: " << word << std::endl;
    std::cout << "Response body explain: " << explain << std::endl;

    return explain;
}