import os
import ctypes
import numpy
from numpy.ctypeslib import ndpointer
import pkg_resources
import platform
import struct
import enum


class CustomExitCodes (enum.Enum):

    STATUS_OK = 0
    TARGET_PROCESS_IS_NOT_CREATED_ERROR = 1
    PROCESS_MONITOR_ALREADY_RUNNING_ERROR = 2
    PROCESS_MONITOR_IS_NOT_RUNNING_ERROR = 3
    GENERAL_ERROR = 4
    PATH_NOT_FOUND_ERROR = 5
    TARGET_PROCESS_WAS_TERMINATED_ERROR = 6


class InjectionError (Exception):
    def __init__ (self, message, exit_code):
        detailed_message = '%s:%d %s' % (CustomExitCodes (exit_code).name, exit_code, message)
        super (InjectionError, self).__init__ (detailed_message)
        self.exit_code = exit_code


class InjectorDLL (object):

    __instance = None

    @classmethod
    def get_instance (cls):
        if cls.__instance is None:
            if platform.system () != 'Windows':
                raise Exception ("For now only Windows is supported, detected platform is %s" % platform.system ())
            cls.__instance = cls ()
        return cls.__instance

    def __init__ (self):
        if struct.calcsize ("P") * 8 == 64:
            self.lib = ctypes.cdll.LoadLibrary (pkg_resources.resource_filename (__name__, os.path.join ('lib', 'DLLInjection64.dll')))
        else:
            self.lib = ctypes.cdll.LoadLibrary (pkg_resources.resource_filename (__name__, os.path.join ('lib', 'DLLInjection32.dll')))

        # start monitoring
        self.StartMonitor = self.lib.StartMonitor
        self.StartMonitor.restype = ctypes.c_int
        self.StartMonitor.argtypes = [
            ctypes.c_char_p,
            ctypes.c_char_p
        ]

        # stop monitorring
        self.StopMonitor = self.lib.StopMonitor
        self.StopMonitor.restype = ctypes.c_int
        self.StopMonitor.argtypes = []

        # set log level
        self.SetLogLevel = self.lib.SetLogLevel
        self.SetLogLevel.restype = ctypes.c_int
        self.SetLogLevel.argtypes = [
            ctypes.c_int
        ]

        # get pid
        self.GetPid = self.lib.GetPid
        self.GetPid.restype = ctypes.c_int
        self.GetPid.argtypes = [
            ndpointer (ctypes.c_int64)
        ]

        # send message
        self.SendMessageToOverlay = self.lib.SendMessageToOverlay
        self.SendMessageToOverlay.restype = ctypes.c_int
        self.SendMessageToOverlay.argtypes = [
            ctypes.c_char_p
        ]

        # run process
        self.RunProcess = self.lib.RunProcess
        self.RunProcess.restype = ctypes.c_int
        self.RunProcess.argtypes = [
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_char_p
        ]


def start_monitor (process_name):
    location = os.path.abspath (os.path.dirname (pkg_resources.resource_filename (__name__, os.path.join ('lib', 'GameOverlay64.dll'))))
    res = InjectorDLL.get_instance ().StartMonitor (process_name.encode (), location.encode ())
    if res != CustomExitCodes.STATUS_OK.value:
        raise InjectionError ('start process creation monitoring error please check logs', res)

def stop_monitor ():
    res = InjectorDLL.get_instance ().StopMonitor ()
    if res != CustomExitCodes.STATUS_OK.value:
        raise InjectionError ('stop monitoring error', res)

def set_log_level (level):
    res = InjectorDLL.get_instance ().SetLogLevel (level)
    if res != CustomExitCodes.STATUS_OK.value:
        raise InjectionError ('failed to set log level', res)

# log level for core module of this library, not related to python logging
def enable_monitor_logger ():
    set_log_level (2)

def disble_monitor_logger ():
    set_log_level (6)

def enable_dev_logger ():
    set_log_level (0)

def get_pid ():
    pid = numpy.zeros (1).astype (numpy.int64)
    res = InjectorDLL.get_instance ().GetPid (pid)
    if res != CustomExitCodes.STATUS_OK.value:
        raise InjectionError ('Callback has not been called yet')
    return pid[0]

def send_message (message):
    res = InjectorDLL.get_instance ().SendMessageToOverlay (message.encode ())
    if res != CustomExitCodes.STATUS_OK.value:
        raise InjectionError ('failed to send message', res)

def run_process (exe_path, exe_args = ""):
    location = os.path.abspath (os.path.dirname (pkg_resources.resource_filename (__name__, os.path.join ('lib', 'GameOverlay64.dll'))))
    res = InjectorDLL.get_instance ().RunProcess (exe_path.encode (), exe_args.encode(), location.encode ())
    if res != CustomExitCodes.STATUS_OK.value:
        raise InjectionError ('failed to run process please check logs', res)
