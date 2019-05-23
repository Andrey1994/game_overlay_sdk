import argparse
import time
import game_overlay_sdk
import game_overlay_sdk.injector
import threading

class MessageThread (threading.Thread):

    def __init__ (self):
        super (MessageThread, self).__init__ ()
        self.need_quit = False

    def run (self):
        i = 0
        while not self.need_quit:
            try:
                game_overlay_sdk.injector.send_message (' Hi from python %d' % i)
                i = i + 1
                time.sleep (1)
            except game_overlay_sdk.injector.InjectionError as err:
                if err.exit_code == game_overlay_sdk.injector.CustomExitCodes.TARGET_PROCESS_IS_NOT_CREATED_ERROR.value:
                    print ('target process is not created')
                    time.sleep (5)
                else:
                    raise err

def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--exe_path', type = str, help = 'exe path', required = True)
    parser.add_argument ('--exe_args', type = str, help = 'exe args', default = "")
    args = parser.parse_args ()

    game_overlay_sdk.injector.set_log_level (0)
    game_overlay_sdk.injector.run_process (args.exe_path, args.exe_args)

    # start sending messages to overlay
    thread = MessageThread ()
    thread.start ()
    input ("Press Enter to stop...")
    thread.need_quit = True
    thread.join ()


if __name__ == "__main__":
    main ()