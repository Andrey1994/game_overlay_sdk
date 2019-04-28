import argparse
import time
import game_overlay_sdk
import game_overlay_sdk.injector

def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--name', type = str, help  = 'process name', required = True)
    args = parser.parse_args ()

    game_overlay_sdk.injector.set_log_level (0)
    game_overlay_sdk.injector.start_monitor (args.name)

    print ('start monitorring process creation, you need to run process manually now')

    time.sleep (600)
    game_overlay_sdk.injector.stop_monitor ()


if __name__ == "__main__":
    main ()