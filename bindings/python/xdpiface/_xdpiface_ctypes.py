################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################

from __future__ import print_function
import os, sys
from ctypes import *
from ctypes.util import find_library

# xdpiface
lib = None
# check to see if the shared object was embedded locally, attempt to load it
# if not, try to load it using the default system paths...
# we need to use os.chdir instead of trying to modify $LD_LIBRARY_PATH and reloading the interpreter
t = os.getcwd()
p = os.path.join(os.path.dirname(__file__), '..')  # find the path to our $project_ctypes.py
os.chdir(p)  # change directories briefly

try:
    from xdpiface import libxdpiface                # attempt to import the shared lib if it exists
    lib = CDLL(libxdpiface.__file__)         # if it exists try to load the shared lib
except ImportError:
    pass
finally:
    os.chdir(t)  # switch back to orig dir

if not lib:
    try:
        # If LD_LIBRARY_PATH or your OSs equivalent is set, this is the only way to
        # load the library.  If we use find_library below, we get the wrong result.
        if os.name == 'posix':
            if sys.platform == 'darwin':
                lib = cdll.LoadLibrary('libxdpiface.1.dylib')
            else:
                lib = cdll.LoadLibrary("libxdpiface.so.1")
        elif os.name == 'nt':
            lib = cdll.LoadLibrary('libxdpiface.dll')
    except OSError:
        libpath = find_library("xdpiface")
        if not libpath:
            raise ImportError("Unable to find libxdpiface")
        lib = cdll.LoadLibrary(libpath)

class xdp_iface_t(Structure):
    pass # Empty - only for type checking
xdp_iface_p = POINTER(xdp_iface_t)

class xdp_sock_t(Structure):
    pass # Empty - only for type checking
xdp_sock_p = POINTER(xdp_sock_t)


# xdp_iface
lib.xdp_iface_new.restype = xdp_iface_p
lib.xdp_iface_new.argtypes = [c_char_p]
lib.xdp_iface_destroy.restype = None
lib.xdp_iface_destroy.argtypes = [POINTER(xdp_iface_p)]
lib.xdp_iface_load_program.restype = c_int
lib.xdp_iface_load_program.argtypes = [xdp_iface_p, c_char_p]
lib.xdp_iface_unload_program.restype = None
lib.xdp_iface_unload_program.argtypes = [xdp_iface_p]
lib.xdp_iface_get_name.restype = c_char_p
lib.xdp_iface_get_name.argtypes = [xdp_iface_p]
lib.xdp_iface_get_prog.restype = c_void_p
lib.xdp_iface_get_prog.argtypes = [xdp_iface_p]
lib.xdp_iface_test.restype = None
lib.xdp_iface_test.argtypes = [c_bool]

class XdpIface(object):
    """
    The xdp_iface class!
    """

    DEFAULT = "lo" # The default network interface
    allow_destruct = False
    def __init__(self, *args):
        """
        Create a new xdp iface
        """
        if len(args) == 2 and type(args[0]) is c_void_p and isinstance(args[1], bool):
            self._as_parameter_ = cast(args[0], xdp_iface_p) # Conversion from raw type to binding
            self.allow_destruct = args[1] # This is a 'fresh' value, owned by us
        elif len(args) == 2 and type(args[0]) is xdp_iface_p and isinstance(args[1], bool):
            self._as_parameter_ = args[0] # Conversion from raw type to binding
            self.allow_destruct = args[1] # This is a 'fresh' value, owned by us
        else:
            assert(len(args) == 1)
            self._as_parameter_ = lib.xdp_iface_new(args[0]) # Creation of new raw type
            self.allow_destruct = True

    def __del__(self):
        """
        Destroy the xdp_iface.
        """
        if self.allow_destruct:
            lib.xdp_iface_destroy(byref(self._as_parameter_))

    def __eq__(self, other):
        if type(other) == type(self):
            return other.c_address() == self.c_address()
        elif type(other) == c_void_p:
            return other.value == self.c_address()

    def c_address(self):
        """
        Return the address of the object pointer in c.  Useful for comparison.
        """
        return addressof(self._as_parameter_.contents)

    def __bool__(self):
        "Determine whether the object is valid by converting to boolean" # Python 3
        return self._as_parameter_.__bool__()

    def __nonzero__(self):
        "Determine whether the object is valid by converting to boolean" # Python 2
        return self._as_parameter_.__nonzero__()

    def load_program(self, xdp_prog_path):
        """
        Load compiled XDP BPF object
        """
        return lib.xdp_iface_load_program(self._as_parameter_, xdp_prog_path)

    def unload_program(self):
        """
        Unload compiled XDP BPF object
        """
        return lib.xdp_iface_unload_program(self._as_parameter_)

    def get_name(self):
        """
        Retrive xdp interface name
        """
        return lib.xdp_iface_get_name(self._as_parameter_)

    def get_prog(self):
        """
        Retrive xdp program descriptor
        """
        return c_void_p(lib.xdp_iface_get_prog(self._as_parameter_))

    @staticmethod
    def test(verbose):
        """
        Self test of this class.
        """
        return lib.xdp_iface_test(verbose)


# xdp_sock
lib.xdp_sock_new.restype = xdp_sock_p
lib.xdp_sock_new.argtypes = [xdp_iface_p]
lib.xdp_sock_destroy.restype = None
lib.xdp_sock_destroy.argtypes = [POINTER(xdp_sock_p)]
lib.xdp_sock_test.restype = None
lib.xdp_sock_test.argtypes = [c_bool]

class XdpSock(object):
    """
    The xdp_sock class!
    """

    allow_destruct = False
    def __init__(self, *args):
        """
        Create a new xdp socket
        """
        if len(args) == 2 and type(args[0]) is c_void_p and isinstance(args[1], bool):
            self._as_parameter_ = cast(args[0], xdp_sock_p) # Conversion from raw type to binding
            self.allow_destruct = args[1] # This is a 'fresh' value, owned by us
        elif len(args) == 2 and type(args[0]) is xdp_sock_p and isinstance(args[1], bool):
            self._as_parameter_ = args[0] # Conversion from raw type to binding
            self.allow_destruct = args[1] # This is a 'fresh' value, owned by us
        else:
            assert(len(args) == 1)
            self._as_parameter_ = lib.xdp_sock_new(args[0]) # Creation of new raw type
            self.allow_destruct = True

    def __del__(self):
        """
        Destroy the xdp_sock.
        """
        if self.allow_destruct:
            lib.xdp_sock_destroy(byref(self._as_parameter_))

    def __eq__(self, other):
        if type(other) == type(self):
            return other.c_address() == self.c_address()
        elif type(other) == c_void_p:
            return other.value == self.c_address()

    def c_address(self):
        """
        Return the address of the object pointer in c.  Useful for comparison.
        """
        return addressof(self._as_parameter_.contents)

    def __bool__(self):
        "Determine whether the object is valid by converting to boolean" # Python 3
        return self._as_parameter_.__bool__()

    def __nonzero__(self):
        "Determine whether the object is valid by converting to boolean" # Python 2
        return self._as_parameter_.__nonzero__()

    @staticmethod
    def test(verbose):
        """
        Self test of this class.
        """
        return lib.xdp_sock_test(verbose)

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
