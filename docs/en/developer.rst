

.. note::

    To compile the code smoothly, do not compile with the source package downloaded directly from GitHub. The reason is that some files' line endings are replaced with Linux-style line endings when uploading to git, which will cause some of the directly downloaded code to fail to compile smoothly on Windows.

For the C++ API reference, generate the Doxygen documentation with the following command:

.. code-block:: shell

    xmake doxygen -F hikyuu_cpp/Doxygen

.. _developer:


Preparation Before Compiling
----------------------------

1. Install a C++ compiler
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The code is gradually migrating to c++20; compiling requires support for the c++20 language features.

- Windows platform: Visual C++ 2022
- Linux platform: g++ >= 13, clang >= 15


2. Install the build tool xmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

xmake >= 2.8.2, website: `<https://github.com/xmake-io/xmake>`_

See: `<https://xmake.io/#/zh-cn/guide/installation>`_


3. Clone the Hikyuu source code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Execute the following command to clone the hikyuu source code: (do not clone the code in a directory with Chinese characters in the path)

.. code-block:: shell

    git clone https://github.com/fasiondog/hikyuu.git

.. note::

    **Donating users who need to use the plugin, please install the hikyuu_plugin package: pip install hikyuu_plugin**

    If the latest code crashes when using the plugin, it is recommended to checkout the release branch or the corresponding version branch to compile.


4. Install the dependency packages on Linux
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On Linux, the dependent development packages need to be installed. For example on Ubuntu, execute the following command:

.. code-block:: shell
    
    sudo apt-get install -y libsqlite3-dev   


5. Install the Xcode command line tools on macOS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Please install xcode and its command line tools before compiling.
    

Compiling and Installing
------------------------

1. Install the python dependency packages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: shell

    pip install -r requirements.txt  or pip install -r requirements.txt -U  (upgrade the dependency packages periodically)


2. Compile
^^^^^^^^^^

.. note::

    **Note**: if it has not been compiled for a long time, before recompiling after updating the code, please execute python setup.py clear first to completely clear the previous compilation cache. And update the python dependencies, pip install -r requirements.txt


Enter the source code directory and execute python setup.py build -j 10; the other supported commands:

- python setup.py help        -- view the help
- python setup.py build       -- perform the compilation
- python setup.py install     -- compile and install (install to the site-packages directory of python)
- python setup.py uninstall   -- remove the installed Hikyuu
- python setup.py test        -- run the unit tests (with the optional argument --compile=1, compile first)
- python setup.py clear       -- clear the local compilation results
- python setup.py wheel       -- generate the wheel package


For the parameters of each command, you can execute python setup.py commond --help, e.g.: python setup.py build --help



3. Set the PYTHONPATH environment variable
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On Linux, e.g. modify the ~/.bashrc file and add the following at the end (pointing to the source code directory):

.. code-block:: shell

    export PYTHONPATH=/path/to/hikyuu:$PYTHONPATH


4. Convert to a Visual Studio project on Windows
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Please compile once directly with python setup.py build first, then convert.

On Windows, for those used to debugging with msvc, you can use the xmake project -k vsxmake -m "debug,release" command to generate the VS project. After executing the command, a subdirectory such as vsxmake2022 will be generated in the current directory, and the VS project is inside it.

In VS, you can set the demo as the startup project for debugging.


5. The IDE cannot provide hints properly
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Install pybind11-stubgen with the command pip install pybind11-stubgen
2. Run the pybind11-stubgen hikyuu -o . command, and then the hints and the help information will work properly.


6. Using the plugin
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For those who compile by themselves and want to use the hikyuu plugin, please install the standalone plugin package pip install hikyuu-plugin

But note that the plugin version needs to match; it is best to compile with the release branch (or tag) to avoid the version mismatch making it unusable.


Docker Build
------------

In the docker directory of the source code, the Dockerfile_dev files based on Ubuntu/Debain/Fedora are provided, which can be used to quickly build the Hikyuu compilation environment.

.. code-block:: shell

    cd docker
    docker build -t hikyuu_dev -f Dockerfile_dev .

    docker run -it hikyuu_dev /bin/bash

Enter the hikyuu directory; the rest is the same as the source code compilation steps.

You can also use the dockerfile that installs Hikyuu based on pip, see /docker/Dockerfile_miniconda .

Hikyuu needs to import the data before using it; the Docker image does not include the GUI, and you can directly execute the python hikyuu/gui/importdata.py command to import the data.

The hikyuu configuration file is in the /root/.hikyuu directory, and the data files (HDF5) are stored in the /root/stocks directory; you can specify the mounted directories yourself when creating the docker container.
