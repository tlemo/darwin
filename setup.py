
import os
import subprocess
import sysconfig

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
        print('  (see .build/build.log for build details)')

        build_dir = os.path.abspath('.build')

        # environment variables used during the build
        build_env = os.environ.copy()
        build_env['PYTHON_INCLUDE'] = sysconfig.get_path('include')

        # create build directory
        if not os.path.exists(build_dir):
            os.makedirs(build_dir)

        build_log = open(os.path.join(build_dir, 'build.log'), 'w')

        # invoke qmake
        config = 'debug' if self.debug else 'release'
        subprocess.check_call(
            [self.qmake_bin, '../darwin.pro', f'CONFIG+={config}'],
            cwd=build_dir,
            env=build_env,
            stdout=build_log)

        # invoke make (this will likely take a while)
        subprocess.check_call(
            ['make', f'-j{os.cpu_count()}'],
            cwd=build_dir,
            env=build_env,
            stdout=build_log)


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
    options={'build': {'build_lib': '.build/bindings/python'}},
    zip_safe=False)
