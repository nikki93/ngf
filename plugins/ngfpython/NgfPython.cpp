/*
 * =====================================================================================
 *
 *       Filename:  NgfPython.cpp
 *
 *    Description:  NGF-Python implementation
 *
 *        Version:  1.0
 *        Created:  15/03/2009 11:29:15 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#include "ngfplugins/NgfPython.h"
#include <boost/python/stl_iterator.hpp>

template<> NGF::PythonManager* Ogre::Singleton<NGF::PythonManager>::ms_Singleton = 0;
NGF::PythonManager::PrintFunc NGF::PythonManager::mPrinter = 0;

namespace NGF {

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
		    ;
	    
	    //Bind Ogre::Quaternion.
	    py::class_<Ogre::Quaternion>("Quaternion", py::init<>())
		    .def(py::init<Ogre::Real,Ogre::Real,Ogre::Real,Ogre::Real>())
		    .def_readwrite("w", &Ogre::Quaternion::w)
		    .def_readwrite("x", &Ogre::Quaternion::x)
		    .def_readwrite("y", &Ogre::Quaternion::y)
		    .def_readwrite("z", &Ogre::Quaternion::z)
		    ;
    }

/*
 * =============================================================================================
 * NGF::PythonGameObject
 * =============================================================================================
 */

    PythonGameObject::PythonGameObject(Ogre::Vector3 pos, Ogre::Quaternion rot, ID id, PropertyList properties, Ogre::String name)
	    : GameObject(pos, rot, id , properties, name),
	      mConnector(new PythonObjectConnector(this)),
	      mPyEvents(py::dict())
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

	    //The script should /do/ something.
	    if (script != "")
	    {
		    //The default events, in case the script doesn't override some.
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

		    //Run the script.
		    runString(script);

		    //Bind the events.
		    mPyEvents["init"] = main["init"];
		    mPyEvents["create"] = main["create"];
		    mPyEvents["destroy"] = main["destroy"];
		    mPyEvents["utick"] = main["utick"];
		    mPyEvents["ptick"] = main["ptick"];
		    mPyEvents["collide"] = main["collide"];

		    //Clear the temporary function names.
		    runString(
				    "del create\n"
				    "del destroy\n"
				    "del utick\n"
				    "del ptick\n"
				    "del collide\n"
			     );
	    }
	    else
	    {
		    //No events, bind some empty functions.
		    mPyEvents["init"] = py::eval("lambda *args: 0", main, main); 
		    mPyEvents["create"] = mPyEvents["init"];
		    mPyEvents["destroy"] = mPyEvents["init"];
		    mPyEvents["utick"] = mPyEvents["init"];
		    mPyEvents["ptick"] = mPyEvents["init"];
		    mPyEvents["collide"] = mPyEvents["init"];
	    }
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
 * NGF::PythonManager
 * =============================================================================================
 */

    //--------------------------------------------------------------------------------------
    PythonManager::PythonManager(PythonManager::PrintFunc printer = 0)
    {
	    //Do the binds.
	    initNgf();

	    //Import 'main' and get the main namespace.
	    mMainModule = py::import("__main__");
	    mMainNamespace = mMainModule.attr("__dict__");

	    //Set the printer.
	    mPrinter = printer;

	    //Some Python stuff.
	    py::exec(
		    "import sys\n"
		    "import Ngf\n\n"

		    //This code allows us to write 'self.someMethod()' for 'self.method("someMethod", ()),
		    //while allowing variables of form 'm_' to be used locally by scripts, and 'p_' to
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
		    "Ngf.GameObjectConnector.__setattr__ = tmp_GameObjectConnector__setattr__\n\n"

		    //We override the output to send it to our own callback instead. The old streams remain 
		    //in sys.__stdout__, sys.__stderr__.
		    "class OutputRedirect:\n"
		    " 	def write(self, str):\n"
		    " 		Ngf._print(str)\n\n"

		    "outputRedirector = OutputRedirect()\n"
		    "sys.stdout = outputRedirector\n"
		    "sys.stderr = outputRedirector\n\n"

		    //Remove the temporaries.
		    "del tmp_GameObjectConnector__getattr__\n"
		    "del tmp_GameObjectConnector__setattr__\n"
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

} //namespace NGF
