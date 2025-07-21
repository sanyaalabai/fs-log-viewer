#ifndef FLV_CONFIG_H
#define FLV_CONFIG_H

#include <iostream>
#include <filesystem>
#include "../engine/include/utils/log.hpp"
#include "../engine/include/utils/json.hpp"

struct Config {
    unsigned int width=800, height=600;
    bool preferencesOpen=false;
    bool messageViewerOpen=true;
    bool harvestedSystemInfoOpen=false;
    bool categorize=false;
    bool multitoggles=false;
    bool showRecent=true;
    int maxRecent=5;

    void load() {
        if(!std::filesystem::exists("viewer.config.json")) {
            LOG_ERRR("Failed to open file at: viewer.config");
            return;
        }
        auto cfg=StrFromFile("viewer.config.json");
        nlohmann::json json=nlohmann::json::parse(cfg);
        if(!json["window"].is_null()) {
        if(!json["window"]["width"].is_null()) width=json["window"]["width"];
        if(!json["window"]["height"].is_null()) height=json["window"]["height"];
    }
        if(!json["view"].is_null()) {
        if(!json["view"]["viewer"].is_null()) messageViewerOpen=json["view"]["viewer"];
        if(!json["view"]["systeminfo"].is_null()) harvestedSystemInfoOpen=json["view"]["systeminfo"];
    }
        if(!json["preferences"].is_null()) {
        if(!json["preferences"]["categorize"].is_null()) categorize=json["preferences"]["categorize"];
        if(!json["preferences"]["multitoggles"].is_null()) multitoggles=json["preferences"]["multitoggles"];
        if(!json["preferences"]["show_recent_files"].is_null()) showRecent=json["preferences"]["show_recent_files"];
        if(!json["preferences"]["max_recent_files"].is_null()) maxRecent=json["preferences"]["max_recent_files"];
    }
    }
    void save() {
        nlohmann::json json;
        json["view"]["viewer"]=messageViewerOpen;
        json["view"]["systeminfo"]=harvestedSystemInfoOpen;
        json["preferences"]["categorize"]=categorize;
        json["preferences"]["multitoggles"]=multitoggles;
        json["preferences"]["show_recent_files"]=showRecent;
        json["preferences"]["max_recent_files"]=maxRecent;
        std::ofstream o("viewer.config.json");
        o << std::setw(4) << json << std::endl;
    }
};


#endif // !FLV_CONFIG_H