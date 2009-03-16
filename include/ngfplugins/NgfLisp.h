/*
 * =====================================================================================
 *
 *       Filename:  NgfLispInterface.h
 *
 *    Description:  Stuff to help use NGF with Lisp scripting using ECL.
 *
 *        Version:  1.0
 *        Created:  02/22/2009 08:34:59 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#include <ecl/ecl.h>
#include <OgrePrerequisites.h>
#include <OgreStringConverter.h>

namespace NGF {

/*
 * =====================================================================================
 *        Class:  LispGameObject
 *
 *  Description:  A scriptable GameObject. Implement the get, set and method functions.
 *  		  The idea is similar to that followed in 'SimKin'.
 * =====================================================================================
 */

class LispGameObject : public GameObject
{
protected:
	//These store our event scripts.
	Ogre::String mCreateScript, mUTickScript, mPTickScript, mColScript, mDestScript;

public:
	LispGameObject(Ogre::Vector3 pos, Ogre::Quaternion rot, NGF::ID id, NGF::PropertyList properties, Ogre::String name);
	~LispGameObject() {}

	//Run a script. Use this to get *this* working.
	cl_object runScript(Ogre::String script);

	//--- Lisp GameObject interface. For, (ngf-method ... ), (ngf-get ...) etc. ----

	virtual cl_object lispGet(Ogre::String name) { return Cnil; }
	virtual void lispSet(Ogre::String name, cl_object value) {}
	virtual cl_object lispMethod(Ogre::String name, cl_object args) { return Cnil; }

	//--- Events. Call these from your derived object's respective methods ---------

	void lispCreate(); //<-- Don't call this one. This one get's called automatically.
	void lispDestroy();
	void lispUnpausedTick(const Ogre::FrameEvent& evt);
	void lispPausedTick(const Ogre::FrameEvent& evt);
	void lispCollide(GameObject *other);
};

/*
 * =====================================================================================
 *        Class:  LispInterface
 *
 *  Description:  Makes ECL use easy. You can bind your own functions, and LispInterface
 *  		  already provides the following:-
 *
 *  		  (ngf-get-all)
 *  		  Returns a list of IDs of all GameObjects.
 *
 *  		  (ngf-get <idno/name> <valuename>)
 *  		  Get a LispGameObject's value.
 *
 *  		  (ngf-set <idno/name> <valuename> <valuevalue>)
 *  		  Set a LispGameObject's value.
 *
 *  		  (ngf-method <idno/name> <methodname> <argumentlist>)
 *  		  Call a LispGameObject's method.
 *
 *  		  (ngf-send-message <idno/name> <subject> <body>)
 *  		  Sends message to GameObject with given ID.
 *
 *  		  *this*
 *  		  The ID of the GameObject running a script. For this to work, any
 *  		  GameObject calling a script should pass it's ID as the second argument
 *  		  of 'runScript()'.
 *
 *  	   Note:  LispInterface does not boot or shutdown ecl, you have to do it
 *  	   	  yourself with cl_boot() and cl_shutdown().
 * =====================================================================================
 */

class LispInterface 
{
protected:
	typedef fastdelegate::FastDelegate1<Ogre::String> PrinterFunction;
	static PrinterFunction mPrintFunc;
	static NGF::Loading::LoaderHelperFunction mHelpFunc;

public:

	//--- Constructor/Destructor ---------------------------------------------------

	LispInterface(NGF::Loading::LoaderHelperFunction);
	~LispInterface();

	//--- Public interface ---------------------------------------------------------

	//Run the given Ogre::String. If you're running from a GameObject, make sure
	//you pass in the ID, so that scripts can use the '*this*' value to
	//get the ID.
	static cl_object runScript(Ogre::String script);

	//Bind a function. Don't forget to cast the funcPtr to (cl_object)!
	static void bindFunction(Ogre::String name, void *funcPtr, int numArgs);

	//Set the printer function, of form 'void func(Ogre::String* str)'. This function
	//receives the output from ECL.
	void setPrinterFunction(PrinterFunction func);

	//Conversion stuff.
	static cl_object convertFloat(Ogre::Real val) { return ecl_make_doublefloat(val); }
	static Ogre::Real convertFloat(cl_object val) { return ecl_to_double(val); }
	static cl_object convertInt(int val) { return MAKE_FIXNUM(val); }
	static Ogre::Real convertInt(cl_object val) { return ecl_to_fixnum(val); }
	static cl_object convertString(Ogre::String val) { return make_base_string_copy(val.c_str()); }
	static Ogre::String convertString(cl_object val) { return (char*)val->base_string.self; }

