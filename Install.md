Installation and usage
======================

Introduction
------------

This software uses qmake to build all its components. 
qmake is part of a Qt distribution. 

qmake reads project files, that contain the options and rules how to 
build a certain project. A project file ends with the suffix "*.pro". 
Files that end with the suffix "*.pri" are included by the project 
files and contain definitions, that are common for several project files.

The first step is to edit the *.pri / *.pro files to adjust 
them to your needs. Take care to select your deployment directories.
<Br><Br>


Building WebArnBrowser
----------------------

The software can be built both by command line and IDE (Qt Creator).

The configuration in WebArnBrowser.pro will give a starting point. 
It works well when using the same base directory for ArnLib as the application,
e.g. basedir/ArnLib and basedir/WebArnBrowser. 

WebArnBrowser is optimized and tested for _Wt_ library ver 3.2.3.
It's strongly recommended to use this version.

### A) Unix 

> qmake <Br>
> make <Br>

The _Wt_ html resources has to be available to WebArnBrowser.
It can be done with something similar to: 
> ln -s /usr/local/share/Wt/resources .

For getting all features like code and xhtml editing, the _Wt_ resources should be complemented
to contain the following:
> Wt/resources/tiny_mce <Br>
> Wt/resources/tiny_mce/plugins <Br>
> Wt/resources/code_mirror <Br>
> Wt/resources/code_mirror/mode <Br>

Tiny_mce and Code_mirror are available for free download from their web sites.

Regarding ArnLib, the easiest way is to let it be placed in a standard location 
for librarys and includes, e.g. /usr/lib and /usr/include/ArnLib.

When using a shared library it's path has to be known to 
the run-time linker of your operating system. On Linux systems read
"man ldconfig" ( or google for it ). Another option is to use
the LD_LIBRARY_PATH (on some systems LIBPATH is used instead, on MacOSX
it is called DYLD_LIBRARY_PATH) environment variable.


### B) Win32/MSVC 

Has not been tested yet ...

There are probably dependency problems with _Wt_ library for windows.

Check that your Qt version has been built with MSVC - not with MinGW !

Please read the qmake documentation how to convert 
your *.pro files into your development environment.

For example MSVC with nmake:
> qmake WebArnBrowser.pro <Br>
> nmake <Br>

Windows doesn't like mixing of debug and release binaries.

In windows it's possible to install the dll files together with the application binary,
as the application directory always is included in the search path for dll.


### C) Win32/MinGW 

Has not been tested yet ...

There are probably dependency problems with _Wt_ library for windows.

Using Qt Creator for windows, will give you the needed tools for building a Qt project.

Check that your Qt version has been built with MinGW - not with MSVC !

Start a Shell, where Qt4 is initialized. (e.g. with
"Programs->Qt by Trolltech ...->Qt 4.x.x Command Prompt" ).
Check if you can execute "make" or something like "mingw32-make".

> qmake WebArnBrowser.pro <Br>
> make <Br>

Windows doesn't like mixing of debug and release binaries.

In windows it's possible to install the dll files together with the application binary,
as the application directory always is included in the search path for dll.


### D) MacOSX

Has not been tested yet ...

There are probably dependency problems with _Wt_ library for MacOSX.

Well, the Mac is only another Unix system. So read the instructions in A).

In the recent Qt4 releases the default target of qmake is to generate
XCode project files instead of makefiles. So you might need to do the
following:
> qmake -spec macx-g++


### E) Qt Embedded

Has not been tested yet ...

To build should be as simple as for a regular Unix build.
<Br><Br>


Using WebArnBrowser
-------------------

The program has the following command line parameters:

> **--docroot** see _Wt_ documentation. <Br> 
> **--http-address** see _Wt_ documentation. <Br>
> **--http-port** see _Wt_ documentation. <Br>
> **arnHost=** is the ArnServer to be browsed. <Br>
> **arnBrowseName=** is the used name as label for the browsing. <Br>

To ensure proper layout from _Wt_, use: `LC_NUMERIC=C`

Example invocation:

    LC_NUMERIC=C ./WebArnBrowser --docroot . --http-address 0.0.0 --http-port 2012
    arnHost=localhost arnBrowseName=Test
 
WebArnBrowser is assumed to run in a non graphical environment.
<Br><Br>


    This Install.md file is based on documentation in the Qwt project.
