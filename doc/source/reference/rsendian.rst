Endianness Handling
===================

All of libredstone's endian functions take the form rs_endian_type,
where type is the type of variable that the function converts. All
of these functions convert from big-endian (the format that
Minecraft uses everywhere) to native-endian, whatever that may be.

Calling these functions on a native-endian value has the effect of
switching back to big-endian.

Functions are provided for signed and unsigned 16, 32, and 64 bit
integers, as well as unsigned 24-bit integers (stored in a 32-bit
int). There are also functions to convert floats (32-bit) and
doubles (64-bit).

.. doxygenfile:: rsendian.h
