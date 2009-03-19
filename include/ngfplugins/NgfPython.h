/*
 * =====================================================================================
 *
 *       Filename:  NgfPython.h
 *
 *    Description:  NGF-Python binding, and some other useful Python stuff.
 *
 *        Version:  1.0
 *        Created:  02/27/2009 11:40:53 PM
 *       Revision:  none
 *       Compiler:  gcc
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

namespace NGF {

/*
 * =====================================================================================
 *        Class:  PythonGameObject
 *
 *  Description:  A scriptable GameObject. Implement the get, set and method functions.
 *  		  The idea is similar to that followed in 'SimKin'.
 * =====================================================================================
 */

class PythonObjectConnector;
typedef boost::shared_ptr<PythonObjectConnector> PythonObjectConnectorPtr;

class PythonGameObject : public GameObject
{
    protected:
	    PythonObjectConnectorPtr mConnector;

	    py::object mPyEvents;

    public:
	    PythonGameObject(Ogre::Vector3 pos, Ogre::Quaternion rot, NGF::ID id, 
			    NGF::PropertyList properties, Ogre::String name);
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
	    void setUpScript(Ogre::String script);
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
	    py::object mMainNamespace;

	    static PrintFunc mPrinter;

    public:
	    PythonManager(PrintFunc printer);
	    ~PythonManager() {}

	    //Usual singleton stuff.
	    static PythonManager& getSingleton(void);
	    static PythonManager* getSingletonPtr(void);

	    //Get the Python main module or namespace.
	    py::object &getMainModule() { return mMainModule; }
	    py::object &getMainNamespace() { return mMainNamespace; }

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

	    ~PythonObjectConnector() 
	    {
		    //Clears the locals.
		    py::object main = PythonManager::getSingleton().getMainNamespace();
		    mLocals = py::eval("0", main, main);
	    }

	    //Get pointer to our GameObject.
	    PythonGameObject *getObject() { return mObj; }

	    //--- Python calls this, this tells the GameObject ---------------------

	    py::object method(std::string name, py::object args) 
	    { return mObj->pythonMethod(Ogre::String(name), args); }

	    //--- GameObject interface wrapping ------------------------------------

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

} //namespace NGF

/*
 * =====================================================================================
 *       Macros:  NGF_PY_*
 *
 *  Description:  Makes it easier to write methods and map properties from C++ to
 *  		  Python. Uses gperf for string hashing.
 * =====================================================================================
 */

//Hacks for method declarations.
#define NGF_PY_BEGIN_DECL py::object pythonMethod(Ogre::String name, py::object args); enum
#define NGF_PY_END_DECL ;
#define NGF_PY_METHOD_DECL(pyname) pm_ ## pyname,
#define NGF_PY_PROPERTY_DECL(pyname) pget_ ## pyname, pset_ ## pyname,

//Hacks for inline definitions.
#define NGF_PY_BEGIN_INLINE py::object pythonMethod(Ogre::String NGF_name, py::object args) { \
    const PythonMethod *NGF_res = PythonHash_ ## classnm ::Lookup(NGF_name.c_str(), NGF_name.length()); \
    if ( NGF_res ) switch ( NGF_res->code ) {
#define NGF_PY_END_INLINE } return py::object(); }

//Hacks for 'out-of-header' implementations.
#define NGF_PY_BEGIN_IMPL(classnm) py::object classnm ::pythonMethod(Ogre::String NGF_name, py::object args) { \
    const PythonMethod *NGF_res = PythonHash_ ## classnm ::Lookup(NGF_name.c_str(), NGF_name.length()); \
    if ( NGF_res ) switch ( NGF_res->code ) {
#define NGF_PY_END_IMPL } return py::object(); }

//Hacks for implementing methods or properties, can be used both in 'inline's or in 'impl's.
#define NGF_PY_METHOD(pyname) case (pm_ ## pyname):
#define NGF_PY_PROPERTY(cname,ctype) case (pget_ ## cname): { return py::object(cname); } \
    case (pset_ ## cname): { cname = py::extract<ctype>(args[0]); return py::object(); }

//Hacks for easy gperf-writing. Just write <python-name> NGF_PY_METHOD_GPERF(<class-name>, <enum-name>)
//in your gperf keywords list.
#define NGF_PY_CLASS_GPERF(classnm) PythonHash_ ## classnm 
#define NGF_PY_METHOD_GPERF(classnm,pyname) classnm :: pm_ ## pyname
#define NGF_PY_GET_GPERF(classnm,pyname) classnm :: pget_ ## pyname
#define NGF_PY_SET_GPERF(classnm,pyname) classnm :: pset_ ## pyname

#endif //#ifndef __NGF_PYTHON_H__
