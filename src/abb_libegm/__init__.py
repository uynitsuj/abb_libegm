"""Python bindings for ABB LibEGM."""
import os
import sys

# Add the directory containing the module to Python's path
if os.path.dirname(__file__) not in sys.path:
    sys.path.insert(0, os.path.dirname(__file__))

from ._abb_libegm import *

# __all__ = [
#     "RobotAxes",
#     "EGMModes",
#     "BaseConfiguration",
#     "EGMControllerInterface",
#     "Constants",
# ]