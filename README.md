# Game Overlay SDK
Library to write text messages on top of game window.

### Supported Graphical APIs:
* DirectX 11
* DirectX 12
* Vulkan

### Supported Architectures:
* x86
* x64

## Usage
**Run it with admin priviligies!**


To draw overlay for fullscreen game you have to hook inside game process before initializing of graphical API, so there are two options to achive it:

* Create suspended process, install hooks and resume threads
* Register a callback for CreateProcess Event, if target process was created - suspend all threads, install hooks and resume all threads( I use [WMI](https://docs.microsoft.com/en-us/windows/desktop/wmisdk/using-wmi) to achive it)

Both of these options are implemented, so there are:
```
run_process (exe_path, exe_args = "", steam_app_id = None)
start_monitor (process_name)
```
*Important note: for start_monitor you need to provide executable name exactly like in TaskManager and run target process by yourself while for run_process you need to provide full path to the executable and SDK will run it for you*

As soon as DLL was loaded inside game process you are able to call
```
send_message (message)
```
To send message and render it on top of game process window

For Inter Process Communication I use [Memory Mapped Files](https://docs.microsoft.com/en-us/windows/desktop/memory/creating-named-shared-memory), so send_message method just sends data to this Memory Mapped File and DLL which was injected to game process reads it.

*Important note: message will be showed as soon as game presents new frame and I don't ensure that all your messages will be displayed, for example if you send messages faster than game draws frames only the latest message will be displayed, also game process reads data one time per 200ms(but I use queue in python code to buffer messages, so real rate could be bigger)*

Also, there are several methods to control log level of Cpp code:
```
set_log_level (level) # level here is not related to python's log levels!
enable_monitor_logger ()
disble_monitor_logger ():
enable_dev_logger ()
```
I recommend to use *enable_monitor_logger* by default

And method which should be called in the end:
```
release_resources ()
```
And I've added python logging handler to write log messages on top of game window
```
overlay_log_handler = game_overlay_sdk.injector.OvelrayLogHandler ()
```

### Usage with Steam Games
Steam forks game process by default so there are two process and it's impossible to distinguish them and it breaks DLL injection, for *run_process* DLL will be loaded to the wrong process so no overlay will be drawed while for *start_monitor* it's almost random, it may hangs if wrong process was selected or may works well

Fortunately there is a way to force Steam to create only one process, we just need to create file *steam_appid.txt* in game folder, and it should be done only once per game, that's why there is steam_app_id argument in *run_process* method. You can get app_id [here](https://steamdb.info/search/) also make sure that Steam Client App is running


### Examples
#### How does it look?
![Demo](https://live.staticflickr.com/65535/47939286276_08fbb08c45_h.jpg)
#### How to achive it?
**Code:**
```
import argparse
import time
import game_overlay_sdk
import game_overlay_sdk.injector
import threading
import logging


logging.basicConfig (filename = 'test.log', level = logging.WARNING)

logger = logging.getLogger (__name__)
logger.setLevel (logging.INFO)
overlay_log_handler = game_overlay_sdk.injector.OvelrayLogHandler ()
formatter = logging.Formatter ('%(levelname)s:%(message)s')
overlay_log_handler.setFormatter (formatter)
logger.addHandler (overlay_log_handler)


class MessageThread (threading.Thread):

    def __init__ (self):
        super (MessageThread, self).__init__ ()
        self.need_quit = False

    def run (self):
        i = 0
        while not self.need_quit:
            logger.info ('Hi from python OverlayLogHandler %d' % i)
            i = i + 1
            time.sleep (1)


def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--exe_path', type = str, help = 'exe path', required = True)
    parser.add_argument ('--exe_args', type = str, help = 'exe args', default = '')
    parser.add_argument ('--steam_app_id', type = int, help = 'for steam games please provide app_id', required = False)
    args = parser.parse_args ()

    game_overlay_sdk.injector.enable_monitor_logger ()
    game_overlay_sdk.injector.run_process (args.exe_path, args.exe_args, args.steam_app_id)

    # start sending messages to overlay
    thread = MessageThread ()
    thread.start ()
    input ("Press Enter to stop...")
    thread.need_quit = True
    thread.join ()

    game_overlay_sdk.injector.release_resources ()


if __name__ == "__main__":
    main ()
```
**Command line(I use trial demo of Tomb Raider, for another Steam game you need to change steam_app_id):**
```
python examples\overlay_log_handler.py --exe_path "D:\Steam\steamapps\common\Shadow of the Tomb Raider Trial\SOTTR.exe" --steam_app_id 974630
```

#### Example with *start_monitor* instead *run_process*:
**Code:**
```
import argparse
import time
import game_overlay_sdk
import game_overlay_sdk.injector
import threading
import logging


class MessageThread (threading.Thread):

    def __init__ (self):
        super (MessageThread, self).__init__ ()
        self.need_quit = False

    def run (self):
        i = 0
        while not self.need_quit:
            try:
                game_overlay_sdk.injector.send_message ('Hi from python %d' % i)
                i = i + 1
                time.sleep (1)
            except game_overlay_sdk.injector.InjectionError as err:
                if err.exit_code == game_overlay_sdk.injector.CustomExitCodes.TARGET_PROCESS_IS_NOT_CREATED_ERROR.value:
                    logging.warning ('target process is not created')
                    time.sleep (5)
                elif err.exit_code == game_overlay_sdk.injector.CustomExitCodes.TARGET_PROCESS_WAS_TERMINATED_ERROR.value:
                    logging.warning ('target process was stopped')
                    # in monitor mode we can run process several times so dont need to stop this thread here
                    i = 0
                    time.sleep (5)
                else:
                    raise err


def main ():
    logging.basicConfig (level = logging.DEBUG)
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--name', type = str, help  = 'process name', required = True)
    args = parser.parse_args ()

    game_overlay_sdk.injector.enable_monitor_logger ()
    game_overlay_sdk.injector.start_monitor (args.name)

    # start sending messages to overlay
    thread = MessageThread ()
    thread.start ()
    input ("Press Enter to stop...")
    thread.need_quit = True
    thread.join ()

    game_overlay_sdk.injector.release_resources ()


if __name__ == "__main__":
    main ()
```
**Command Line:**
```
python examples\monitor.py --name SOTTR.exe
```
*Important note: with *start_monitor* you can run target process several times sequentially the only requirement here: dont run several processes at the same time*

#### More examples [here](./python/examples)

### Build Instructions
* Install [Vulkan SDK](https://vulkan.lunarg.com/) (I've used v1.1.106.0)
* Install cmake
* Install Visual Studio/MSBUILD
* run cmake_build.cmd or cmake_build32.cmd (if you have VS!=2017 you will need to change cmake generator)
