import re
import sys
from setuptools import find_packages
from setuptools.command.build_ext import build_ext

# try:
from skbuild import setup
# except ImportError:
#     print(
#         "Please update pip, you need pip 24 or greater,\n"
#         " or you need to install the PEP 518 requirements in pyproject.toml yourself",
#         file=sys.stderr,
#     )
#     raise
# Add pybind11 if it is installed


class BuildProtobuf(build_ext):
    def run(self):
        from pathlib import Path
        import subprocess

        proto_dir = Path('proto')
        proto_files = [str(p) for p in proto_dir.glob('*.proto')]

        # Generate Protobuf C++ code
        subprocess.run(['protoc', '--cpp_out=.', '--proto_path=proto'] + proto_files, check=True)

        # Include the generated Protobuf files in the package
        self.distribution.package_data['abb_libegm'] = [
            'proto/*.pb.h',
            'proto/*.pb.cc',
        ]

        super().run()
        
cmake_args = []
try:
    from pybind11 import get_cmake_dir
except ImportError:
    pass
else:
    cmake_args.append("-Dpybind11_DIR:PATH=" + get_cmake_dir())
# Read version from package.xml
with open("package.xml", "r") as f:
    __version__ = re.search(
        "<version>([0-9]+.[0-9]+.[0-9]+)</version>", f.read()
    ).group(1)
setup(
    name="abb_libegm",
    version=__version__,
    description="ABB Externally Guided Motion (EGM) Python Bindings",
    author="Justin Yu",
    author_email="yujustin@berkeley.edu",
    packages=find_packages(where="python"),
    package_dir={"": "python"},
    cmake_install_dir="python/abb_libegm",
    cmake_args=cmake_args,
    cmdclass={
        'build_ext': BuildProtobuf,
    },
)