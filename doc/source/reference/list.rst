List Manipulation
=================

A generic singly-linked list type.

This is a data type used internally by libredstone, but it is
exposed here as a convenience to C developers who do not (by
default) have such a data type.

List cells are meant to be used directly, as well as with the
functions outlined here; iteration is usually done by accessing the
data and next members directly.

Lists of zero length are everywhere represented by the NULL pointer.

.. autodoxygenindex:: list.h
   :source: libredstone
