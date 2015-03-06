libredstone Tools
=================

This directory contains various useful tools built on top of libredstone
with which one may analyse and modify minecraft save files.

Please be aware that you are using these tools at your own risk - any damage
they may inadvertently cause are your own responsibility.

nbttool
-------

A tool to look at the NBT structure of minecraft save files.

### Usage
    
*   `nbttool <File>`

    Read NBT data from file and pretty print it to stdout

*   `nbttool <Regionfile> <X> <Z>`

    Read NBT data from chunk X, Z in specified region file and
    pretty print it to stdout

### Examples
    
    nbttool "~/.minecraft/saves/My World/level.dat"
    
    nbttool "~/.minecraft/saves/My World/regions/r.0.0.mca" 2 1

setspawn
--------

A small utility to change the saved spawn location of a world.

### Usage

*   `setspawn <level.dat> <X> <Y> <Z>`

    Set the spawn point in the world's level.dat file to the
    specified X, Y and Z coordinates.

### Examples

    setspawn "~/.minecraft/saves/My World/level.dat" 40 64 -200

mcrtool
-------

Lists chunks within a .mcr or .mca file with their size, timestamp and
compression method.

### Usage
    
*   `mcrtool <Regionfile>`

    List all chunks within the region file.

### Examples

    mcrtool ~/.minecraft/saves/foo/r.0.0.mca