#include <../include/app.hpp>
using namespace Firesteel;

class FsLogViewer : public Firesteel::App {
    virtual void onInitialize() override {
        LOG_INFO("Hello World!");
    }
};

int main() {
    return FsLogViewer{}.start();
}