/*
 * =====================================================================================
 *
 *       Filename:  NgfPython.cpp
 *
 *    Description:  NGF-Python implementation
 *
 *        Version:  1.0
 *        Created:  03/15/2009 12:32:42 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#include "ngfplugins/NgfPython.h"

#include <boost/python/stl_iterator.hpp>

template<> NGF::Python::PythonManager* Ogre::Singleton<NGF::Python::PythonManager>::ms_Singleton = 0;
NGF::Python::PythonManager::PrintFunc NGF::Python::PythonManager::mPrinter = 0;

namespace NGF { namespace Python {

    static void runPycFile(FILE *fp, const char *filename);

/*
 * =============================================================================================
 * The bindings
 * =============================================================================================
 */

    BOOST_PYTHON_MODULE(Ngf)
    {
            //Docstring settings.
            py::docstring_options doc_options(true, true, false);

	    //Bind some non-class stuff.		
	    py::def("createObject", &PythonManager::_createObject, "createObject(type,name,pos,rot,props)");
	    py::def("destroyObject", &PythonManager::_destroyObject, "destroyObject(obj)");
	    py::def("_print", &PythonManager::_print, "<internal stuff>");

	    PythonObjectConnectorPtr (*getObjectStr) (std::string) = &PythonManager::_getObject;
	    py::def("getObject", getObjectStr, "getObject(name)\n      or\ngetObject(id)");
	    PythonObjectConnectorPtr (*getObjectID) (int) = &PythonManager::_getObject;
	    py::def("getObject", getObjectID);

	    //Bind our NGF connector.
	    py::class_<PythonObjectConnector, PythonObjectConnectorPtr >("GameObjectConnector", py::no_init)
		    .def_readwrite("locals", &PythonObjectConnector::mLocals)
		    .def("method", &PythonObjectConnector::method, "<internal stuff>")
		    .def("getID", &PythonObjectConnector::getID, "getID()")
		    .def("getName", &PythonObjectConnector::getName, "getName()")
		    .def("addFlag", &PythonObjectConnector::addFlag, "addFlag(flag)")
		    .def("hasFlag", &PythonObjectConnector::hasFlag, "hasFlag(flag)")
		    .def("removeFlag", &PythonObjectConnector::removeFlag, "removeFlag(flag)")
		    .def("getFlags", &PythonObjectConnector::getFlags, "getFlags()")
		    .def("getProperty", &PythonObjectConnector::getProperty, "getProperty(name, index, default)")
		    ;

	    //--- Ogre bindings ----------------------------------------------------

	    //Ogre::Radian.
            py::class_<Ogre::Radian>("Radian", py::init<Ogre::Real>())
                    .def(py::init<Ogre::Degree>())
                    .def("valueDegrees", &Ogre::Radian::valueDegrees)
                    .def("valueRadians", &Ogre::Radian::valueRadians)

                    .def(+py::self)
                    .def(py::self + py::self)
                    .def(py::self + Ogre::Degree())
                    .def(py::self += py::self)
                    .def(py::self += Ogre::Degree())
                    .def(-py::self)
                    .def(py::self - py::self)
                    .def(py::self - Ogre::Degree())
                    .def(py::self -= py::self)
                    .def(py::self -= Ogre::Degree())

                    .def(py::self * Ogre::Real())
                    .def(py::self * py::self)
                    .def(py::self *= Ogre::Real())
                    .def(py::self / Ogre::Real())
                    .def(py::self /= Ogre::Real())

                    .def(py::self < py::self)
                    .def(py::self <= py::self)
                    .def(py::self == py::self)
                    .def(py::self != py::self)
                    .def(py::self >= py::self)
                    .def(py::self > py::self)

                    .enable_pickling()
            ;

	    //Ogre::Degree.
            py::class_<Ogre::Degree>("Degree", py::init<Ogre::Real>())
                    .def(py::init<Ogre::Radian>())
                    .def("valueDegrees", &Ogre::Degree::valueDegrees)
                    .def("valueRadians", &Ogre::Degree::valueRadians)

                    .def(+py::self)
                    .def(py::self + py::self)
                    .def(py::self + Ogre::Radian())
                    .def(py::self += py::self)
                    .def(py::self += Ogre::Radian())
                    .def(-py::self)
                    .def(py::self - py::self)
                    .def(py::self - Ogre::Radian())
                    .def(py::self -= py::self)
                    .def(py::self -= Ogre::Radian())

                    .def(py::self * Ogre::Real())
                    .def(py::self * py::self)
                    .def(py::self *= Ogre::Real())
                    .def(py::self / Ogre::Real())
                    .def(py::self /= Ogre::Real())

                    .def(py::self < py::self)
                    .def(py::self <= py::self)
                    .def(py::self == py::self)
                    .def(py::self != py::self)
                    .def(py::self >= py::self)
                    .def(py::self > py::self)

                    .enable_pickling()
            ;

	    //Ogre::Vector3.
            py::object vector3Class = py::class_<Ogre::Vector3>("Vector3", py::init<>())
		    .def(py::init<Ogre::Real,Ogre::Real,Ogre::Real>())
		    .def_readwrite("x", &Ogre::Vector3::x)
		    .def_readwrite("y", &Ogre::Vector3::y)
		    .def_readwrite("z", &Ogre::Vector3::z)

                    .def(py::self == py::self)
                    .def(py::self != py::self)
                    .def(py::self < py::self)
                    .def(py::self > py::self)

                    .def(py::self + py::self)
                    .def(py::self - py::self)
                    .def(py::self * Ogre::Real())
                    .def(py::self * py::self)
                    .def(py::self / Ogre::Real())
                    .def(py::self / py::self)
                    .def(+py::self)
                    .def(-py::self)

                    .def(Ogre::Real() * py::self)
                    .def(Ogre::Real() / py::self)
                    .def(py::self + Ogre::Real())
                    .def(Ogre::Real() + py::self)
                    .def(py::self - Ogre::Real())
                    .def(Ogre::Real() - py::self)

                    .def(py::self += py::self)
                    .def(py::self += Ogre::Real())
                    .def(py::self -= py::self)
                    .def(py::self -= Ogre::Real())
                    .def(py::self *= py::self)
                    .def(py::self *= Ogre::Real())
                    .def(py::self /= py::self)
                    .def(py::self /= Ogre::Real())

                    .def("length", &Ogre::Vector3::length)
                    .def("squaredLength", &Ogre::Vector3::squaredLength)
                    .def("distance", &Ogre::Vector3::distance)
                    .def("squaredDistance", &Ogre::Vector3::squaredDistance)
                    .def("dotProduct", &Ogre::Vector3::dotProduct)
                    .def("absDotProduct", &Ogre::Vector3::absDotProduct)
                    .def("normalise", &Ogre::Vector3::normalise)
                    .def("crossProduct", &Ogre::Vector3::crossProduct)
                    .def("midPoint", &Ogre::Vector3::midPoint)
                    .def("makeFloor", &Ogre::Vector3::makeFloor)
                    .def("makeCeil", &Ogre::Vector3::makeCeil)
                    .def("perpendicular", &Ogre::Vector3::perpendicular)
                    .def("randomDeviant", &Ogre::Vector3::randomDeviant)
                    .def("angleBetween", &Ogre::Vector3::angleBetween)
                    .def("getRotationTo", &Ogre::Vector3::getRotationTo)
                    .def("isZeroLength", &Ogre::Vector3::isZeroLength)
                    .def("normalisedCopy", &Ogre::Vector3::normalisedCopy)
                    .def("reflect", &Ogre::Vector3::reflect)
                    .def("positionEquals", &Ogre::Vector3::positionEquals)
                    .def("positionCloses", &Ogre::Vector3::positionCloses)
                    .def("directionEquals", &Ogre::Vector3::directionEquals)

		    .enable_pickling()
		    ;
	    
            vector3Class.attr("ZERO") = Ogre::Vector3::ZERO;
            vector3Class.attr("UNIT_X") = Ogre::Vector3::UNIT_X;
            vector3Class.attr("UNIT_Y") = Ogre::Vector3::UNIT_Y;
            vector3Class.attr("UNIT_Z") = Ogre::Vector3::UNIT_Z;
            vector3Class.attr("NEGATIVE_UNIT_X") = Ogre::Vector3::NEGATIVE_UNIT_X;
            vector3Class.attr("NEGATIVE_UNIT_Y") = Ogre::Vector3::NEGATIVE_UNIT_Y;
            vector3Class.attr("NEGATIVE_UNIT_Z") = Ogre::Vector3::NEGATIVE_UNIT_Z;
            vector3Class.attr("UNIT_SCALE") = Ogre::Vector3::UNIT_SCALE;

	    //Ogre::Quaternion.
            //TODO: Bind overloaded functions.
            py::object quaternionClass = py::class_<Ogre::Quaternion>("Quaternion", py::init<>())
		    .def(py::init<Ogre::Real,Ogre::Real,Ogre::Real,Ogre::Real>())
		    .def(py::init<Ogre::Radian&,Ogre::Vector3&>())
                    .def(py::init<Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&>())
                    .def(py::init<Ogre::Vector3*>())
		    .def_readwrite("w", &Ogre::Quaternion::w)
		    .def_readwrite("x", &Ogre::Quaternion::x)
		    .def_readwrite("y", &Ogre::Quaternion::y)
		    .def_readwrite("z", &Ogre::Quaternion::z)
                    .def("swap", &Ogre::Quaternion::swap)

                    .def("FromAngleAxis", &Ogre::Quaternion::FromAngleAxis)
                    .def("xAxis", &Ogre::Quaternion::xAxis)
                    .def("yAxis", &Ogre::Quaternion::yAxis)
                    .def("zAxis", &Ogre::Quaternion::zAxis)

                    .def(py::self + py::self)
                    .def(py::self - py::self)
                    .def(py::self * py::self)
                    .def(py::self * Ogre::Real())
                    .def(Ogre::Real() * py::self)
                    .def(-py::self)
                    .def(py::self == py::self)
                    .def(py::self != py::self)

                    .def("Dot", &Ogre::Quaternion::Dot)
                    .def("Norm", &Ogre::Quaternion::Norm)
                    .def("normalise", &Ogre::Quaternion::normalise)
                    .def("Inverse", &Ogre::Quaternion::Inverse)
                    .def("UnitInverse", &Ogre::Quaternion::UnitInverse)
                    .def("Exp", &Ogre::Quaternion::Exp)
                    .def("Log", &Ogre::Quaternion::Log)

                    .def(py::self * Ogre::Vector3())

                    .def("getRoll", &Ogre::Quaternion::getRoll)
                    .def("getPitch", &Ogre::Quaternion::getPitch)
                    .def("getYaw", &Ogre::Quaternion::getYaw)
                    .def("Slerp", &Ogre::Quaternion::Slerp)
                    .def("SlerpExtraSpins", &Ogre::Quaternion::SlerpExtraSpins)
                    .def("Intermediate", &Ogre::Quaternion::Intermediate)
                    .def("Squad", &Ogre::Quaternion::Squad)
                    .def("nlerp", &Ogre::Quaternion::nlerp)

		    .enable_pickling()
		    ;

            quaternionClass.attr("ZERO") = Ogre::Quaternion::ZERO;
            quaternionClass.attr("IDENTITY") = Ogre::Quaternion::IDENTITY;
            quaternionClass.attr("ms_fEpsilon") = Ogre::Quaternion::ms_fEpsilon;

            //Ogre::ColourValue
            py::object colourValueClass = py::class_<Ogre::ColourValue>("ColourValue", py::init<>())
                .def(py::init<float, float, float>())
                .def(py::init<float, float, float, float>())

                .def("getAsRGBA", &Ogre::ColourValue::getAsRGBA)
                .def("getAsRGBA", &Ogre::ColourValue::getAsRGBA)
                .def("getAsARGB", &Ogre::ColourValue::getAsARGB)
                .def("getAsBGRA", &Ogre::ColourValue::getAsBGRA)
                .def("getAsABGR", &Ogre::ColourValue::getAsABGR)
                .def("setAsRGBA", &Ogre::ColourValue::setAsRGBA)
                .def("setAsARGB", &Ogre::ColourValue::setAsARGB)
                .def("setAsBGRA", &Ogre::ColourValue::setAsBGRA)
                .def("setAsABGR", &Ogre::ColourValue::setAsABGR)
                .def("setHSB", &Ogre::ColourValue::setHSB)
                .def("getHSB", &Ogre::ColourValue::getHSB)

                .def("saturate", &Ogre::ColourValue::saturate)
                .def("saturateCopy", &Ogre::ColourValue::saturateCopy)
                ;

            colourValueClass.attr("ZERO") = Ogre::ColourValue::ZERO;
            colourValueClass.attr("Black") = Ogre::ColourValue::Black;
            colourValueClass.attr("White") = Ogre::ColourValue::White;
            colourValueClass.attr("Red") = Ogre::ColourValue::Red;
            colourValueClass.attr("Green") = Ogre::ColourValue::Green;
            colourValueClass.attr("Blue") = Ogre::ColourValue::Blue;
    }

