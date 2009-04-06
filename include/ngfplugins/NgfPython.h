/*
 * =====================================================================================
 *
 *       Filename:  NgfPython.h
 *
 *    Description:  NGF-Python binding, and some other useful Python stuff.
 *
 *        Version:  1.0
 *        Created:  02/27/2009 11:40:53 PM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#ifndef __NGF_PYTHON_H__
#define __NGF_PYTHON_H__

#include <Ogre.h> //Change this to only include specific headers when done.
#include <Ngf.h>
#include <python2.6/Python.h>
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>

namespace py = boost::python;

namespace NGF { namespace Python {

class PythonObjectConnector;
//boost.python plays well with boost.shared_ptr.
typedef boost::shared_ptr<PythonObjectConnector> PythonObjectConnectorPtr;

/*
 * =====================================================================================
 *        Class:  PythonGameObject
 *
 *  Description:  A scriptable GameObject. Implement the get, set and method functions.
 *  		  The idea is similar to that followed in 'SimKin'.
 *
 *  		  We don't inherit from NGF::GameObject here to prevent
 *  		  multiple-inheritance problems.
 * =====================================================================================
 */

class PythonGameObject : virtual public GameObject
{
    protected:
	    PythonObjectConnectorPtr mConnector;

	    py::dict mPythonEvents;

    public:
	    PythonGameObject();
	    virtual ~PythonGameObject();

	    //--- Python GameObject interface, called through Python, via the 'connector' --

	    virtual py::object pythonMethod(Ogre::String name, py::object args)
	    { return py::object("base"); }

	    //--- Some helper functions --------------------------------------------

	    //Makes 'self' etc. work.
	    void runString(Ogre::String script);

	    //Get the connector.
	    PythonObjectConnectorPtr getConnector() { return mConnector; }

	    //Sets up the script.
	    void setScript(Ogre::String script);
};

/*
 * =====================================================================================
 *        Class:  PythonManager
 *
 *  Description:  Manages the binding, global functions, etc.
 * =====================================================================================
 */

class PythonManager : public Ogre::Singleton<PythonManager>
{
    public:
	    typedef fastdelegate::FastDelegate1<Ogre::String> PrintFunc;

    protected:
	    py::object mMainModule;
	    py::dict mMainNamespace;

	    static PrintFunc mPrinter;

    public:
	    PythonManager(PrintFunc printer);
	    ~PythonManager() {}

	    //Usual singleton stuff.
	    static PythonManager& getSingleton(void);
	    static PythonManager* getSingletonPtr(void);

	    //Get the Python main module or namespace.
	    inline py::object &getMainModule() { return mMainModule; }
	    inline py::dict &getMainNamespace() { return mMainNamespace; }

	    //--- Python binds, internal stuff -------------------------------------
	    
	    static PythonObjectConnectorPtr _createObject(std::string type, std::string name = "", 
		    Ogre::Vector3 pos = Ogre::Vector3::ZERO, Ogre::Quaternion = Ogre::Quaternion::IDENTITY, 
		    py::dict props = py::dict());
	    static void _destroyObject(PythonObjectConnector *obj);
	    static PythonObjectConnectorPtr _getObject(std::string name);
	    static PythonObjectConnectorPtr _getObject(int ID);

	    static void _print(std::string str) { if (mPrinter) mPrinter(str); }
};

/*
 * =====================================================================================
 *        Class:  PythonObjectConnector
 *
 *  Description:  The PythonGameObject 'ambassador' in the scripted world.
 * =====================================================================================
 */

class PythonObjectConnector
{
    public:
	    //A dictionary that 'saves' variables, including 'self' etc.
	    py::object mLocals;

    protected:
	    PythonGameObject *mObj;

    public:
	    PythonObjectConnector(PythonGameObject *obj)
		    : mObj(obj),
		      mLocals(py::dict()) //Make an empty 'locals' dictionary
	    {
	    }

	    ~PythonObjectConnector();

	    //Get pointer to our GameObject.
	    PythonGameObject *getObject() { return mObj; }

	    //Dumping or loading locals to a string using pickle. Useful if you want to save 
            //your GameObject's state (the NGF_PY_SERIALISE_LOCALS() macro at the end of this 
            //file makes it easy to do so with Ngf::Serialisation).
	    Ogre::String dumpLocals();
	    void loadLocals(Ogre::String str);

	    //--- Python calls this, this tells the GameObject ---------------------

