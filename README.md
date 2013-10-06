SWF MP3 Looper
==============

Gaplessly loop MP3 sounds and music in an Adobe SWF container. This is particularly useful for anyone using FlashDevelop, but can even provide better results than Adobe Flash Professional!

The code is cross platform, however, I haven't put together projects or built it for OS X or Linux. Since FlashDevelop only runs on Windows, and that's how I do my Flash development, I haven't needed this tool on OS X or Linux. If you want to see this project running on OS X or Linux, just open an issue saying so and it'll motivate me to do it :)

Download
==============
A Windows build can be downloaded at: http://github.com/mjbshaw/SWF-MP3-Looper/raw/bin/bin/SWF_MP3_Looper.zip

Note that you will likely have to have the Microsoft Visual C++ 2010 Redistributable Package: http://www.microsoft.com/en-us/download/details.aspx?id=5555

If you have any issues or feedback, please open an issue!

Contributing
==============
I originally started this  as a little tool for my own Flash work and never expected people to contribute (or even find this project). However, I'd be excited if you did! Make a pull request and I'll try to merge all improvement as quickly as possible. If you want to know how to get this project building on your own system, read the next section.

Building from Source
==============
I recommend using the latest version of Qt Creator for this project. Here are all the dependencies that are needed (and where to get them):
 - Qt Creator and MingW 32-bit 4.8: Get the "[Qt 5.1.x for Windows 32-bit (MinGW 4.8)](http://qt-project.org/downloads)" download.
 - FFmpeg and Lame libraries: Get these pre-built static libraries from branch [ffmpeg](https://github.com/mjbshaw/SWF-MP3-Looper/tree/ffmpeg) (commit [8c60843db](https://github.com/mjbshaw/SWF-MP3-Looper/commit/88c60843dbe8a3c950e6d65468a245ff9160cc12))
 - Qt 5.1.0 32-bit static libraries: Get these pre-built static libraries from branch [qt-mingw48-32-static](https://github.com/mjbshaw/SWF-MP3-Looper/tree/qt-mingw48-32-static) (commit [694b3d5fa9](https://github.com/mjbshaw/SWF-MP3-Looper/commit/694b3d5fa931b9ffb027e2591be9e5e1895fb07f))

Place the FFmpeg libraries in a folder named `lib` and the include files in a folder named `include`. Place the static Qt libaries in a sane location (I put mine by the rest of my Qt library versions in `C:\Qt\5.1.0\mingw48_32_static`). Open Qt Creator and go into your Build & Run options and add a new Qt Version (it needs to be the static Qt 5.1.0 version I provide; give it a relevant name and select the correct `qmake.exe`). Technically, you could build your own non-static FFmpeg libraries and use a non-static version of Qt if you wanted (it's not too hard, but you're on your own for that). I use static builds because I just want a single executable to distribute.

From there, just build from Qt Creator. If you have any troubles or issues, please open a new issue so I can correct any mistakes I may have made or better explain these instructions.

License
==============
This project is released under the [GNU GPL version 2](http://www.gnu.org/licenses/gpl-2.0.html) or [version 3](http://www.gnu.org/licenses/gpl-3.0.html), whichever you prefer. Every file in this repository is covered by this license, with exception to everything under the `external` folder. [FFmpeg](http://ffmpeg.org/), which this project uses, [is released](http://ffmpeg.org/legal.html) under the [GNU LGPL version 2.1](http://www.gnu.org/licenses/lgpl-2.1.html), or if you prefer, the [GNU GPL version 2](http://www.gnu.org/licenses/gpl-2.0.html). This project also uses [LAME](http://lame.sourceforge.net/), which is released under the [GNU LGPL version 2.1](http://www.gnu.org/licenses/lgpl-2.1.html). This project also uses [Qt](http://qt-project.org/), which is released under the [GNU LGPL version 2.1](http://www.gnu.org/licenses/lgpl-2.1.html). Note that these licenses have no effect on the SWF files you produce with this program.
