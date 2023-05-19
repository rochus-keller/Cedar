### Welcome to the Cedar/Mesa TiogaViewer

This is a little viewer application for the Tioga editor file format used by Xerox for the Cedar/Mesa project. I implemented it to study/analyze the source tree published by the Computer History Museum on May 10 2023 (see https://computerhistory.org/blog/a-backup-of-historical-proportions/).

I'm particularly interested in the 1993 Solaris port of Cedar/Mesa, which was deployed as a CD and can be downloaded from https://xeroxparcarchive.computerhistory.org/_cdcsl_93-16_/1/.index.html (many thanks to Paul McJones for the hint). Using wget with the -r option I was able to make a local copy of the whole CD file tree which took about 3.5 hours (31638 files, 18678 of which hidden, 235 MB visible files, 942 MB in total); the tree includes 3441 *.mesa (Cedar sources) files and 440 *.tioga files (documentation).

The CD also includes plugins for the Andrew toolkit and Emacs from which I was able to derive a decoder for the Tioga format used by most of the files (even the majority of Cedar source files are in Tioga format; only a few are ASCII). The file tree also includes documentation about Cedar, particularly an EBNF grammar, which I used to implement a lexer and syntax highlighter. A parser and cross-referencer are work in progress.

The path to the root of the source tree can be passed as a command line argument, or just open a directory using the CTRL+O shortcut from the GUI.

#### Screenshots

A documentation file:
![TiogaViewer Screenshot](http://software.rochus-keller.ch/tiogaviewer-screenshot-1.png)

A Cedar source file:
![TiogaViewer Screenshot](http://software.rochus-keller.ch/tiogaviewer-screenshot-2.png)

#### Status on May 19, 2023

I generated a parser using EbnfStudio and Coco/R with the existing grammar and started testing it. As it looks at the moment, the grammar still requires considerable effort until full compatibility with the existing code is achieved. 

However, based on the present findings, I am rather reluctant to put in further effort.

Having already inspected parts of the code with my tool (1993 Solaris version and Cedar 6.1) I'm very surprised that there don't seem to be any object oriented features in the given Cedar versions. I had a look at Lampson's Cedar language description some years ago and had the impression that it included OO support, but this has apparently not been implemented. I'm particularly interested in Cedar because Wirth created Oberon after his second stay at PARC where he was exposed to the Cedar language and environment. But Oberon and the Cedar language seem to have little in common. Instead the Cedar versions at hand seem just to be a moderate extension of Mesa with a rather peculiar and LL(1) unfriendly syntax. So at the moment I assume that Wirth - apart from certain similarities of the Cedar and Oberon system user interface - rather learned from Cedar how not to do it; Cedar as a language seemed to have played a minor role in the design of Oberon, which also limits my interest. I continue to look for confirmation of my findings.

#### Precompiled versions

No precompiled versions are available at this time.

#### How to build the TiogaViewer

The executable can be built on all common platforms using regular Qt 5.x or using [LeanQt](https://github.com/rochus-keller/LeanQt) with minimal dependencies.

The project includes the TiogaViewer.pro file which can be opened and built in Qt Creator or directly with qmake on the command line.

To build the Code Navigator using LeanQt and the BUSY build system (with no other dependencies than a C++98 compiler) instead, do the following:

1. Create a new directory; we call it the root directory here
1. Download https://github.com/rochus-keller/Cedar/archive/refs/heads/master.zip and unpack it to the root directory; rename the resulting directory to "Cedar".
1. Download https://github.com/rochus-keller/LeanQt/archive/refs/heads/master.zip and unpack it to the root directory; rename the resulting directory to "LeanQt".
1. Download https://github.com/rochus-keller/BUSY/archive/refs/heads/master.zip and unpack it to the root directory; rename the resulting directory to "build".
1. Open a command line in the build directory and type `cc *.c -O2 -lm -o lua` or `cl /O2 /MD /Fe:lua.exe *.c` depending on whether you are on a Unix or Windows machine; wait a few seconds until the Lua executable is built.
1. Now type `./lua build.lua ../Cedar` (or `lua build.lua ../Cedar` on Windows); wait until the TiogaViewer executable is built; you find it in the output subdirectory.

#### Support
If you need support or would like to post issues or feature requests please use the Github issue list at https://github.com/rochus-keller/Cedar/issues or send an email to the author.





