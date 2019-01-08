
import os
import sys

kwargs = dict(
    name='Chapter 10 Programmer\'s Handbook Samples',
    author='Micah Ferrill',
    author_email='mcferrill@gmail.com',
    options={
        'build_exe': {
            'excludes': ['_hashlib', '_socket', '_ssl', 'bz2'],
            'optimize': 2,
        },
        'install_exe': {
            'install_dir': os.path.join(os.path.dirname(__file__), 'bin'),
        },
    },
    packages=[
        'mplayer_pyside',
    ])

try:
    from cx_Freeze import setup, Executable

    kwargs['executables'] = [
        Executable('c10_stat.py', base='Console'),
        Executable('c10_dump.py', base='Console'),
        Executable('c10_copy.py', base='Console'),
        Executable('video.py',
                   base=sys.platform == 'win32' and 'Win32GUI' or None),
    ]

    if sys.platform == 'win32':
        kwargs['options']['build_exe']['include_files'] = (
            ('../mplayer.exe', 'mplayer.exe'),
        )

except ImportError:
    from distutils.core import setup


setup(**kwargs)
