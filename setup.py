
import os
import subprocess
import sysconfig
import shutil

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class QmakeExtension(Extension):
    def __init__(self, name):
        Extension.__init__(self, name, sources=[])


class QmakeBuild(build_ext):
    def run(self):
        print('===============================================================')

        qt_dir = os.getenv('QTDIR')
        if qt_dir is None:
            raise RuntimeError('QTDIR environment variable must be set')

        # sanity check, make sure we can execute qmake
        self.qmake_bin = os.path.join(qt_dir, 'bin', 'qmake')
        try:
            subprocess.check_output([self.qmake_bin, '--version'])
        except subprocess.SubprocessError:
            raise RuntimeError("Can't run qmake")

        for ext in self.extensions:
            self.build_extension(ext)

        print('===============================================================')

    def build_extension(self, ext):
        print(f'Building {ext.name} ...')
        print('  (see build.log for build details)')

        # environment variables used during the build
        build_env = os.environ.copy()
        build_env['PYTHON_INCLUDE'] = sysconfig.get_path('include')

        build_log = open('build.log', 'w')

        # make sure the build temp directory is created
        os.makedirs(self.build_temp, exist_ok=True)

        # invoke qmake
        project_file = os.path.join(darwin_root, 'darwin.pro')
        config = 'debug' if self.debug else 'release'
        subprocess.check_call(
            [self.qmake_bin, project_file, f'CONFIG+={config}'],
            cwd=self.build_temp,
            env=build_env,
            stdout=build_log)

        # invoke make (this will likely take a while)
        subprocess.check_call(
            ['make', f'-j{os.cpu_count()}'],
            cwd=self.build_temp,
            env=build_env,
            stdout=build_log)

        # make sure we built the extension binary
        ext_binary = os.path.join(
            self.build_temp, 'bindings', 'python', 'darwin.so')
        if not os.path.isfile(ext_binary):
            raise RuntimeError(f"'{ext_binary}' is missing, aborting.")

        # finally, copy the extension files
        ext_dst = self.get_ext_fullpath(ext.name)
        os.makedirs(os.path.dirname(ext_dst), exist_ok=True)
        shutil.copy(ext_binary, ext_dst)


# Pre-setup: if we're running setup.py from the darwin root location,
# create a sandbox subdirectory to contain all the fallout from setuptools
# (ex. darwin.egg-info, dist, ...)
#
# Other than avoiding the top level litter, this makes it easy to
# do a full cleanup by just deleting the .build directory
#
darwin_root = os.path.dirname(os.path.abspath(__file__))

if os.path.abspath(os.getcwd()) == darwin_root:
    # create the build directory for the binding extension
    build_root = os.path.abspath('.build')
    os.makedirs(build_root, exist_ok=True)
    os.chdir(build_root)
else:
    build_root = os.getcwd()


setup(
    name="darwin",
    version="1.0",
    author='Leonard Mosescu',
    description='Darwin Framework',
    long_description='A framework for Evolutionary Algorithms',
    url='https://github.com/tlemo/darwin',
    license='Apache License 2.0',
    ext_modules=[QmakeExtension(name='darwin')],
    cmdclass=dict(build_ext=QmakeBuild),
    test_suite="tests.bindings.python",
    zip_safe=False)
