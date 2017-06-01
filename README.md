Software renderer for DOS
================
This is a small playground where I test various graphics features that can be done in software. It is also my attempt to provide some good, working DOS code to the public, since finding it on the Internet is getting more and more difficult due to the system's obscurity. DOS played a big part in the history of software development, so I want to preserve at least some bits and pieces of how things were done back in the day!

The project can be built on modern systems using the [Open Watcom](http://www.openwatcom.org/) compiler. This was chosen, since Watcom comes with easy to use memory extender for DOS.

Features
-------

- triangle rasterization
- quad rasterization (using triangle composition)
- affine and perspective corrected texture mapping
- perspective and view calculations - "DOF6 Camera Ready (tm)"
- line and point rendering
- loading, resizing and displaying bitmaps (8bpp) with optional color keying
- bitmap scrolling
- texture atlas support
- double buffering

The executable is a set of pre-made tests that demonstrate each feature in detail.

![Screenshot](http://kondrak.info/images/dos3d/1.png?raw=true)
![Screenshot](http://kondrak.info/images/dos3d/2.png?raw=true)
![Screenshot](http://kondrak.info/images/dos3d/3.png?raw=true)

TODO
-------

- subpixel and subtexel precision improvements
- Z-buffer
- add proper clipping
- ???
