#include "PulseManager.h"

#include "Utils.h"
#include <glib.h>
#include <memory>

PulseManager::PulseManager()
    : main_loop(pa_threaded_mainloop_new()), main_loop_api(pa_threaded_mainloop_get_api(main_loop)) {
    pa_threaded_mainloop_lock(main_loop);
    pa_threaded_mainloop_start(main_loop);

    context = pa_context_new(main_loop_api, "PulseaudioConnectionWatcher");

    pa_context_set_state_callback(context, &PulseManager::context_state_cb, this);
    pa_context_connect(context, nullptr, PA_CONTEXT_NOFAIL, nullptr);
    pa_threaded_mainloop_wait(main_loop);
    pa_threaded_mainloop_unlock(main_loop);

    if (context_ready) {
        subscribe_to_events();
    } else {
        util::error(log_tag + "context initialization failed");
    }
}

PulseManager::~PulseManager() {

    drain_context();

    pa_threaded_mainloop_lock(main_loop);

    util::debug(log_tag + "disconnecting Pulseaudio context...");
    pa_context_disconnect(context);

    util::debug(log_tag + "Pulseaudio context was disconnected");

    util::debug(log_tag + "unreferencing Pulseaudio context");
    pa_context_unref(context);

    pa_threaded_mainloop_unlock(main_loop);

    util::debug(log_tag + "stopping pulseaudio threaded main loop");
    pa_threaded_mainloop_stop(main_loop);

    util::debug(log_tag + "freeing Pulseaudio threaded main loop");
    pa_threaded_mainloop_free(main_loop);
}

void PulseManager::context_state_cb(pa_context* ctx, void* data) {
    auto pm = static_cast<PulseManager*>(data);

    auto state = pa_context_get_state(ctx);

    if (state == PA_CONTEXT_UNCONNECTED) {
        util::debug(pm->log_tag + "context is unconnected");
    } else if (state == PA_CONTEXT_CONNECTING) {
        util::debug(pm->log_tag + "context is connecting");
    } else if (state == PA_CONTEXT_AUTHORIZING) {
        util::debug(pm->log_tag + "context is authorizing");
    } else if (state == PA_CONTEXT_SETTING_NAME) {
        util::debug(pm->log_tag + "context is setting name");
    } else if (state == PA_CONTEXT_READY) {
        util::debug(pm->log_tag + "context is ready");
        util::debug(pm->log_tag + "connected to: " + pa_context_get_server(ctx));

        auto protocol = std::to_string(pa_context_get_protocol_version(ctx));
        util::debug(pm->log_tag + "protocol version: " + protocol);

        pm->context_ready = true;
        pa_threaded_mainloop_signal(pm->main_loop, 0);
    } else if (state == PA_CONTEXT_FAILED) {
        util::debug(pm->log_tag + "failed to connect context");

        pm->context_ready = false;
        pa_threaded_mainloop_signal(pm->main_loop, 0);
    } else if (state == PA_CONTEXT_TERMINATED) {
        util::debug(pm->log_tag + "context was terminated");

        pm->context_ready = false;
        pa_threaded_mainloop_signal(pm->main_loop, 0);
    }
}

void PulseManager::subscribe_to_events() {
    pa_context_set_subscribe_callback(
                context,
                [](auto c, auto t, auto idx, auto d) {
        auto f = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

        auto pm = static_cast<PulseManager*>(d);

        if (f == PA_SUBSCRIPTION_EVENT_SINK) {
            auto e = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

            if (e == PA_SUBSCRIPTION_EVENT_CHANGE) {
                pa_context_get_sink_info_by_index(
                            c, idx,
                            []([[maybe_unused]] auto cx, auto info,
                            [[maybe_unused]] auto eol, auto d) {
                    if (info != nullptr) {
                        auto pm = static_cast<PulseManager*>(d);
                        auto si = std::make_shared<mySinkInfo>();

                        si->name = info->name;
                        si->index = info->index;
                        si->description = info->description;
                        si->rate = info->sample_spec.rate;
                        si->format = pa_sample_format_to_string(info->sample_spec.format);

                        if (info->active_port != nullptr) {
                            si->active_port = info->active_port->name;
                        } else {
                            si->active_port = "null";
                        }

                        Glib::signal_idle().connect_once([pm, si = move(si)] { pm->sink_changed.emit(si); });
                    }
                },
                pm);
            }
        }
    },
    this);

    auto mask = static_cast<pa_subscription_mask_t>(PA_SUBSCRIPTION_MASK_SINK);

    pa_context_subscribe(
                context, mask,
                []([[maybe_unused]] auto c, auto success, auto d) {
        auto pm = static_cast<PulseManager*>(d);

        if (success == 0) {
            util::critical(pm->log_tag + "context event subscribe failed!");
        }
    },
    this);
}

void PulseManager::drain_context() {
    pa_threaded_mainloop_lock(main_loop);

    auto o = pa_context_drain(
                context,
                [](auto c, auto d) {
        auto pm = static_cast<PulseManager*>(d);

        if (pa_context_get_state(c) == PA_CONTEXT_READY) {
            pa_threaded_mainloop_signal(pm->main_loop, false);
        }
    },
    this);

    if (o != nullptr) {
        while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
            pa_threaded_mainloop_wait(main_loop);
        }

        pa_operation_unref(o);

        pa_threaded_mainloop_unlock(main_loop);

        util::debug(log_tag + "Context was drained");
    } else {
        pa_threaded_mainloop_unlock(main_loop);

        util::debug(log_tag + "Context did not need draining");
    }
}
