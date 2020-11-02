# PulseConnectionListener
Listen for sink changes on the default Pulseaudio server

## Build from sources

Clone repository:
```
git clone https://github.com/Audio4Linux/PulseConnectionListener
cd PulseConnectionListener
```

Install dependencies (glibmm-2.4, giomm-2.4 and libpulse):
```bash
# Arch Linux
sudo pacman -S glibmm pulseaudio
```

Compile project:
```
mkdir build && cd build
cmake ..
make
```

You should end up with a binary called `pa_conn_watcher`:
```
./pa_conn_watcher --help
```
```
Watch for pulseaudio sink changes
Usage:
  ./pa_conn_watcher [OPTION...]

  -o, --once        Wait for one event and exit
  -r, --run arg     Execute shell command on sink update
  -f, --filter arg  Filter by sink name
  -c, --contains    Filter modifier: don't require exact match
  -s, --silent      Disable output
  -h, --help        Print this message

```

## Usage

First you need to find the name of the sink you want to watch:
```
pactl list sinks | grep "Name:" -A1
```
I'll use my Bluetooth earbuds (`bluez_sink.80_7B_3E_21_79_EC.a2dp_sink`) for the following examples. (You can also leave this filter out if you want to receive events from all sinks.)

Launch the `pa_conn_watcher` executable with the corresponding parameters: 
```
./pa_conn_watcher --filter "bluez_sink.80_7B_3E_21_79_EC.a2dp_sink"
```

We can also run a shell command when an event has been received. For example, we can print something to stdout or run external scripts:
```
./pa_conn_watcher --run "echo 'Something happened!'" --silent --filter "bluez_sink.80_7B_3E_21_79_EC.a2dp_sink"

./pa_conn_watcher --run "viper restart" --filter "bluez_sink.80_7B_3E_21_79_EC.a2dp_sink"
```
