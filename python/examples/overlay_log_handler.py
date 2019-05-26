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
