Software renderer for DOS (mode 13h)
================
This is a triangle based renderer which utilizes the neat property of mode 13h allowing the programmer to access video memory in the same way one would write to an array of bytes. It is also an attempt to provide working DOS code to the public, since finding it on the Internet is getting increasingly more difficult. The idea behind this project is to use as much "platform independent" code as possible on a now forgotten OS. As such, there's little to no DOS-specific assembly present and the rendering code can be fairly quickly ported to any contemporary platform.

The project can be built out of the box using the [Open Watcom](http://www.openwatcom.org/) compiler. This was chosen, since Watcom comes with easy to use memory extender for DOS.

Minimum specs: 75MHz CPU, 4MB RAM, MS-DOS 5.0 or higher

Recommended specs: 100MHz CPU, 8MB RAM, MS-DOS 5.0 or higher

Features
-------

- triangle rasterization
- front/back face culling (CCW surfaces are considered "back")
- affine and perspective corrected texture mapping
- multiple render targets
- depth testing (using a 1/Z buffer)
- projection and view calculations using quaternion and matrix ops - "DOF6 Camera Ready (tm)"
- line and point rendering
- wireframe rendering
- loading, resizing, scrolling and displaying bitmaps (8bpp) with optional color keying
- texture atlas support
- double buffering

The executable is a set of pre-made tests that demonstrate each feature.

![Screenshot](IMAGES/1.png?raw=true)
![Screenshot](IMAGES/2.png?raw=true)
![Screenshot](IMAGES/3.png?raw=true)
![Screenshot](IMAGES/q_shambler.gif?raw=true)
![Screenshot](IMAGES/4.png?raw=true)
![Screenshot](IMAGES/5.png?raw=true)

TODO
-------

- switch from floats and doubles to fixed point for stable precision
- add proper polygon clipping
- perspective texture mapping doesn't have to interpolate every pixel
- ???
