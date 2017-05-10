To install Otter Browser from source you need to have [Qt SDK](http://www.qt.io/download-open-source/) (5.3.0 or newer) installed. You can use system wide libraries when available, but you will need to install header files for modules specified in *CMakeLists.txt* (listed in line starting with `qt5_use_modules`) and sometimes additional header files for GStreamer and libxml2. Sometimes you may need to manually add QtConcurrent to list of required modules in CMakeLists.txt, by adding it to the line starting with `qt5_use_modules(otter-browser`, resulting in `qt5_use_modules(otter-browser Concurrent`.

CMake (3.0.0 or newer) is used as build system, you need to perform these steps to get clean build (last step is optional):

    mkdir build
    cd build
    cmake ../
    make
    make install

Alternatively you can use either Qt Creator IDE to compile sources or export native project files using CMake generators. You can also use CPack to create packages.

To create a portable version, create a file named *arguments.txt* with this line:
`--portable`. Place this file in a directory containing the main Otter executable file (the file with name starting with `otter-browser`).
