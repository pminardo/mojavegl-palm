# mojavegl-palm
MojaveGL 2D is a high-performance graphics library designed for Palm OS 5 and Tapwave devices. The built-in Palm OS graphics APIs have very limited features and slow performance as most Palm OS devices do not have GPUs. MojaveGL is a framework and set of C++ classes that let you take your Palm OS games and apps to the next level.

MojaveGL powers the advanced graphics features in the "Saguaro" project for Palm OS. For more information about the "Saguaro" project, please see: https://en.wikipedia.org/wiki/Saguaro_(Palm_OS)

The MojaveGL framework provides the following features:

- Surface-based API for graphics operations. Drawing is done using CMglSurface objects. A CMglSurfae may be stored in local memory or video memory depending on device capabilities.
- Supports alpha blended graphics with normal, additive, and subtractive blend modes
- Draws primitives including gradients, ellipses, anti-aliased lines, anti-aliased beziers, rectangles, polylines, and polygons (filled and unfilled)
- Blitter operations such as rotation and scale
- Highly optimized blitters that run inside an ARMlet (native ARM code). Depending on the surface buffer alignment, the blitters may operate in one of the following ways:
	1) Read and write 32-bit pixel data
    2) Read 32-bit pixel data and write 16-bit pixel data
    3) Read and write 16-bit pixel data
- Rendering of TrueType fonts. The CMglFont class provides a nice API for fonts, drawing, measuring, and formatting text. The font rendering is powered by a version of FreeType that I ported to Palm OS.
