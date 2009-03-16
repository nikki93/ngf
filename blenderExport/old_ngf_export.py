#!BPY

"""
Name: 'NGF Level (.ngf)'
Blender: 237
Group: 'Export'
Tooltip: 'Exports a level to NGF format'
"""

import Blender

def write(filename):
    out = file(filename, "w")
    scn= Blender.Scene.GetCurrent()
    for object in scn.getChildren():
        objType = object.getProperty("type").getData()
        objArgs = object.getProperty("args").getData()
        objPos = object.getLocation()
        objRot = object.getMatrix().toQuat()
        out.write("%s \t %s \t| %f %f %f \t| %f %f %f %f \n" % (objType, objArgs, -(objPos[0] * 10), objPos[2] * 10, objPos[1] * 10, objRot[0], objRot[1], objRot[3], objRot[2]))
    out.write("*-*-*");

Blender.Window.FileSelector(write, "Export")
