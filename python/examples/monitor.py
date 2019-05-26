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