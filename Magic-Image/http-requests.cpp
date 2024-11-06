#include "http-requests.h"
#include "utils.h"
// 将文本放入剪贴板
bool SetClipboardText(const std::string& text) {
    // 打开剪贴板
    if (!OpenClipboard(nullptr)) {
        std::cerr << "无法打开剪贴板" << std::endl;
        return false;
    }

    // 清空剪贴板
    EmptyClipboard();

    // 分配全局内存以存放文本
    HGLOBAL glob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (glob) {
        // 将文本复制到全局内存
        memcpy(GlobalLock(glob), text.c_str(), text.size() + 1);
        GlobalUnlock(glob);

        // 设置剪贴板数据
        SetClipboardData(CF_TEXT, glob);
    }

    CloseClipboard();
    return true;
}

String ParseJson(std::string& jsonString) {

    // 创建 JSON 对象
    Json::CharReaderBuilder readerBuilder;
    Json::Value jsonData;
    std::string errs;

    // 解析 JSON 数据
    std::istringstream s(jsonString);
    if (!Json::parseFromStream(readerBuilder, s, &jsonData, &errs)) {
        std::cerr << "解析 JSON 失败: " << errs << std::endl;
    }

    // 获取 data 数组
    const Json::Value& dataArray = jsonData["data"];
    std::string combinedText;

    // 遍历 data 数组并获取 text 字段
    for (const auto& item : dataArray) {
        if (item.isMember("text")) {
            combinedText += item["text"].asString() + "\r\n";  // 组合文本
        }
    }
    String str = Utf8ToLocalCP(combinedText);
    std::cout << "combinedText: " << str << std::endl;
    if (!combinedText.empty()) {
        SetClipboardText(str);
    }

    return str;
}

String Post(const std::string& filePath) {
    // 创建 HTTP 客户端
    httplib::Client cli("http://127.0.0.1:3001");  // 替换为你的服务器地址

    // 读取文件内容
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string fileContent = oss.str();

    // 创建 multipart/form-data 请求
    httplib::MultipartFormDataItems items = {
      {"img", fileContent, "image.jpg", "image/jpeg"}  // 文件字段
    };

    // 发送 POST 请求
    auto res = cli.Post("/", items);

    // 检查响应
    if (res) {
        return ParseJson(res->body);
    } else {
        std::cerr << "请求失败: " << res.error() << std::endl;
    }
    return String();
}