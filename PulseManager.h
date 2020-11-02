#ifndef PULSEMANAGER_H
#define PULSEMANAGER_H

#include <glib.h>
#include <glibmm.h>
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <memory>

struct mySinkInfo {
    std::string name;
    uint index;
    std::string description;
    uint owner_module;
    uint monitor_source;
    std::string monitor_source_name;
    uint rate;
    std::string format;
    std::string active_port;
};

class PulseManager {

public:
    PulseManager();
    PulseManager(const PulseManager&) = delete;
    auto operator=(const PulseManager&) -> PulseManager& = delete;
    PulseManager(const PulseManager&&) = delete;
    auto operator=(const PulseManager &&) -> PulseManager& = delete;
    ~PulseManager();

    pa_threaded_mainloop* main_loop = nullptr;

    sigc::signal<void, std::shared_ptr<mySinkInfo>> sink_changed;

private:
    std::string log_tag = "pulse_manager: ";
    bool context_ready = false;
    pa_mainloop_api* main_loop_api = nullptr;
    pa_context* context = nullptr;
    static void context_state_cb(pa_context* ctx, void* data);
    void subscribe_to_events();
    void drain_context();

};

#endif //PULSEMANAGER_H
