

.. note::

    For a successful build, do not compile from a source package downloaded directly from GitHub: when files are uploaded to git, some line endings are converted to Linux-style line endings, so parts of a directly downloaded source package may fail to compile on Windows.

To build the C++ API reference, generate the Doxygen documentation with the following command:

.. code-block:: shell

    xmake doxygen -F hikyuu_cpp/Doxygen

.. _developer:


Build Prerequisites
-------------------

1. Install a C++ compiler
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The codebase is gradually migrating to C++20, so a compiler that supports the C++20 language features is required.

- Windows: Visual C++ 2022
- Linux: g++ >= 13, clang >= 15


2. Install the xmake build tool
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

xmake >= 2.8.2. Website: `<https://github.com/xmake-io/xmake>`_

See: `<https://xmake.io/#/zh-cn/guide/installation>`_


3. Clone the Hikyuu source code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Run the following command to clone the Hikyuu source code (do not clone into a directory whose path contains Chinese characters):

.. code-block:: shell

    git clone https://github.com/fasiondog/hikyuu.git

.. note::

    **Donor users who need the plugin should install the hikyuu_plugin package: pip install hikyuu_plugin**

    If the plugin crashes when used with the latest code, check out the release branch or the corresponding version branch and build from that.


4. Install the dependency packages on Linux
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On Linux, the required development packages must be installed. On Ubuntu, for example, run:

.. code-block:: shell
    
    sudo apt-get install -y libsqlite3-dev   


5. Install the Xcode command-line tools on macOS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Install Xcode and its command-line tools before building.
    

Building and Installing
-----------------------

1. Install the Python dependencies
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: shell

    pip install -r requirements.txt  or pip install -r requirements.txt -U  (upgrade the dependencies periodically)


2. Build
^^^^^^^^^^

.. note::

    **Note**: if you have not built for a while, run python setup.py clear before rebuilding after updating the code, so that the previous build cache is fully cleared. Also update the Python dependencies: pip install -r requirements.txt


From the source directory, run python setup.py build -j 10. Other supported commands:

- python setup.py help        -- show the help
- python setup.py build       -- run the build
- python setup.py install     -- build and install (into Python's site-packages directory)
- python setup.py uninstall   -- remove the installed Hikyuu
- python setup.py test        -- run the unit tests (optionally pass --compile=1 to build first)
- python setup.py clear       -- clear the local build artifacts
- python setup.py wheel       -- generate a wheel package


For the options of each command, run python setup.py <command> --help, for example: python setup.py build --help



3. Set the PYTHONPATH environment variable
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On Linux, for example, add the following line to the end of ~/.bashrc (pointing to the source directory):

.. code-block:: shell

    export PYTHONPATH=/path/to/hikyuu:$PYTHONPATH


4. Generate a Visual Studio project on Windows
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Run a complete build with python setup.py build first, and generate the project only afterwards.

On Windows, if you prefer to debug with MSVC, run xmake project -k vsxmake -m "debug,release" to generate a Visual Studio project. After the command finishes, a subdirectory such as vsxmake2022 is created in the current directory, and the Visual Studio project is inside it.

In Visual Studio, you can set the demo as the startup project for debugging.


5. IDE code hints do not work
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Install pybind11-stubgen with pip install pybind11-stubgen
2. Run pybind11-stubgen hikyuu -o .; code hints and help information will then work correctly.


6. Using the plugin
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you build from source and want to use the Hikyuu plugin, install the standalone plugin package: pip install hikyuu-plugin

Note that the plugin version must match your build: it is best to build from the release branch (or a release tag), so that a version mismatch does not render the plugin unusable.


Docker Build
------------

The docker directory in the source tree contains Dockerfile_dev files based on Ubuntu, Debian and Fedora, which can be used to quickly set up a Hikyuu build environment.

.. code-block:: shell

    cd docker
    docker build -t hikyuu_dev -f Dockerfile_dev .

    docker run -it hikyuu_dev /bin/bash

Enter the hikyuu directory; the remaining steps are the same as the source build instructions above.

There is also a Dockerfile that installs Hikyuu via pip; see /docker/Dockerfile_miniconda .

Hikyuu requires data to be imported before use. The Docker image does not include the GUI; run python hikyuu/gui/importdata.py directly to import the data.

The Hikyuu configuration file is located in /root/.hikyuu, and the data files (HDF5) are stored in /root/stocks; you can specify your own mount directories when creating the Docker container.
