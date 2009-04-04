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

/*
 * =============================================================================================
 * The bindings
 * =============================================================================================
 */

    BOOST_PYTHON_MODULE(Ngf)
    {
	    //Bind some non-class stuff.		

	    //Raw-pointer version.
	    /*
	    py::def("createObject", &PythonManager::_createObject, 
		    py::return_value_policy<py::reference_existing_object>());
	    py::def("destroyObject", &PythonManager::_destroyObject);
	    py::def("_print", &PythonManager::_print);

	    PythonObjectConnectorPtr (*getObjectStr) (std::string) = &PythonManager::_getObject;
	    PythonObjectConnectorPtr (*getObjectID) (int) = &PythonManager::_getObject;
	    py::def("getObject", getObjectStr, py::return_value_policy<py::reference_existing_object>());
	    py::def("getObject", getObjectID, py::return_value_policy<py::reference_existing_object>());
	    */

	    //Smart-pointer version.
	    py::def("createObject", &PythonManager::_createObject);
	    py::def("destroyObject", &PythonManager::_destroyObject);
	    py::def("_print", &PythonManager::_print);

	    PythonObjectConnectorPtr (*getObjectStr) (std::string) = &PythonManager::_getObject;
	    PythonObjectConnectorPtr (*getObjectID) (int) = &PythonManager::_getObject;
	    py::def("getObject", getObjectStr);
	    py::def("getObject", getObjectID);

	    //Bind our NGF connector.
	    py::class_<PythonObjectConnector, PythonObjectConnectorPtr >("GameObjectConnector", py::no_init)
		    .def_readwrite("locals", &PythonObjectConnector::mLocals)
		    .def("method", &PythonObjectConnector::method)
		    .def("getID", &PythonObjectConnector::getID)
		    .def("getName", &PythonObjectConnector::getName)
		    .def("addFlag", &PythonObjectConnector::addFlag)
		    .def("hasFlag", &PythonObjectConnector::hasFlag)
		    .def("removeFlag", &PythonObjectConnector::removeFlag)
		    .def("getFlags", &PythonObjectConnector::getFlags)
		    ;

	    //Bind Ogre::Vector3.
	    py::class_<Ogre::Vector3>("Vector3", py::init<>())
		    .def(py::init<Ogre::Real,Ogre::Real,Ogre::Real>())
		    .def_readwrite("x", &Ogre::Vector3::x)
		    .def_readwrite("y", &Ogre::Vector3::y)
		    .def_readwrite("z", &Ogre::Vector3::z)
		    .enable_pickling()
		    ;
	    
	    //Bind Ogre::Quaternion.
	    py::class_<Ogre::Quaternion>("Quaternion", py::init<>())
		    .def(py::init<Ogre::Real,Ogre::Real,Ogre::Real,Ogre::Real>())
		    .def_readwrite("w", &Ogre::Quaternion::w)
		    .def_readwrite("x", &Ogre::Quaternion::x)
		    .def_readwrite("y", &Ogre::Quaternion::y)
		    .def_readwrite("z", &Ogre::Quaternion::z)
		    .enable_pickling()
		    ;
    }

/*
 * =============================================================================================
 * NGF::Python::PythonGameObject
 * =============================================================================================
 */

    PythonGameObject::PythonGameObject()
	    : GameObject(Ogre::Vector3(), Ogre::Quaternion(), ID(), PropertyList(), ""),
	      mConnector(new PythonObjectConnector(this)),
	      mPythonEvents(py::dict())
    {
    }
    //--------------------------------------------------------------------------------------
    PythonGameObject::~PythonGameObject()
    {
	    mConnector.reset();
    }
    //--------------------------------------------------------------------------------------
    void PythonGameObject::setUpScript(Ogre::String script)
    {
            py::object main = PythonManager::getSingleton().getMainNamespace();

            //The default events, in case the script doesn't override some.
            /*
            runString(
                            "import Ngf\n\n"

                            "def init(self):\n"
                            " 	pass\n\n"
                            "def create(self):\n"
                            " 	pass\n\n"
                            "def destroy(self):\n"
                            " 	pass\n\n"
                            "def utick(self, elapsed):\n"
                            " 	pass\n\n"
                            "def ptick(self, elapsed):\n"
                            " 	pass\n\n"
                            "def collide(self, other):\n"
                            " 	pass\n\n"
                     );
                     */

            if (script != "") 
                    runString(script);

            //Store the events.
            NGF_PY_SAVE_EVENT(init);
            NGF_PY_SAVE_EVENT(create);
            NGF_PY_SAVE_EVENT(destroy);
            NGF_PY_SAVE_EVENT(utick);
            NGF_PY_SAVE_EVENT(ptick);
            NGF_PY_SAVE_EVENT(collide);
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

/*
 * =============================================================================================
 * NGF::Python::PythonManager
 * =============================================================================================
 */

    //--------------------------------------------------------------------------------------
    PythonManager::PythonManager(PythonManager::PrintFunc printer = 0)
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
		    props.addProperty(py::extract<std::string>(cur[0]), py::extract<std::string>(cur[1]));
	    }

	    GameObject *obj = GameObjectManager::getSingleton().createObject(type, pos, rot, props, name);
	    PythonGameObject *PythonObject = dynamic_cast<PythonGameObject*>(obj);
	    return PythonObject ? (PythonObject->getConnector()) : PythonObjectConnectorPtr();
    }
    //--------------------------------------------------------------------------------------
    void PythonManager::_destroyObject(PythonObjectConnector *obj)
    {
	    PythonGameObject *PythonObject = obj->getObject();
	    Ogre::LogManager::getSingleton().logMessage("Before, ID: " + Ogre::StringConverter::toString(PythonObject->getID()));
	    GameObjectManager::getSingleton().requestDestroy(PythonObject->getID());
    }
    //--------------------------------------------------------------------------------------
    PythonObjectConnectorPtr PythonManager::_getObject(std::string name)
    {
	    GameObject *obj = GameObjectManager::getSingleton().getByName(name);
	    if (!obj)
		    return PythonObjectConnectorPtr();

	    PythonGameObject *PythonObject = dynamic_cast<PythonGameObject*>(obj);
	    return PythonObject ? (PythonObject->getConnector()) : PythonObjectConnectorPtr();
    }
    //--------------------------------------------------------------------------------------
    PythonObjectConnectorPtr PythonManager::_getObject(int ID)
    {
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
