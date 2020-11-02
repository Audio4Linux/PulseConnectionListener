#include <memory>

#include "PulseManager.h"
#include "Utils.h"

#include "cxxopts.hpp"

int main(int argc, char *argv[])
{
    cxxopts::Options options(argv[0], "Watch for pulseaudio sink changes");
    options.add_options()
      ("o,once", "Wait for one event and exit", cxxopts::value<bool>()->default_value("false"))
      ("r,run", "Execute shell command on sink update", cxxopts::value<std::string>())
      ("f,filter", "Filter by sink name", cxxopts::value<std::string>())
      ("c,contains", "Filter modifier: don't require exact match", cxxopts::value<bool>()->default_value("false"))
      ("s,silent", "Disable output", cxxopts::value<bool>()->default_value("false"))
      ("h,help", "Print this message");

    bool once = false;
    bool silent = false;
    std::string filter;
    std::string run;

    auto result = options.parse(argc, argv);
    if(result.count("once") > 0)
        once = result["once"].as<bool>();
    if(result.count("silent") > 0)
        silent = result["silent"].as<bool>();
    if(result.count("run") > 0)
        run = result["run"].as<std::string>();
    if(result.count("filter") > 0)
        filter = result["filter"].as<std::string>();

    if (result.count("help") > 0){
      std::cout << options.help() << std::endl;
      exit(0);
    }

    GMainLoop* loop = g_main_loop_new(NULL, FALSE);
    PulseManager* pm = new PulseManager();

    pm->sink_changed.connect([&](const std::shared_ptr<mySinkInfo>& sink_info) {
        util::debug("sink updated: " + sink_info->name);

        if (!filter.empty() && sink_info->name != filter)
            return;

        if(!silent)
            g_print("%s\n", sink_info->name.c_str());

        if(!run.empty())
            system(run.c_str());

        if(once)
            exit(0);
    });


    g_main_loop_run(loop);
}
