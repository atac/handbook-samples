
import sys

kwargs = dict(
    name='Chapter 10 Programmer\'s Handbook Samples',
    author='Micah Ferrill',
    author_email='mcferrill@gmail.com',
    options={
        'build_exe': {
            'excludes': ['_hashlib', '_socket', '_ssl', 'bz2'],
            'includes': ['contextlib', 'Py106'],
            'optimize': 2,
        },
    })

try:
    from cx_Freeze import setup, Executable

    kwargs['executables'] = [
        Executable('stat.py', base='Console'),
        #Executable('c10_dump.py', base='Console'),
        Executable('copy.py', base='Console'),
        #Executable('video.py',
        #           base=sys.platform == 'win32' and 'Win32GUI' or None),
    ]

    #if sys.platform == 'win32':
    #    kwargs['options']['build_exe']['include_files'] = (
    #        ('../mplayer.exe', 'mplayer.exe'),
    #    )

except ImportError:
    from distutils.core import setup


setup(**kwargs)
