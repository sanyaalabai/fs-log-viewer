#ifndef FLV_LOG_H
#define FLV_LOG_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../engine/include/utils/log.hpp"
#include "../engine/include/utils/json.hpp"
#include "../engine/include/utils/utils.hpp"

size_t gMsgTypeCount[8]={0,0,0,0,0,0,0,0};

enum MessageType {
    MT_NONE=0,
    MT_STATE,
    MT_DEBUG,
    MT_INFO,
    MT_WARNING,
    MT_ERROR,
    MT_CRITICAL,
    MT_UNKNOWN,
    MT_ALL
};
MessageType getType(const std::string& tText) {
    auto c=tText[1];
    switch (c) {
    case 'S': gMsgTypeCount[1]+=1; return MT_STATE;
    case 'D': gMsgTypeCount[2]+=1; return MT_DEBUG;
    case 'I': gMsgTypeCount[3]+=1; return MT_INFO;
    case 'W': gMsgTypeCount[4]+=1; return MT_WARNING;
    case 'E': gMsgTypeCount[5]+=1; return MT_ERROR;
    case 'C': gMsgTypeCount[6]+=1; return MT_CRITICAL;
    default: gMsgTypeCount[7]+=1; return MT_UNKNOWN;
    }
}
struct LogMessage {
    std::string time;
    MessageType type;
    std::string typeBackup;
    std::string msg;
    bool showInMultitoggle=true;
};
struct MultitoggleFilters {
    bool stat=true;
    bool dbug=true;
    bool info=true;
    bool warn=true;
    bool errr=true;
    bool crit=true;
    bool unkn=true;
    bool ntag=true;
};
struct LogParser {
    std::string filePath="";
    std::vector<LogMessage> readValues{};
    MultitoggleFilters mtFilters{};

    void parse() {
        //Get file.
        readValues.clear();
        std::ifstream f(filePath);
        if(!f.is_open()) {
            LOG_ERRR("Failed to open file at: "+filePath);
            return;
        }
        std::string curStr="";
        //Get current line.
        while(std::getline(f,curStr)) {
            //Parse the line.
            LogMessage msg{};
            auto parsed=StrSplit(curStr,' ');
            msg.time=parsed[0];
            msg.typeBackup=parsed[1];
            //Validate message tag.
            if(parsed[1][0]=='[') {
                //Found a tag.
                msg.type=getType(parsed[1]);
                //Parse full message.
                msg.msg=parsed[2];
                for(size_t s=3;s<parsed.size();s++)
                    msg.msg+=" "+parsed[s];
            } else {
                //Didn't find a tag.
                msg.type=MT_NONE;
                gMsgTypeCount[0]+=1;
                //Parse full message.
                msg.msg=parsed[1];
                for(size_t s=2;s<parsed.size();s++)
                    msg.msg+=" "+parsed[s];
            }
            readValues.emplace_back(msg);
        }
    }
    void mtSort() {
        for(size_t m=0;m<readValues.size();m++) {
            auto type=readValues[m].type;
            switch (type) {
            case MT_NONE: readValues[m].showInMultitoggle=mtFilters.ntag; break;
            case MT_STATE: readValues[m].showInMultitoggle=mtFilters.stat; break;
            case MT_DEBUG: readValues[m].showInMultitoggle=mtFilters.dbug; break;
            case MT_INFO: readValues[m].showInMultitoggle=mtFilters.info; break;
            case MT_WARNING: readValues[m].showInMultitoggle=mtFilters.warn; break;
            case MT_ERROR: readValues[m].showInMultitoggle=mtFilters.errr; break;
            case MT_CRITICAL: readValues[m].showInMultitoggle=mtFilters.crit; break;
            case MT_UNKNOWN: readValues[m].showInMultitoggle=mtFilters.unkn; break;
            }
        }
    }
};


#endif // !FLV_LOG_H