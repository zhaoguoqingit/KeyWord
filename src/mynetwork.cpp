#include "mycq.hpp"
#include "httplib.h"

#include <json/json.h>
#include <fstream>
#include <string>

#include "mynetwork.h"
using namespace std;

#define HOST "keyword.xiaoxiaoge.cn"
#define PORT 80
#define PATH_UPDATE_CHECK "/version"

void mynetwork::openUrl(std::string url) {
    string temp = "\"" + url;
    temp += "\"";

    ShellExecuteA(NULL, "open", "explorer.exe", temp.c_str(), NULL, SW_SHOW);
}

Update::updateType Update::check(int version_id, string &inf) {
    try {
        httplib::Client cli(HOST, PORT);
        auto res = cli.Get(PATH_UPDATE_CHECK);

        if (!res || res->status != 200) {
            throw exception("获取数据失败 网络异常");
        }
        string json = res->body;

        Json::Value root;
        Json::Reader reader;

        reader.parse(json, root);

        int getVersion_id = root["version_id"].asInt();
        if (getVersion_id > version_id) {
            newVersion = root["version"].asString();
            host = root["host"].asString();
            path = root["path"].asString();
            port = root["port"].asInt();
            appName = root["file_name"].asString();

            int must = root["must_version_id"].asInt();
            if (version_id > must) return updateType::mustUpdate;

            inf = "有新版本,更新内容:\r\n";
            inf += root["inf"].asString();
            return updateType::update;
        }
        inf = "版本已是最新";
        return updateType::no;

    } catch (exception &e) {
        inf = e.what();
        return updateType::error;
    } catch (...) {
        inf = "未知原因";
        return updateType::error;
    }
}

bool Update::getUpdate(string &inf) {
    if (host.empty() || path.empty()) {
        inf = u8"更新链接为空";
        return false;
    }

    try {
        httplib::Client client(host, port);

        auto ret = client.Get(path.c_str());
        if (!ret || ret->status != 200) {
            throw exception("插件下载失败 网络异常");
        }

        string appPath = cq::dir::app();
        ofstream file(appPath + appName, ios::out | ios::binary);
        file << ret->body;
        file.close();
        inf = "更新完成";
        return true;

    } catch (exception &e) {
        inf = e.what();
        return false;
    } catch (...) {
        inf = "未知原因";
        return false;
    }
}
