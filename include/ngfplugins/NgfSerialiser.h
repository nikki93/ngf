/*
 * =====================================================================================
 *
 *       Filename:  NgfSerialiser.h
 *
 *    Description:  GameObject saving and loading mechanism.
 *
 *        Version:  1.0
 *        Created:  03/22/2009 09:47:54 AM
 *       Revision:  none
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#ifndef __NGF_SERIALISER_H__
#define __NGF_SERIALISER_H__

#include <Ogre.h> //Change this to only include specific headers when done.
#include <Ngf.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include <fstream>

namespace NGF {

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
	static int mSig[];

    public:
	//Save the game. Automatically saves all GameObjects inheriting form 'SerialisableGameObject'.
	static void save(Ogre::String filename, Ogre::String password = "42-1337")
	{
		mRecords.clear();
		GameObjectManager::getSingleton().forEachGameObject(_saveOne);

		std::ofstream ofs(filename.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa << mRecords;
	}

	//Load the game. Creates GameObjects based on the type contained in the GameObjectRecord, and
	//passes the object the relevant information through it's 'load' method. The GameObject also
	//receives the PropertyList when it is created.
	static void load(Ogre::String filename, Ogre::String password = "42-1337")
	{
		mRecords.clear();
		{
			std::ifstream ifs(filename.c_str());
			boost::archive::text_iarchive ia(ifs);
			ia >> mRecords;
		}

		for (std::vector<GameObjectRecord>::iterator iter = mRecords.begin(); iter!= mRecords.end(); ++iter)
		{
			GameObjectRecord rec = *iter;

			PropertyList info = rec.getInfo();

			Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(info.getValue("NGF_POSITION", 0, "0 0 0"));
			//props.erase("NGF_POSITION");
			Ogre::Quaternion rot = Ogre::StringConverter::parseQuaternion(info.getValue("NGF_ROTATION", 0, "1 0 0 0"));
			//props.erase("NGF_ROTATION");

			SerialisableGameObject *obj = dynamic_cast<SerialisableGameObject*>
				(GameObjectManager::getSingleton().createObject(rec.mType, pos, rot, rec.mProps, rec.mName));

			if (obj)
				obj->serialise(false, info);
		} 
	}

	//--- Internal stuff --------------------------------------------------------------
	
	static void _saveOne(GameObject *o)
	{
		SerialisableGameObject *obj = dynamic_cast<SerialisableGameObject*>(o);

		if (obj)
		{
		        PropertyList dummy;
			GameObjectRecord rec = obj->serialise(true, dummy);
			rec.mName = obj->getName();
			rec.mProps = obj->getProperties();
			mRecords.push_back(rec);
		}
	}
};

std::vector<GameObjectRecord> Serialiser::mRecords = std::vector<GameObjectRecord>();

}

/*
 * =====================================================================================
 *       Macros:  NGF_SERIALISE_*
 *
 *  Description:  These macros make defining the 'serialise' function easier for
 *  		  Ogre datatypes (Real, Vector3, Quaternion etc.). If you look at the
 *  		  macros below together, you can see a 'serialise' function take form.
 *  		  ;-)
 * =====================================================================================
 */

#define NGF_SERIALISE_BEGIN(classnm)                                                           \
	NGF::GameObjectRecord serialise(bool save, NGF::PropertyList &in)                      \
	{                                                                                      \
	    NGF::PropertyList out;                                                             \
	    Ogre::String type = #classnm;

#define NGF_SERIALISE_POSITION(pos)                                                            \
	    if (save)                                                                          \
                out.addProperty("NGF_POSITION", Ogre::StringConverter::toString(pos), "")

#define NGF_SERIALISE_ROTATION(rot)                                                            \
            if (save)                                                                          \
                out.addProperty("NGF_ROTATION", Ogre::StringConverter::toString(rot), "")

#define NGF_SERIALISE_STRING(var)                                                              \
	    if (save) {                                                                        \
		out.addProperty( #var , var, "");                                              \
	    } else  {                                                                          \
		var = in.getValue( #var, 0, "");                                               \
	    }

#define NGF_SERIALISE_OGRE(type, var)                                                          \
	    if (save) {                                                                        \
		out.addProperty( #var , Ogre::StringConverter::toString(var), "");             \
	    } else  {                                                                          \
		Ogre::String var ## str = in.getValue( #var, 0, "n");                          \
		if ( var ## str != "n")                                                        \
		    var = Ogre::StringConverter::parse ## type ( var ## str );                 \
	    }

#define NGF_SERIALISE_ON_SAVE if(save)
#define NGF_SERIALISE_ON_LOAD if(!save)

#define NGF_SERIALISE_END                                                                      \
	    return NGF::GameObjectRecord( type , out);                                         \
	}


#endif //#ifndef __NGF_SERIALISER_H__
