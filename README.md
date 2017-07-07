Software renderer for DOS (mode 13h)
================
This is a triangle based renderer which utilizes the neat property of mode 13h allowing the programmer to access video memory in the same way one would write to an array of bytes. It is also an attempt to provide working DOS code to the public, since finding it on the Internet is getting increasingly more difficult due to the system's age. The goal of this project is to translate "modern day graphics" to a now forgotten platform. As such, there's little to no DOS-specific assembly utilized and the rendering code can be fairly quickly ported to any contemporary operating system.

The project can be built on modern systems using the [Open Watcom](http://www.openwatcom.org/) compiler. This was chosen, since Watcom comes with easy to use memory extender for DOS.

Features
-------

- triangle rasterization
- front/back face culling (CCW surfaces are considered "back")
- affine and perspective corrected texture mapping
- multiple render targets
- depth testing (using a 1/Z buffer)
- perspective and view calculations - "DOF6 Camera Ready (tm)"
- line and point rendering
- wireframe rendering
- loading, resizing, scrolling and displaying bitmaps (8bpp) with optional color keying
- texture atlas support
- double buffering

The executable is a set of pre-made tests that demonstrate each feature.

![Screenshot](http://kondrak.info/images/dos3d/1.png?raw=true)
![Screenshot](http://kondrak.info/images/dos3d/2.png?raw=true)
![Screenshot](http://kondrak.info/images/dos3d/3.png?raw=true)
![Screenshot](http://kondrak.info/images/dos3d/q_shambler.gif?raw=true)
![Screenshot](http://kondrak.info/images/dos3d/4.png?raw=true)
![Screenshot](http://kondrak.info/images/dos3d/5.png?raw=true)

TODO
-------

- switch from floats and doubles to fixed point for stable precision
- add proper polygon clipping
- ???