	    py::object method(std::string name, py::object args) 
	    { return mObj->pythonMethod(Ogre::String(name), args); }

	    //--- GameObject interface wrapping ------------------------------------

	    ID getID()
	    { return mObj->getID(); }
	    std::string getName()
	    { return mObj->getName(); }
	    void addFlag(std::string flag)
	    { mObj->addFlag(flag); }
	    bool hasFlag(std::string name)
	    { return mObj->hasFlag(name); }
	    bool removeFlag(std::string name)
	    { return mObj->removeFlag(name); }
	    std::string getFlags()
	    { return mObj->getFlags(); }
}; 

} //namespace Python

} //namespace NGF

/*
 * =====================================================================================
 *       Macros:  NGF_PY_*
 *
 *  Description:  These macros make it easy to write methods useable from Python and
 *  		  map C++ member variables to Python attributes. Uses gperf for hashing,
 *  		  the tool 'ngfpydef' eases writing gperf configuration files by just
 *  		  parsing your '*_DECL' macros.
 *
 *  		  The macros use enums and switches to handle the messages. Gperf takes
 *  		  the string name, and returns a struct containing the enum value (it
 *  		  needs a gperf config file and 'ngfpydef' helps make that).
 * =====================================================================================
 */

//Macros for beginning or ending method or property declarations or implementations.
//These macros make the enums with a 'pm_' prefix for methods, 'pget_' prefix for
//get-methods, and 'pset_' prefix for set methods. Braces are added by the user.
#define NGF_PY_BEGIN_DECL(classnm)                                                         \
	py::object pythonMethod(Ogre::String name, py::object args);                       \
	enum
#define NGF_PY_METHOD_DECL(pyname)                                                         \
	pm_##pyname,                                                               
#define NGF_PY_PROPERTY_DECL(pyname)                                                       \
	pget_##pyname,                                                                     \
	pset_##pyname,
#define NGF_PY_END_DECL                                                                    \
	;

//Macros for declaring or implementing methods or properties. They rely on the enums.
//'NGF_PY_PROPERTY_IMPL' saves you from having to mess with the get and set methods,
//properties act like normal variables.
#define NGF_PY_BEGIN_IMPL(classnm)                                                             \
	py::object classnm::pythonMethod(Ogre::String NGF_name, py::object args)               \
	{                                                                                      \
	    const PythonMethod *NGF_res = PythonHash_##classnm                                 \
		::Lookup(NGF_name.c_str(), NGF_name.length());                                 \
	    if (NGF_res)                                                                       \
	    {                                                                                  \
		switch (NGF_res->code)                                                         \
		{
#define NGF_PY_METHOD_IMPL(pyname)                                                             \
		    case (pm_##pyname):
#define NGF_PY_PROPERTY_IMPL(pyname,cname,ctype)                                               \
		    case (pget_##pyname):                                                      \
			return py::object(cname);                                              \
		    case (pset_##pyname):                                                      \
		    {                                                                          \
			cname = py::extract<ctype>(args[0]);                                   \
			return py::object();                                                   \
		    }
#define NGF_PY_END_IMPL                                                                        \
		}                                                                              \
	    }                                                                                  \
	    return py::object();                                                               \
	}

//To save an event. If the event isn't defined, a 'do nothing' event is created. After
//saving the event, it is removed from the main namespace, so the name can be reused.
#define NGF_PY_SAVE_EVENT(evt)                                                                 \
        mPythonEvents[#evt]                                                                    \
                = NGF::Python::PythonManager::getSingleton().getMainNamespace()                \
                        .setdefault(#evt, py::eval("lambda *args: None"));                     \
        runString("del " #evt "\n")

//To call a saved event. The event (funciton) receives a 'self' parameter, and whatever
//other parameters are passed in.
#define NGF_PY_CALL_EVENT(evt, ...)                                                            \
        mPythonEvents[#evt](mConnector, ##__VA_ARGS__)

//Used by 'ngfpydef', you don't usually have to use these yourself.
#define NGF_PY_CLASS_GPERF(classnm) PythonHash_##classnm 
#define NGF_PY_METHOD_GPERF(classnm,pyname) classnm :: pm_##pyname
#define NGF_PY_GET_GPERF(classnm,pyname) classnm :: pget_##pyname
#define NGF_PY_SET_GPERF(classnm,pyname) classnm :: pset_##pyname

#endif //#ifndef __NGF_PYTHON_H__
