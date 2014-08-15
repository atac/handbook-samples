
Py106 Samples
=============

These samples use a python wrapper to a DLL of the `Irig 106`_ library from irig106.org.

Dependencies
------------

* Python_ 2.7
* docopt_
* PySide_ (for video sample)

Building and Running
--------------------

If the dependencies are installed you can run each sample from the commandline using python (eg: python stat.py) and commandline help will show you how to use each one.

To build .exe files you will also need the cx_Freeze_ library. Once installed you should be able to use the setup.py script to build like so: python setup.py build

.. _PyChapter10: https://bitbucket.org/mcferrill/pychapter10
.. _Python: http://python.org
.. _docopt: http://docopt.org
.. _cx_Freeze: http://cx-freeze.sourceforge.net/
.. _Irig 106: http://sourceforge.net/projects/irig106/
.. _PySide: http://qt-project.org/wiki/PySide