/*
 * =============================================================================================
 * NGF::Python::PythonGameObject
 * =============================================================================================
 */

    PythonGameObject::PythonGameObject()
	    : GameObject(Ogre::Vector3(), Ogre::Quaternion(), ID(), PropertyList(), ""),
	      mConnector(new PythonObjectConnector(this)),
	      mPythonEvents()
    {
    }
    //--------------------------------------------------------------------------------------
    PythonGameObject::~PythonGameObject()
    {
	    mConnector.reset();
    }
    //--------------------------------------------------------------------------------------
    void PythonGameObject::_initScript()
    {
            //Store the events.
            NGF_PY_SAVE_EVENT(init);
            NGF_PY_SAVE_EVENT(create);
            NGF_PY_SAVE_EVENT(destroy);
            NGF_PY_SAVE_EVENT(utick);
            NGF_PY_SAVE_EVENT(ptick);
            NGF_PY_SAVE_EVENT(collide);
    }
    //--------------------------------------------------------------------------------------
    void PythonGameObject::setScriptString(const Ogre::String &script)
    {
            //Run string.
            if (script != "") 
                    runString(script);

            _initScript();
    }
    //--------------------------------------------------------------------------------------
    void PythonGameObject::setScriptCodeObject(const Ogre::String &codeObjName)
    {
            //Run a code object.
            if (codeObjName != "")
                    runString("exec(" + codeObjName + ")");

            _initScript();
    }
    //--------------------------------------------------------------------------------------
    void PythonGameObject::setScriptFile(Ogre::String filename, const Ogre::String &resourceGroup)
    {
            //Ogre::String filename = "Prefab_Door.pyc";

            //Run code from file in Ogre resource locations.
            if (filename != "")
            {
                    Ogre::DataStreamPtr stream;

                    for (;;)
                    {
                            try
                            {
                                    stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename, resourceGroup);
                                    goto found;
                            }
                            catch (...)
                            {
                                    //If not found, try other files.
                                    char last = *(filename.rbegin());

                                    switch (last)
                                    {
                                            case 'o':
                                                    last = 'c';
                                                    goto found;

                                            case 'c':
                                                    last = '\0';
                                                    goto found;

                                            default:
                                                    OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND, 
                                                                    "File " + filename + "not found!", "NGF::Python::PythonGameObject::setScriptFile()");
                                    }
                            }
                    }
