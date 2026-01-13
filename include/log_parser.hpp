#ifndef FLV_LOG_H
#define FLV_LOG_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <firesteel/utils/log.hpp>
#include <firesteel/utils/json.hpp>
#include <firesteel/utils/utils.hpp>

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
struct EnviromentInfo {
    std::string fsVersion;
    std::string fsRenderer;
    CPUInfo cpu{};
    MotherboardInfo motherboard{};
    RAMInfo ram{};
    GPUInfo gpu{};
    OSInfo os{};
};
struct LogParser {
    std::string filePath="";
    std::vector<LogMessage> readValues{};
    MultitoggleFilters mtFilters{};
    EnviromentInfo env{};

    void clear() {
        env.fsVersion="";
        env.fsRenderer="";
        env.cpu.output="-";
        env.gpu.output="-";
        env.ram.output="-";
        env.motherboard.output="-";
        env.os.name="-";
        readValues.clear();
        filePath="";
    }

    void parse() {
        //Get file.
        readValues.clear();
        std::ifstream f(filePath);
        if(!f.is_open()) {
            LOG_ERRR("Failed to open file at: "+filePath);
            return;
        }
        std::string curStr="";
        bool hasHardwareInfo=true;
        bool parsingEnvInfo=false;
        bool isLogPreV021=false;
        bool hasValidCPU=true,hasValidGPU=true,hasValidRAM=true,hasValidMotherboard=true,hasValidOS=true;
        short parsedEnvLines=0;
        short invalidInfoOffset=0;
        //Get current line.
        while(std::getline(f,curStr)) {
            //Parse the line.
            LogMessage msg{};
            auto parsed=String::split(curStr,' ');
            msg.time=parsed[0];
            msg.typeBackup=parsed[1];
            //Validate message tag.
            if(parsed[1][0]=='[') {
                //Found a tag.
                msg.type=getType(parsed[1]);
                //Parse full message.
                msg.msg=parsed[2];
                //Get whole message.
                for(size_t s=3;s<parsed.size();s++)
                    msg.msg+=" "+parsed[s];
                //Check if log was made before model_loaders release.
                if(msg.type==MT_STATE && readValues.size()<10&&!isLogPreV021) isLogPreV021=(msg.typeBackup=="[STATE]");
                //Get harvestable info.
                if(msg.type==MT_INFO && readValues.size()<50) {
                    if(!parsingEnvInfo) {
                        parsingEnvInfo=
                            (msg.msg=="Found global Firesteel config. Retrieving...") ||
                            (isLogPreV021&&(msg.msg=="OpenGL context:"));
                        if(isLogPreV021) env.fsRenderer="OpenGL";
                        else if(parsed.size()>3) {LOG(parsed[3].c_str()); if(parsed[3]=="context:") env.fsRenderer=parsed[2];}
                        if(parsingEnvInfo) {
                            LOG("Found cfg line");
                            if(!isLogPreV021) parsedEnvLines+=3;
                        }
                    } else {
                        //Check if messages info can be useful.
                        parsedEnvLines+=1;
                        if(!hasHardwareInfo&&parsedEnvLines==4) {hasHardwareInfo=parsed[2]=="Hardware";LOG("Found hardware block");}
                        if(hasHardwareInfo&&msg.msg[0]==' ') {
                            // PARSE CPU //
                            if(parsedEnvLines==6) {hasValidCPU=!(
                                (parsed[0]=="WMI") ||
                                (parsed[0]=="Security") ||
                                (parsed[0]=="Failed")
                            );if(hasValidCPU){LOG("Found valid CPU info in log");env.cpu.output="";}else{invalidInfoOffset+=4;env.cpu.output="-";}}
                            if(hasValidCPU&&parsedEnvLines>5&&parsedEnvLines<10) {
                                curStr="";
                                for(size_t i=6;i<parsed.size();i++)
                                    curStr+=parsed[i];
                                switch (parsedEnvLines) {
                                case 6:env.cpu.vendor=curStr;break;
                                case 7:env.cpu.model=curStr;break;
                                case 8:env.cpu.cores=static_cast<unsigned int>(std::stoi(curStr));break;
                                case 9:env.cpu.frequency=static_cast<unsigned int>(std::stoi(curStr));break;
                                }
                            }
                            // PARSE GPU //
                            if(parsedEnvLines==(11-invalidInfoOffset)) {hasValidGPU=!(
                                (parsed[0]=="WMI")
                            );if(hasValidGPU){LOG("Found valid GPU info in log");env.gpu.output="";}else{invalidInfoOffset+=2;env.gpu.output="-";}}
                            if(hasValidGPU&&parsedEnvLines>(10-invalidInfoOffset)&&parsedEnvLines<(13-invalidInfoOffset)) {
                                curStr="";
                                for(size_t i=6;i<parsed.size();i++)
                                    curStr+=parsed[i];
                                if(parsedEnvLines==(11-invalidInfoOffset)) env.gpu.model=curStr;
                                else if(parsedEnvLines==(12-invalidInfoOffset)) env.gpu.memoryGB=std::stod(curStr);
                            }
                            // PARSE RAM //
                            if(parsedEnvLines==(14-invalidInfoOffset)) {hasValidRAM=!(
                                (parsed[0]=="Failed")
                            );if(hasValidRAM){LOG("Found valid RAM info in log");env.ram.output="";}else{invalidInfoOffset+=1;env.ram.output="-";}}
                            if(hasValidRAM&&parsedEnvLines==(14-invalidInfoOffset)) {
                                curStr="";
                                for(size_t i=6;i<parsed.size();i++)
                                    curStr+=parsed[i];
                                //"if" here is redundant
                                /*if(parsedEnvLines==(14-invalidInfoOffset))*/ env.ram.memoryGB=std::stod(curStr);
                            }
                            // PARSE MOTHERBOARD //
                            if(parsedEnvLines==(16-invalidInfoOffset)) {hasValidMotherboard=!(
                                (parsed[0]=="WMI") ||
                                (parsed[0]=="Failed")
                            );if(hasValidMotherboard){LOG("Found valid Motherboard info in log");env.motherboard.output="";}else{invalidInfoOffset+=2;env.motherboard.output="-";}}
                            if(hasValidMotherboard&&parsedEnvLines>(15-invalidInfoOffset)&&parsedEnvLines<(18-invalidInfoOffset)) {
                                curStr="";
                                for(size_t i=6;i<parsed.size();i++)
                                    curStr+=parsed[i];
                                if(parsedEnvLines==(16-invalidInfoOffset)) env.motherboard.model=curStr;
                                else if(parsedEnvLines==(17-invalidInfoOffset)) env.motherboard.vendor=curStr;
                            }
                            // PARSE OS //
                            if(parsedEnvLines==(19-invalidInfoOffset)) {hasValidOS=!(
                                (parsed[0]=="Failed")
                            );if(hasValidOS){LOG("Found valid OS info in log")}else{invalidInfoOffset+=4;env.os.name="-";}}
                            if(hasValidOS&&parsedEnvLines>(18-invalidInfoOffset)&&parsedEnvLines<=(22-invalidInfoOffset)) {
                                curStr="";
                                for(size_t i=6;i<parsed.size();i++)
                                    curStr+=parsed[i];
                                if(parsedEnvLines==(19-invalidInfoOffset)) env.os.name=curStr;
                                else if(parsedEnvLines==(20-invalidInfoOffset)) env.os.distroBuild=curStr;
                                else if(parsedEnvLines==(21-invalidInfoOffset)) env.os.version=curStr;
                                else if(parsedEnvLines==(22-invalidInfoOffset)) env.os.architecture=curStr;
                            }
                        }
                    }
                }
            } else {
                //Didn't find a tag.
                msg.type=MT_NONE;
                gMsgTypeCount[0]+=1;
                //Parse full message.
                msg.msg=parsed[1];
                if(parsed[1]=="Firesteel") env.fsVersion=parsed[2];
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