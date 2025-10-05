#include <../include/app.hpp>
#include "imgui.utils/include/base.hpp"
#include "include/cfg.hpp"
#include "include/log_parser.hpp"
#include "include/recent.hpp"

using namespace Firesteel;
/// Data types.
Config config{};
LogParser logParser{};
RecentFiles recent{};
MessageType showTypes=MT_ALL;
/// File operations.
std::filesystem::path appPath;
std::vector<std::string> openFileFilters{
    "Log files","*.log",
    "All Files","*"
};

void openLog(const std::string& tPath) {
    logParser.clear();
    logParser.filePath=tPath;
    //Reset current path.
    std::filesystem::current_path(appPath);
    recent.check(logParser.filePath, config.maxRecent);
    //Parse the log.
    logParser.parse();
}

class FsLogViewer : public Firesteel::App {
    virtual void onInitialize() override {
        appPath=std::filesystem::current_path();
        recent.load();
        logParser.clear();
        if(config.lightTheme) ImGui::StyleColorsLight();
    }
    virtual void onUpdate() override {
        if(Keyboard::keyDown(KeyCode::O)&&(Keyboard::getKey(KeyCode::LEFT_CONTROL)||Keyboard::getKey(KeyCode::RIGHT_CONTROL))) {
            auto paths = OS::fileDialog(false,false,"",&openFileFilters,"Open log");
            if(paths.size() > 0) openLog(paths[0]);
            else LOG_ERRR("Failed to request log file path");
        }
        ImGui::PopStyleVar(3);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiID dockspace_id = ImGui::GetID("Firesteel Log Viewer");
        ImGui::Begin("Firesteel Log Viewer", NULL, FSImGui::defaultDockspaceWindowFlags);
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), FSImGui::defaultDockspaceFlags);
        ImGui::PopStyleVar(1);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
        if(ImGui::BeginMenuBar()) {
            if(ImGui::BeginMenu("File")) {
                if(ImGui::MenuItem("Open (Ctrl+O)")) {
                    auto paths = OS::fileDialog(false,false,"",&openFileFilters,"Open log");
                    if(paths.size()>0) openLog(paths[0]);
                    else LOG_ERRR("Failed to request log file path");
                }
                if(config.showRecent) if(ImGui::BeginMenu("Recent")) {
                    for(size_t r=0;r<recent.paths.size();r++)
                        if(ImGui::MenuItem(recent.paths[r].c_str())) openLog(recent.paths[r]);
                    ImGui::EndMenu();
                }
                if(ImGui::MenuItem("Preferences")) config.preferencesOpen=true;
                ImGui::Separator();
                if(ImGui::MenuItem("Exit")) window.close();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::End();
        if(config.preferencesOpen) {
            ImGui::Begin("Preferences",&config.preferencesOpen);

            ImGui::Text("General");
            //ImGui::Checkbox("Categorize", &config.categorize);
            if(ImGui::Checkbox("Light theme", &config.lightTheme)) config.lightTheme?ImGui::StyleColorsLight():ImGui::StyleColorsDark();
            ImGui::Checkbox("Allow multiple filters", &config.multitoggles);

            ImGui::Text("Security");
            ImGui::Checkbox("Show recently open files", &config.showRecent);
            ImGui::BeginDisabled(!config.showRecent);
            ImGui::DragInt("Max recent files", &config.maxRecent, 1, 1, 32);
            ImGui::EndDisabled();

            if(ImGui::Button("NUT")) exit(-1);
            ImGui::End();
        }
        /* Toolbar */ {
            ImGui::PopStyleVar(1);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));
            ImGui::Begin("Toolbar");
            if(!config.multitoggles) {
                if(ImGui::Button("All")) showTypes=MT_ALL; ImGui::SameLine();
                if(ImGui::Button(("State "+std::to_string(gMsgTypeCount[1])).c_str())) showTypes=MT_STATE; ImGui::SameLine();
                if(ImGui::Button(("Debug "+std::to_string(gMsgTypeCount[2])).c_str())) showTypes=MT_DEBUG; ImGui::SameLine();
                if(ImGui::Button(("Info "+std::to_string(gMsgTypeCount[3])).c_str())) showTypes=MT_INFO; ImGui::SameLine();
                if(ImGui::Button(("Warning "+std::to_string(gMsgTypeCount[4])).c_str())) showTypes=MT_WARNING; ImGui::SameLine();
                if(ImGui::Button(("Error "+std::to_string(gMsgTypeCount[5])).c_str())) showTypes=MT_ERROR; ImGui::SameLine();
                if(ImGui::Button(("Critical "+std::to_string(gMsgTypeCount[6])).c_str())) showTypes=MT_CRITICAL; ImGui::SameLine();
                if(ImGui::Button(("Unknown "+std::to_string(gMsgTypeCount[7])).c_str())) showTypes=MT_UNKNOWN; ImGui::SameLine();
                if(ImGui::Button(("No Tag "+std::to_string(gMsgTypeCount[0])).c_str())) showTypes=MT_NONE;
            } else {
                if(ImGui::Button("All")) {
                    logParser.mtFilters.stat=logParser.mtFilters.dbug=logParser.mtFilters.info=logParser.mtFilters.warn=
                    logParser.mtFilters.errr=logParser.mtFilters.crit=logParser.mtFilters.unkn=logParser.mtFilters.ntag=true;
                    logParser.mtSort();
                } ImGui::SameLine();
                if(ImGui::Checkbox(("Stat "+std::to_string(gMsgTypeCount[1])).c_str(),&logParser.mtFilters.stat)) logParser.mtSort(); ImGui::SameLine();
                if(ImGui::Checkbox(("Dbug "+std::to_string(gMsgTypeCount[2])).c_str(),&logParser.mtFilters.dbug)) logParser.mtSort(); ImGui::SameLine();
                if(ImGui::Checkbox(("Info "+std::to_string(gMsgTypeCount[3])).c_str(),&logParser.mtFilters.info)) logParser.mtSort(); ImGui::SameLine();
                if(ImGui::Checkbox(("Warn "+std::to_string(gMsgTypeCount[4])).c_str(),&logParser.mtFilters.warn)) logParser.mtSort(); ImGui::SameLine();
                if(ImGui::Checkbox(("Errr "+std::to_string(gMsgTypeCount[5])).c_str(),&logParser.mtFilters.errr)) logParser.mtSort(); ImGui::SameLine();
                if(ImGui::Checkbox(("Crit "+std::to_string(gMsgTypeCount[6])).c_str(),&logParser.mtFilters.crit)) logParser.mtSort(); ImGui::SameLine();
                if(ImGui::Checkbox(("Unkn "+std::to_string(gMsgTypeCount[7])).c_str(),&logParser.mtFilters.unkn)) logParser.mtSort(); ImGui::SameLine();
                if(ImGui::Checkbox(("Ntag "+std::to_string(gMsgTypeCount[0])).c_str(),&logParser.mtFilters.ntag)) logParser.mtSort(); ImGui::SameLine();
                if(ImGui::Button("None")) {
                    logParser.mtFilters.stat=logParser.mtFilters.dbug=logParser.mtFilters.info=logParser.mtFilters.warn=
                    logParser.mtFilters.errr=logParser.mtFilters.crit=logParser.mtFilters.unkn=logParser.mtFilters.ntag=false;
                    logParser.mtSort();
                }
            }
            ImGui::End();
        }
        if(config.harvestedSystemInfoOpen) {
            ImGui::Begin("Harvested System Info", &config.harvestedSystemInfoOpen);
            ImGui::TextWrapped("This tab can show incomplete information if user\
 has disabled hardware enumeration in global Firesteel config file\
 or if systemspecs has failed to retrieve information.");
            if(!logParser.env.fsVersion.empty()) if(ImGui::CollapsingHeader("Firesteel Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text(("Version: "+logParser.env.fsVersion).c_str());
                ImGui::Text(("Renderer: "+logParser.env.fsRenderer).c_str());
            }
            if(logParser.env.os.name!="-") if(ImGui::CollapsingHeader("OS", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text(("Name: "+logParser.env.os.name).c_str());
                ImGui::Text(("Build/distro: "+logParser.env.os.distroBuild).c_str());
                ImGui::Text(("Version: "+logParser.env.os.version).c_str());
                ImGui::Text(("Architecture: "+logParser.env.os.architecture).c_str());
            }
            if(logParser.env.cpu.output!="-") if(ImGui::CollapsingHeader("CPU", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text(("Vendor: "+logParser.env.cpu.vendor).c_str());
                ImGui::Text(("Model: "+logParser.env.cpu.model).c_str());
                ImGui::Text("Cores: %i",logParser.env.cpu.cores);
                ImGui::Text("Frequency: %i",logParser.env.cpu.frequency);
            }
            if(logParser.env.gpu.output!="-") if(ImGui::CollapsingHeader("GPU", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text(("Model: "+logParser.env.gpu.model).c_str());
                ImGui::Text("Memory: %.2fGB",logParser.env.gpu.memoryGB);
            }
            if(logParser.env.ram.output!="-") if(ImGui::CollapsingHeader("RAM", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Memory: %.2fGB",logParser.env.ram.memoryGB);
            }
            if(logParser.env.motherboard.output!="-") if(ImGui::CollapsingHeader("Motherboard", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text(("Model: "+logParser.env.motherboard.model).c_str());
                ImGui::Text(("Vendor: "+logParser.env.motherboard.vendor).c_str());
            }
            ImGui::End();
        }
        if(config.messageViewerOpen) {
            ImGui::PopStyleVar(1);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Messages",&config.messageViewerOpen);
            for(size_t m=0;m<logParser.readValues.size();m++) {
                //If isn't correct type - don't even bother.
                if(!config.multitoggles) {
                    if(showTypes!=MT_ALL&&logParser.readValues[m].type!=showTypes) continue;
                } else if(!logParser.readValues[m].showInMultitoggle) continue;
                //Draw time mark.
                ImGui::TextColored(ImVec4(0.4f,0.4f,0.4f,1),logParser.readValues[m].time.c_str());
                ImGui::SameLine();
                //Message type.
                if(logParser.readValues[m].type!=MT_NONE) {
                    //Get message type.
                    ImVec4 color=ImVec4(1,1,1,1);
                    switch (logParser.readValues[m].type) {
                    case MT_STATE: color=ImVec4(0.1f,0.4f,0.9f,1); break;
                    case MT_DEBUG: color=ImVec4(0.1f,0.8f,0.2f,1); break;
                    case MT_INFO: config.lightTheme?color=ImVec4(0.25f,0.25f,0.25f,1):color=ImVec4(0.75f,0.75f,0.75f,1); break;
                    case MT_WARNING: color=ImVec4(0.6f,0.5f,0.1f,1); break;
                    case MT_ERROR: color=ImVec4(0.8f,0.25f,0.15f,1); break;
                    case MT_CRITICAL: color=ImVec4(0.75f,0.1f,0.75f,1); break;
                    case MT_UNKNOWN: color=ImVec4(0.4f,0,0.4f,1); break;
                    }
                    //Draw message type.
                    ImGui::TextColored(color,logParser.readValues[m].typeBackup.c_str());
                    ImGui::SameLine();
                }
                //Draw the message.
                if(ImGui::Selectable(logParser.readValues[m].msg.c_str())) OS::copyToClipboard(logParser.readValues[m].msg);
                ImGui::NewLine();
            }
            ImGui::End();
        }
    }
    virtual void onShutdown() override {
        logParser.clear();
        config.save();
    }
};

int main() {
    config.load();
    return FsLogViewer{}.start("Firesteel Log Viewer v.1.1",config.width,config.height);
}