/*
 * =====================================================================================
 *
 *       Filename:  NgfSerialisation.h
 *
 *    Description:  GameObject saving and loading mechanism.
 *
 *        Version:  1.0
 *        Created:  03/22/2009 09:47:54 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#ifndef __NGF_SERIALISATION_H__
#define __NGF_SERIALISATION_H__

#include <Ogre.h> //Change this to only include specific headers when done.
#include <Ngf.h>
#include <boost/archive/text_oarchive.hpp>

namespace NGF { namespace Serialisation {

class GameObjectRecord;

/*
 * =====================================================================================
 *        Class:  SerialisableGameObject
 *
 *  Description:  An interface for GameObjects that can be saved and loaded. Implement
 *  		  the 'save' and 'load' methods.
 * =====================================================================================
 */

class SerialisableGameObject : virtual public GameObject
{
    public:
        SerialisableGameObject()
            : GameObject(Ogre::Vector3(), Ogre::Quaternion(), ID(), PropertyList(), "")
        {
        }

        //Override this to get serialisability. The 'save' parameter tells you whether
        //we are in write mode (if true) or read mode (if false). If in write mode,
        //return a GameObjectRecord with your type and whatever information you want
        //to save. If in read mode, use the 'in' PropertyList to read back the information
        //you wrote.
        virtual GameObjectRecord serialise(bool save, PropertyList &in) = 0;
};

/*
 * =====================================================================================
 *        Class:  Serialiser
 *
 *  Description:  Easy saving and loading of GameObjects with just 
 *  		  NGF::Serialiser::save(filename) and NGF::Serialiser::load(filename).
 * =====================================================================================
 */

class Serialiser : public Ogre::Singleton<Serialiser>
{
    protected:
	static std::vector<GameObjectRecord> mRecords;

    public:
	//Save the game. Automatically saves all GameObjects inheriting form 'SerialisableGameObject'. 
        //Calls 'serialise' on all GameObjects to allow them to save their position and rotation 
        //and any extra information.
	static void save(Ogre::String filename);

	//Load the game. Creates GameObjects based on the type contained in the GameObjectRecord, 
        //with the old GameObject's name, properties, and position and rotation saved by the 
        //GameObject. Additional information saved by the GameObject is also restored.
	static void load(Ogre::String filename);

	//--- Internal stuff --------------------------------------------------------------
	
	static void _saveOne(GameObject*);
};

/*
 * =====================================================================================
 *        Class:  GameObjectRecord
 *
 *  Description:  Stores information for one GameObject. It stores the type of the
 *  		  object, the name, and the properties, and extra information returned
 *  		  by the object.
 * =====================================================================================
 */

class GameObjectRecord
{
    protected:
	Ogre::String mType;
	Ogre::String mName;
	std::map<Ogre::String, std::vector<Ogre::String> > mProps;
	std::map<Ogre::String, std::vector<Ogre::String> > mInfo;

	friend class boost::serialization::access;
	friend class Serialiser;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
	    ar & mType;
	    ar & mName;
	    ar & mProps;
	    ar & mInfo;
	}

    public:
	GameObjectRecord()
	    : mType("none")
	{
	}

	GameObjectRecord(Ogre::String type, PropertyList info)
	    : mType(type),
	      mInfo(info)
	{
	}

	PropertyList getInfo() { return PropertyList(mInfo); }
};

} //namespace Serialisation

} //namespace NGF

/*
 * =====================================================================================
 *       Macros:  NGF_SERIALISE_*
 *
 *  Description:  These macros make defining the 'serialise' function easier for
 *  		  Ogre datatypes (Real, Vector3, Quaternion etc.). If you look at the
 *  		  macros below together, you can see a 'serialise' function take form.
 *  		  ;-)
 *
 *  		  Note: all the 'var' parameters take the variable name in the current 
 *  		        context. So, if the name of a member is 'mHealth', write 
 *  		        'mHealth'. Local variables in the 'serialise' function will
 *  		        work too.
 * =====================================================================================
 */

