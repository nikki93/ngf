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

    materials = Blender.Material.Get()
    numMat = 0

    scenes = Blender.Scene.Get()

    for scn in scenes:
        out.write("ngflevel %s\n{\n" % (scn.getName()))

        numBrush = 0

        for object in scn.getChildren():
            object.select(False)

            objType = object.getProperty("type").getData()
            objName = object.getProperty("name").getData()
            objPos = object.getLocation()
            objRot = object.getMatrix().toQuat()

            out.write("\tobject\n\t{\n")
            out.write("\t\ttype %s\n" % (objType))
            out.write("\t\tname %s\n" % (objName))
            out.write("\t\tposition %f %f %f\n" % (objPos[0], objPos[2], -objPos[1]))
            out.write("\t\trotation %f %f %f %f\n" % (objRot[0], objRot[1], objRot[3], -objRot[2]))

            objProps = object.getAllProperties()
            numProps = 0
            propStr = "\n\t\tproperties\n\t\t{\n"

            for prop in objProps:
                if prop.getName() != "type" and prop.getName() != "name":
                    numProps = numProps + 1
                    key = prop.getName()
                    params = prop.getData()
                    data = ""

                    if prop.getName() == "isBrush":
                        nameToSet = scn.getName() + "_b" + str(numBrush)
                        object.setName(nameToSet)
                        object.getData(0,1).name = (nameToSet)

                        numMat = 0
                        materials = object.getData(0,1).materials

                        for mat in materials:
                            mat.setName(nameToSet + "_m" + str(numMat))
                            numMat += 1

                        numBrush += 1

                        key = "brushMeshFile"
                        params = object.getData(1) + ".mesh"
                        object.select(1)

                    if (prop.getType() == "STRING"):
                        #If string check for refText.
                        if params.startswith("refText"):
                            textName = params[8:]
                            text = Blender.Text.Get(textName)
                            indent = "\t\t\t" + (" " * len(key)) + " "
                            params = ": "
                            notFirst = False
                            for line in text.asLines():
                                if notFirst:
                                    params = params + "\n" + indent + ": "
                                params = params + line
                                notFirst = True
                        data = params
                    else:
                        #Otherwise convert whatever it is (float, bool, etc.) to a string.
                        data = str(params)

                    propStr = propStr + ("\t\t\t%s %s\n" % (key, data))

            propStr = propStr + "\t\t}\n"
            if numProps != 0:
            out.write(propStr)

	    out.write("\t}\n")

	out.write("}\n\n")
	
	if "NGFExport.cfg" in [text.getName() for text in Blender.Text.Get()]:
		if ((Blender.Text.Get("NGFExport.cfg").asLines()[0]) != filename):
			old = Blender.Text.Get("NGFExport.cfg")
			old.setName("NGFExport.cfg.old")
			Blender.Text.unlink(old)
	else:
		txt = Blender.Text.New("NGFExport.cfg")
		txt.write(filename)

if "NGFExport.cfg" in [text.getName() for text in Blender.Text.Get()]:
	write(Blender.Text.Get("NGFExport.cfg").asLines()[0])
else:
	Blender.Window.FileSelector(write, "Export")