	//--- Binds, internal stuff, etc. ----------------------------------------------

	static void _addID(NGF::GameObject *);

	static cl_object _lsp_createObject(cl_object, cl_object, cl_object, cl_object, cl_object);
	static cl_object _lsp_sendMessage(cl_object, cl_object, cl_object);
	static cl_object _lsp_getIDs();
	static cl_object _lsp_printFunction(cl_object);

	static cl_object _lsp_objGet(cl_object obj, cl_object name);
	static cl_object _lsp_objSet(cl_object obj, cl_object name, cl_object value);
	static cl_object _lsp_objMethod(cl_object obj, cl_object name, cl_object args);
};

} //namespace NGF

//=-= .CPP File =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=

//#include "NgfLispInterface.h"

#define cl_for_each_begin(x, y, list) cl_object x, y = (list); do { x = cl_car(y);
#define cl_for_each_end(y) } while ((y = cl_cdr(y)) != Cnil);

namespace NGF {

/*
 * =============================================================================================
 * NGF::LispGameObject
 * =============================================================================================
 */

	LispGameObject::LispGameObject(Ogre::Vector3 pos, Ogre::Quaternion rot, NGF::ID id, NGF::PropertyList properties, Ogre::String name) : GameObject(pos, rot, id , properties, name)
	{
		mCreateScript = properties.getValue("ngf-evt-create", 0, "");
		mUTickScript = properties.getValue("ngf-evt-utick", 0, "");
		mPTickScript = properties.getValue("ngf-evt-ptick", 0, "");
		mColScript = properties.getValue("ngf-evt-collide", 0, "");
		mDestScript = properties.getValue("ngf-evt-destroy", 0, "");
		runScript("(defparameter *locals-" + Ogre::StringConverter::toString(getID()) + "* nil)");
	}
	//--------------------------------------------------------------------------------------
	void LispGameObject::lispCollide(GameObject *other)
	{
		if (mColScript != "")
		{
			runScript("(defparameter *other* " + Ogre::StringConverter::toString(other->getID()) + ")");
			runScript(mColScript);
		}
	}
	//--------------------------------------------------------------------------------------
	void LispGameObject::lispCreate()
	{
		if (mCreateScript != "")
			runScript(mCreateScript);

		mCreateScript = "";
	}
	//--------------------------------------------------------------------------------------
	void LispGameObject::lispDestroy()
	{
		if (mDestScript != "")
			runScript(mDestScript);
		runScript("(defparameter *locals-" + Ogre::StringConverter::toString(getID()) + "* nil)");
	}
	//--------------------------------------------------------------------------------------
	void LispGameObject::lispUnpausedTick(const Ogre::FrameEvent& evt) 
	{ 
	    	lispCreate();
		if (mUTickScript != "") 
		{
			runScript("(defparameter *elapsed* " + Ogre::StringConverter::toString(evt.timeSinceLastFrame) + ")");
			runScript(mUTickScript);
		}
	}
	//--------------------------------------------------------------------------------------
	void LispGameObject::lispPausedTick(const Ogre::FrameEvent& evt) 
	{ 
	    	lispCreate();
		if (mPTickScript != "")
		{
			runScript("(defparameter *elapsed* " + Ogre::StringConverter::toString(evt.timeSinceLastFrame) + ")");
			runScript(mPTickScript);
		}
	}
	//--------------------------------------------------------------------------------------
	cl_object LispGameObject::runScript(Ogre::String script)
	{
	    LispInterface::runScript("(setf *this* " + Ogre::StringConverter::toString(getID()) + ")");
	    LispInterface::runScript("(setf *locals* *locals-" + Ogre::StringConverter::toString(getID()) + "*)");
	    return LispInterface::runScript(script);
	}

/*
 * =============================================================================================
 * NGF::LispInterface
 * =============================================================================================
 */

