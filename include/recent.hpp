#ifndef FLV_RECENT_H
#define FLV_RECENT_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../engine/include/utils/utils.hpp"
#include "../engine/include/utils/log.hpp"

struct RecentFiles {
    std::vector<std::string> paths;
    void load() {
        //Get file.
        std::ifstream f("recent-files");
        if(!f.is_open()) {
            LOG_ERRR("Failed to open file at: recent-files");
            return;
        }
        std::string curStr="";
        //Get current line.
        while(std::getline(f,curStr))
            paths.emplace_back(String::split(curStr,'\n')[0]);
    }
    void check(const std::string& tPath, const int& tMaxRecent) {
        //Check if path is already in recent files.
        std::vector<std::string> files;
        bool wasAlreadyInRecent=false;
        for(size_t r=0;r<paths.size();r++) {
            LOG(paths[r]);
            if(paths[r]==tPath) {
                files.insert(files.begin(),tPath);
                LOG("Found path in recent");
                wasAlreadyInRecent=true;
            } else files.emplace_back(paths[r]);
        }
        paths=files;
        //If it wasn't - add it now.
        if(!wasAlreadyInRecent) {
            paths.insert(paths.begin(),tPath);
            if(paths.size()>tMaxRecent) paths.pop_back();
        }
        //Write them to file.
        std::string finalFile=paths[0];
        for(size_t r=1;r<paths.size();r++) finalFile+="\n"+paths[r];
        String::toFile("recent-files",finalFile);
    }
};


#endif // !FLV_RECENT_H