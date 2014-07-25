*das Körperrauschen*'s Arduino development files
================================================

Presentation
------------

This is the main folder of Arduino development for *das Körperrauschen* project.
Each folder in this folder contains a specific project used by *das Körperreauschen* project.

A project folder contains a `Readme.md` file explaining its purpose and two folders : `lib` and `src`.
These folders contains respectively (and obviously) the Arduino libraries used in that project and the source files of the project.

Building / Uploading
--------------------

Those projects have been setup to be build by `ino` command line tool.
To build it, go inside a folder and invoke `ino` to first build a binary for your board and then upload it. For example :

~~~~
cd capsense+fsr
ino build -m micro
ino upload -m micro
~~~~
