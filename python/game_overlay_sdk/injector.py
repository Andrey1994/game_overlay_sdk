import os
import ctypes
import numpy
from numpy.ctypeslib import ndpointer
import pkg_resources
import platform
import struct


class InjectionError (Exception):
    pass


class InjectorDLL (object):

    __instance = None

    @classmethod
    def get_instance (cls):
        if cls.__instance is None:
            if platform.system () != 'Windows':
                raise Exception ("For now only Windows is supported, detected platform is %s" % platform.system ())
            if struct.calcsize ("P") * 8 != 64:
                raise Exception ("You need 64-bit python to use this library")
            cls.__instance = cls ()
        return cls.__instance

    def __init__ (self):

        self.lib = ctypes.cdll.LoadLibrary (pkg_resources.resource_filename (__name__, os.path.join ('lib', 'DLLInjector.dll')))

        # start monitoring
        self.StartMonitor = self.lib.StartMonitor
        self.StartMonitor.restype = ctypes.c_bool
        self.StartMonitor.argtypes = [
            ctypes.c_char_p,
            ctypes.c_char_p
        ]

        # stop monitorring
        self.StopMonitor = self.lib.StopMonitor
        self.StopMonitor.restype = ctypes.c_bool
        self.StopMonitor.argtypes = []

        # set log level
        self.SetLogLevel = self.lib.SetLogLevel
        self.SetLogLevel.restype = None
        self.SetLogLevel.argtypes = [
            ctypes.c_int
        ]

        # get pid
        self.GetPid = self.lib.GetPid
        self.GetPid.restype = ctypes.c_int
        self.GetPid.argtypes = []


def start_monitor (process_name):
    location = os.path.abspath (os.path.dirname (pkg_resources.resource_filename (__name__, os.path.join ('lib', 'GameOverlay64.dll'))))
    res = InjectorDLL.get_instance ().StartMonitor (process_name.encode (), location.encode ())
    if not res:
        raise InjectionError ('start process creation monitoring error please check logs')

def stop_monitor ():
    res = InjectorDLL.get_instance ().StopMonitor ()
    if not res:
        raise InjectionError ('stop monitoring error')

def set_log_level (level):
    InjectorDLL.get_instance ().SetLogLevel (level)

def get_pid ():
    res = InjectorDLL.get_instance ().GetPid ()
    if res == 0:
        raise InjectionError ('Callback has not been called yet')