found:

                    size_t size = stream->size();

                    void *data = malloc(size);
                    stream->read(data, size);

                    //If .pyc/.pyo, run .pyc/.pyo, else run .py.
                    char last = *(filename.rbegin());
                    if ((last == 'c') || (last == 'o' && (Py_OptimizeFlag = 1)))
                    {
                            FILE *fp = fmemopen(data, size, "rb");
                            runPycFile(fp, filename.c_str());
                            fclose(fp);
                    }
                    else
                    {
                            FILE *fp = fmemopen(data, size, "r");
                            PyRun_AnyFile(fp, filename.c_str());
                            fclose(fp);
                    }

                    free(data);
            }

            _initScript();
    }
    //--------------------------------------------------------------------------------------
    void PythonGameObject::runString(Ogre::String script)
    { 
	    try
	    {
		    py::exec(script.c_str(), PythonManager::getSingleton().getMainNamespace(), PythonManager::getSingleton().getMainNamespace()); 
	    }
	    catch (...)
	    {
		    std::cerr << "Error loading script:";
		    PyErr_Print();
	    }
    }

    static void runPycFile(FILE *fp, const char *filename)
    {
            PyCodeObject *co;
            PyObject *v;
            long magic;

            magic = PyMarshal_ReadLongFromFile(fp);
            if (magic != PyImport_GetMagicNumber()) 
            {
                    char *err;
                    sprintf(err, "Bad magic number in file '%s'!", filename);
                    PyErr_SetString(PyExc_RuntimeError, err);

                    return;
            }

            (void) PyMarshal_ReadLongFromFile(fp);
            v = PyMarshal_ReadObjectFromFile(fp);

            if (!v || !PyCode_Check(v)) 
            {
                    Py_XDECREF(v);

                    char *err;
                    sprintf(err, "Bad code object in file '%s'!", filename);
                    PyErr_SetString(PyExc_RuntimeError, err);

                    return;
            }

            co = (PyCodeObject *)v;
            PyObject *main = PythonManager::getSingleton().getMainNamespace().ptr();
            PyEval_EvalCode(co, main, main);

            Py_DECREF(co);
    }