//Begins the serialise function definition.
#define NGF_SERIALISE_BEGIN(classnm)                                                           \
	NGF::Serialisation::GameObjectRecord serialise(bool save, NGF::PropertyList &in)       \
	{                                                                                      \
	    NGF::PropertyList out;                                                             \
	    Ogre::String type = #classnm;

//The position saved this way will be restored as the position sent in the constructor.
#define NGF_SERIALISE_POSITION(pos)                                                            \
	    if (save)                                                                          \
                out.addProperty("NGF_POSITION", Ogre::StringConverter::toString(pos), "")

//The rotation saved this way will be restored as the rotation sent in the constructor.
#define NGF_SERIALISE_ROTATION(rot)                                                            \
            if (save)                                                                          \
                out.addProperty("NGF_ROTATION", Ogre::StringConverter::toString(rot), "")

//For saving strings.
#define NGF_SERIALISE_STRING(var)                                                              \
	    if (save) {                                                                        \
		out.addProperty(#var , var, "");                                               \
	    } else  {                                                                          \
		var = in.getValue(#var, 0, "");                                                \
	    }

//Works for all types that the Ogre::StringConverter::parse* functions support. 'type' must be
//the '*' part of the parse* function used.
#define NGF_SERIALISE_OGRE(type, var)                                                          \
	    if (save) {                                                                        \
		out.addProperty(#var , Ogre::StringConverter::toString(var), "");              \
	    } else  {                                                                          \
		Ogre::String var##str = in.getValue(#var, 0, "n");                             \
		if (var##str != "n")                                                           \
		    var = Ogre::StringConverter::parse##type(var##str);                        \
	    }

//Blocks following these macros would be called on saving and loading respectively.
#define NGF_SERIALISE_ON_SAVE                                                                  \
            if(save)
#define NGF_SERIALISE_ON_LOAD                                                                  \
            if(!save)

//Ends the serialise function definition.
#define NGF_SERIALISE_END                                                                      \
	    return NGF::Serialisation::GameObjectRecord(type , out);                           \
	}

//If you're using NGF::Python, you can save the Python locals ('m_*' variables) with this.
#define NGF_SERIALISE_PYTHON_LOCALS()                                                          \
            if (save) {                                                                        \
                out.addProperty("NGF_PY_LOCALS" , mConnector->dumpLocals(), "");               \
            } else  {                                                                          \
                mConnector->loadLocals(in.getValue("NGF_PY_LOCALS", 0, "'(dp0\n.'"));          \
            }

//Serialises a Bullet RigidBody's state. Uses BtOgre for Vector3/Quaternion conversion. The
//body's position, orientation, linear and angular velocity get saved.
#define NGF_SERIALISE_BULLET_BODY(var)                                                         \
            {                                                                                  \
                Ogre::Vector3 var##pos, var##lVel, var##aVel;                                  \
                Ogre::Quaternion var##rot;                                                     \
                                                                                               \
                NGF_SERIALISE_ON_SAVE                                                          \
                {                                                                              \
                    btTransform trans = var->getWorldTransform();                              \
                                                                                               \
                    var##pos = BtOgre::Convert::toOgre(trans.getOrigin());                     \
                    var##rot = BtOgre::Convert::toOgre(trans.getRotation());                   \
                    var##lVel = BtOgre::Convert::toOgre(var->getLinearVelocity());             \
                    var##aVel = BtOgre::Convert::toOgre(var->getAngularVelocity());            \
                }                                                                              \
                                                                                               \
                NGF_SERIALISE_OGRE(Vector3, var##pos);                                         \
                NGF_SERIALISE_OGRE(Quaternion, var##rot);                                      \
                NGF_SERIALISE_OGRE(Vector3, var##lVel);                                        \
                NGF_SERIALISE_OGRE(Vector3, var##aVel);                                        \
                                                                                               \
                NGF_SERIALISE_ON_LOAD                                                          \
                {                                                                              \
                    btVector3 pos = BtOgre::Convert::toBullet(var##pos);                       \
                    btQuaternion rot = BtOgre::Convert::toBullet(var##rot);                    \
                                                                                               \
                    var->setWorldTransform(btTransform(rot, pos));                             \
                    var->setLinearVelocity(BtOgre::Convert::toBullet(var##lVel));              \
                    var->setAngularVelocity(BtOgre::Convert::toBullet(var##aVel));             \
                }                                                                              \
            }                                                                                      

#endif //#ifndef __NGF_SERIALISATION_H__