	LispInterface::PrinterFunction LispInterface::mPrintFunc = 0;
	NGF::Loading::LoaderHelperFunction LispInterface::mHelpFunc = 0;
	//--------------------------------------------------------------------------------------
	LispInterface::LispInterface(NGF::Loading::LoaderHelperFunction helpFunc)
	{
	    	mHelpFunc = helpFunc;

	    	//Bind stuff.
		bindFunction("ngf-send-message", (cl_object) _lsp_sendMessage, 3);
		bindFunction("ngf-get-all", (cl_object) _lsp_getIDs, 0);
		bindFunction("ngf-get", (cl_object) _lsp_objGet, 2);
		bindFunction("ngf-set", (cl_object) _lsp_objSet, 3);
		bindFunction("ngf-method", (cl_object) _lsp_objMethod, 3);
		bindFunction("ngf-create", (cl_object) _lsp_createObject, 5);

		cl_safe_eval(c_string_to_object("(defvar *this* -1)"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defvar *ngf-ids* nil)"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defvar *locals* nil)"), Cnil, OBJNULL);

		//Output redirection.
		cl_safe_eval(c_string_to_object("(defvar *print-function* nil)"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defclass accumulate-stream (gray:fundamental-output-stream) ((lbuffer :accessor buffer-of :initform \"\") (dirty   :accessor is-dirty :initform nil)))"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defmethod stream-flush-buffer ((stream accumulate-stream)) (if (and (is-dirty stream) *print-function*) (funcall *print-function* (buffer-of stream))) (gray:stream-clear-output stream))"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defmethod gray:stream-force-output ((stream accumulate-stream)) (declare (ignore stream)) (stream-flush-buffer stream) nil)"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defmethod gray:stream-line-column ((stream accumulate-stream)) nil)"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defmethod gray:stream-finish-output ((stream accumulate-stream)) (not (is-dirty stream)))"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defmethod gray:stream-clear-output ((stream accumulate-stream)) (setf (buffer-of stream) \"\" (is-dirty stream) nil))"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defmethod gray:stream-write-string ((stream accumulate-stream) str &optional start end) (setf (is-dirty stream) t) (setf (buffer-of stream) (concatenate 'string (buffer-of stream) str)))"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defmethod gray:stream-write-char ((stream accumulate-stream) char) (setf (is-dirty stream) t) (if (char= char #\\Newline) (stream-flush-buffer stream) (setf (buffer-of stream) (concatenate 'string (buffer-of stream) (coerce (make-array 1 :initial-element char) 'string)))))"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(defmethod gray:stream-fresh-line ((stream accumulate-stream)) (stream-flush-buffer stream))"), Cnil, OBJNULL);
		cl_safe_eval(c_string_to_object("(setq *standard-output* (make-instance 'accumulate-stream))"), Cnil, OBJNULL);

		cl_def_c_function((cl_object) c_string_to_object("c-printer"), (cl_object) _lsp_printFunction, 1);
		cl_safe_eval(c_string_to_object("(setf *print-function* #'c-printer)"), Cnil, OBJNULL);

		runScript("(format t \"Lisp environment initialised!\")");
	}
	//--------------------------------------------------------------------------------------
	LispInterface::~LispInterface()
	{
	}
	//--------------------------------------------------------------------------------------
	cl_object LispInterface::runScript(Ogre::String script)
	{
		cl_object ret = cl_safe_eval(c_string_to_object(script.c_str()), Cnil, OBJNULL);
		ecl_force_output(cl_safe_eval(c_string_to_object("*standard-output*"), Cnil, OBJNULL));
		return ret;
	}
	//--------------------------------------------------------------------------------------
	void LispInterface::bindFunction(Ogre::String name, void *funcPtr, int numArgs)
	{
		cl_def_c_function((cl_object) c_string_to_object(name.c_str()), (cl_object) funcPtr, numArgs);
	}
	//--------------------------------------------------------------------------------------
	void LispInterface::setPrinterFunction(PrinterFunction func)
	{
		mPrintFunc = func;
	}
	//--------------------------------------------------------------------------------------
	cl_object LispInterface::_lsp_sendMessage(cl_object obj, cl_object lsp_subject, cl_object lsp_args)
	{
		Ogre::String subject((char*) lsp_subject->base_string.self);
		Ogre::String args((char*) lsp_args->base_string.self);
		GameObject *o = 0;

		if (type_of(obj) == t_base_string)
		{
			o = NGF::GameObjectManager::getSingleton().getByName((char*)obj->base_string.self);
			if (o)
				NGF::GameObjectManager::getSingleton().sendMessage(o, NGF::Message(subject, args));
		}

		o = NGF::GameObjectManager::getSingleton().getByID(ecl_to_fixnum(obj));
		if (o)
			NGF::GameObjectManager::getSingleton().sendMessage(o, NGF::Message(subject, args));

		return Cnil;
	}
	//--------------------------------------------------------------------------------------
	cl_object LispInterface::_lsp_getIDs()
	{
		runScript("(setf *ngf-ids* nil)");
		NGF::GameObjectManager::getSingleton().forEachGameObject(_addID);

		return runScript("*ngf-ids*");
	}
	//--------------------------------------------------------------------------------------
	void LispInterface::_addID(NGF::GameObject *obj)
	{
		runScript("(push " + Ogre::StringConverter::toString(obj->getID()) + " *ngf-ids*)");
	}
	//--------------------------------------------------------------------------------------
	cl_object LispInterface::_lsp_printFunction(cl_object str)
	{
		if (!mPrintFunc)
			return Cnil;

		Ogre::String s((char*)str->base_string.self); 
		mPrintFunc(s);

		return Cnil;
	}
	//--------------------------------------------------------------------------------------
	cl_object LispInterface::_lsp_objGet(cl_object obj, cl_object name)
	{
		LispGameObject *o = 0;

		if (type_of(obj) == t_base_string)
		{
			o = dynamic_cast<LispGameObject*>(NGF::GameObjectManager::getSingleton().getByName((char*)obj->base_string.self));
			return (o ? o->lispGet((char*)name->base_string.self) : Cnil);
		}

		o = dynamic_cast<LispGameObject*>(NGF::GameObjectManager::getSingleton().getByID(ecl_to_fixnum(obj)));
		return (o ? o->lispGet((char*)name->base_string.self) : Cnil);
	}
	//--------------------------------------------------------------------------------------
	cl_object LispInterface::_lsp_objSet(cl_object obj, cl_object name, cl_object value)
	{
		LispGameObject *o = 0;

		if (type_of(obj) == t_base_string)
		{
			o = dynamic_cast<LispGameObject*>(NGF::GameObjectManager::getSingleton().getByName((char*)obj->base_string.self));
			if (o) 
				o->lispSet((char*)name->base_string.self, value);
			return Cnil;
		}

		o = dynamic_cast<LispGameObject*>(NGF::GameObjectManager::getSingleton().getByID(ecl_to_fixnum(obj)));
		if (o) 
			o->lispSet((char*)name->base_string.self, value);
		return Cnil;
	}
	//--------------------------------------------------------------------------------------
	cl_object LispInterface::_lsp_objMethod(cl_object obj, cl_object name, cl_object args)
	{
		LispGameObject *o = 0;

		if (type_of(obj) == t_base_string)
		{
			o = dynamic_cast<LispGameObject*>(NGF::GameObjectManager::getSingleton().getByName((char*)obj->base_string.self));
			return (o ? o->lispMethod((char*)name->base_string.self, args) : Cnil);
		}

		o = dynamic_cast<LispGameObject*>(NGF::GameObjectManager::getSingleton().getByID(ecl_to_fixnum(obj)));
		assert(o != NULL);
		return (o ? o->lispMethod((char*)name->base_string.self, args) : Cnil);
	}
	//--------------------------------------------------------------------------------------
	cl_object LispInterface::_lsp_createObject(cl_object lsp_type, cl_object lsp_name, cl_object lsp_pos, cl_object lsp_rot, cl_object lsp_props)
	{
	    if (!mHelpFunc)
		return Cnil;

	    Ogre::Vector3 pos = Ogre::Vector3(convertFloat(cl_first(lsp_pos)), convertFloat(cl_second(lsp_pos)), convertFloat(cl_third(lsp_pos)));
	    Ogre::Quaternion rot = Ogre::Quaternion(convertFloat(cl_first(lsp_rot)), convertFloat(cl_second(lsp_rot)), convertFloat(cl_third(lsp_rot)),
		    convertFloat(cl_third(lsp_rot)));

	    NGF::PropertyList props;

	    if (lsp_props != Cnil)
	    {
		cl_for_each_begin(curProp, outerLoop, lsp_props)
		    props.addProperty(convertString(cl_first(curProp)), convertString(cl_second(curProp)));
		cl_for_each_end(outerLoop);
	    }

	    mHelpFunc(convertString(lsp_type), convertString(lsp_name), pos, rot, props);
	}
} //namespace NGF