/*
 * =============================================================================================
 * NGF::Python::PythonManager
 * =============================================================================================
 */

    //--------------------------------------------------------------------------------------
    PythonManager::PythonManager(PythonManager::PrintFunc printer)
    {
	    //Do the binds.
	    initNgf();

	    //Import 'main' and get the main namespace.
	    mMainModule = py::import("__main__");
	    mMainNamespace = py::extract<py::dict>(mMainModule.attr("__dict__"));

	    //Set the printer.
	    mPrinter = printer;

	    //We give up and write the rest in Python. :P
	    py::exec(
		    "import sys\n"
		    "import Ngf\n\n"

		    //This code allows us to write 'self.someMethod()' for 'self.method("someMethod", ()),
		    //while allowing variables of form 'm_*' to be used locally by scripts, and 'p_*' to
		    //refer to C++ members. Closures FTW!
		    "def tmp_GameObjectConnector__getattr__(self, name):\n"
		    " 	if (name[:2] == \"m_\"):\n"
		    " 		return self.locals[name]\n"
		    " 	elif (name[:2] == \"p_\"):\n"
		    " 		return self.method(\"get_\" + name [2:], 0)\n"
		    " 	return lambda *args: self.method(name, args)\n\n"

		    "def tmp_GameObjectConnector__setattr__(self, name, value):\n"
		    " 	if (name[:2] == \"m_\"):\n"
		    " 		self.locals[name] = value\n"
		    " 	elif (name[:2] == \"p_\"):\n"
		    " 		self.method(\"set_\" + name[2:], (value,))\n\n"

		    "Ngf.GameObjectConnector.__getattr__ = tmp_GameObjectConnector__getattr__\n"
		    "Ngf.GameObjectConnector.__setattr__ = tmp_GameObjectConnector__setattr__\n"
		    "del tmp_GameObjectConnector__getattr__\n"
		    "del tmp_GameObjectConnector__setattr__\n\n"

		    //We override the output to send it to our own callback instead. The old streams remain 
		    //in sys.__stdout__, sys.__stderr__.
		    "class OutputRedirect:\n"
		    " 	def write(self, str):\n"
		    " 		Ngf._print(str)\n\n"

		    "outputRedirector = OutputRedirect()\n"
		    "sys.stdout = outputRedirector\n"
		    "sys.stderr = outputRedirector\n\n"

		    //For pickling support. The way this is done is really cool. You just pass a tuple
		    //containing __init__ arguments to construct the new object with!
		    "def tmp_Vector3__getinitargs__(self):\n"
		    " 	return (self.x, self.y, self.z)\n"
		    "Ngf.Vector3.__getinitargs__ = tmp_Vector3__getinitargs__\n"
		    "del tmp_Vector3__getinitargs__\n\n"

		    "def tmp_Quaternion__getinitargs__(self):\n"
		    " 	return (self.w, self.x, self.y, self.z)\n"
		    "Ngf.Quaternion.__getinitargs__ = tmp_Quaternion__getinitargs__\n"
		    "del tmp_Quaternion__getinitargs__\n\n"

		    "def tmp_Radian__getinitargs__(self):\n"
		    " 	return (self.valueRadians())\n"
		    "Ngf.Radian.__getinitargs__ = tmp_Radian__getinitargs__\n"
		    "del tmp_Radian__getinitargs__\n\n"

		    "def tmp_Degree__getinitargs__(self):\n"
		    " 	return (self.valueDegrees())\n"
		    "Ngf.Degree.__getinitargs__ = tmp_Degree__getinitargs__\n"
		    "del tmp_Degree__getinitargs__\n\n"

		    ,mMainNamespace,mMainNamespace
		    );
    }
    //--------------------------------------------------------------------------------------
    PythonManager& PythonManager::getSingleton(void)
    {
	    assert(ms_Singleton);
	    return *ms_Singleton;
    }
    //--------------------------------------------------------------------------------------
    PythonManager* PythonManager::getSingletonPtr(void)
    {
	    return ms_Singleton;
    }
    //--------------------------------------------------------------------------------------
    PythonObjectConnectorPtr PythonManager::_createObject(std::string type, std::string name, Ogre::Vector3 pos, 
		    Ogre::Quaternion rot, py::dict pyProps)
    {
	    //Iterate through string-string pairs (tuples) and add properties.
	    NGF::PropertyList props;
	    py::stl_input_iterator<py::tuple> iter(pyProps.iteritems()), end;
	    py::tuple cur;

	    for(; iter != end; ++iter)
	    {
		    cur = *iter;
                    Ogre::String name = py::extract<std::string>(cur[0]);
                    Ogre::String vals = py::extract<std::string>(cur[1]);

                    Ogre::String sep = " ";
                    if (!strncmp(vals.c_str(), "##", 2))
                        sep = "";

		    props.addProperty(name, vals, sep);
	    }

	    GameObject *obj = GameObjectManager::getSingleton().createObject(type, pos, rot, props, name);
	    PythonGameObject *PythonObject = dynamic_cast<PythonGameObject*>(obj);
	    return PythonObject ? (PythonObject->getConnector()) : PythonObjectConnectorPtr();
    }
    //--------------------------------------------------------------------------------------
    void PythonManager::_destroyObject(PythonObjectConnector *obj)
    {
	    PythonGameObject *PythonObject = obj->getObject();
	    GameObjectManager::getSingleton().requestDestroy(PythonObject->getID());
    }
    //--------------------------------------------------------------------------------------
    PythonObjectConnectorPtr PythonManager::_getObject(std::string name)
    {
            //Return NULL shared pointer (None in Python) if failed.
	    GameObject *obj = GameObjectManager::getSingleton().getByName(name);
	    if (!obj)
		    return PythonObjectConnectorPtr();

	    PythonGameObject *PythonObject = dynamic_cast<PythonGameObject*>(obj);
	    return PythonObject ? (PythonObject->getConnector()) : PythonObjectConnectorPtr();
    }
    //--------------------------------------------------------------------------------------
    PythonObjectConnectorPtr PythonManager::_getObject(int ID)
    {
            //Return NULL shared pointer (None in Python) if failed.
	    GameObject *obj = GameObjectManager::getSingleton().getByID(ID);
	    if (!obj)
		    return PythonObjectConnectorPtr();

	    PythonGameObject *PythonObject = dynamic_cast<PythonGameObject*>(obj);
	    return PythonObject ? (PythonObject->getConnector()) : PythonObjectConnectorPtr();
    }

/*
 * =============================================================================================
 * NGF::Python::PythonObjectConnector
 * =============================================================================================
 */

    PythonObjectConnector::~PythonObjectConnector() 
    {
            //Clears the locals.
            py::object main = PythonManager::getSingleton().getMainNamespace();
            mLocals = py::eval("0", main, main);
    }
    //--------------------------------------------------------------------------------------
    Ogre::String PythonObjectConnector::dumpLocals()
    {
            py::object &main = PythonManager::getSingleton().getMainNamespace();

            //Python stuff is best done in python. ;-)
            py::exec(
                            "import cPickle\n\n"

                            "def dump(obj):\n"
                            " 	return cPickle.dumps(obj.locals)\n",
                            main, main
                    );
            Ogre::String dump = py::extract<Ogre::String>(main["dump"](this));

            //Clean up after ourselves!
            py::exec(
                            "del dump\n",
                            main, main
                    );

            return dump;
    }
    //--------------------------------------------------------------------------------------
    void PythonObjectConnector::loadLocals(Ogre::String str)
    {
            py::object &main = PythonManager::getSingleton().getMainNamespace();

            //Python stuff is best done in python. ;-)
            py::exec(
                            "import cPickle\n\n"

                            "def load(str):\n"
                            " 	return cPickle.loads(str)\n",
                            main, main
                    );
            mLocals = main["load"](str);

            //Clean up after ourselves!
            py::exec(
                            "del load\n",
                            main, main
                    );
    }

} //namespace Python

} //namespace NGF
