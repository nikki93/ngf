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

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include <fstream>

namespace NGF {

/*
 * =====================================================================================
 *        Class:  GameObjectRecord
 *
 *  Description:  Stores information for one GameObject. It stores the type of the
 *  		  object, the name, and the properties.
 * =====================================================================================
 */

class GameObjectRecord
{
    protected:
	Ogre::String mType;
	Ogre::String mName;
	std::map<Ogre::String, std::vector<Ogre::String> > mProperties;

	friend class boost::serialization::access;
	friend class Serialiser;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
	    ar & mType;
	    ar & mName;
	    ar & mProperties;
	}

    public:
	GameObjectRecord()
	    : mType("none"),
	      mProperties(PropertyList::create("none", "none"))
	{
	}

	GameObjectRecord(Ogre::String type, PropertyList props)
	    : mType(type),
	      mProperties(props)
	{
	}

	Ogre::String getType() { return mType; }
	PropertyList getProperties() { return PropertyList(mProperties); }
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
	    SerialisableGameObject(Ogre::Vector3 pos, Ogre::Quaternion rot, ID id, PropertyList props, Ogre::String name)
		    : GameObject(pos, rot, id , props, name)
	    {
	    }

	    //Return a GameObject record containing information about the GameObject. You must fill in
	    //the type and properties, then name gets filled in automatically.
	    virtual GameObjectRecord save() { return GameObjectRecord(); }

	    //Load information from the given GameObjectRecord. It is the same one returned from a 'save'
	    //call, stored in a file and reloaded.
	    virtual void load(GameObjectRecord &record) {}
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
	static void save(Ogre::String filename)
	{
		mRecords.clear();
		GameObjectManager::getSingleton().forEachGameObject(_saveOne);

		std::ofstream ofs(filename.c_str());
		boost::archive::binary_oarchive oa(ofs);
		oa << mRecords;
		mRecords.clear();
	}

	//Load the game. Creates GameObjects based on the type contained in the GameObjectRecord, and
	//passes the object the relevant information through it's 'load' method. The GameObject also
	//receives the PropertyList when it is created.
	static void load(Ogre::String filename)
	{
		mRecords.clear();
		{
			std::ifstream ifs(filename.c_str());
			boost::archive::binary_iarchive ia(ifs);
			ia >> mRecords;
		}

		for (std::vector<GameObjectRecord>::iterator iter = mRecords.begin(); iter!= mRecords.end(); ++iter)
		{
			GameObjectRecord rec = *iter;

			Ogre::String type = rec.getType();
			PropertyList props = rec.getProperties();

			Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(props.getValue("NGF_POSITION", 0, "0 0 0"));
			Ogre::Quaternion rot = Ogre::StringConverter::parseQuaternion(props.getValue("NGF_ROTATION", 0, "1 0 0 0"));

			SerialisableGameObject *obj = dynamic_cast<SerialisableGameObject*>
				(GameObjectManager::getSingleton().createObject(type, pos, rot, props.addProperty("NGF_LOADED", "1"), rec.mName));

			if (obj)
				obj->load(rec);
		} 
		mRecords.clear();
	}

	//--- Internal stuff --------------------------------------------------------------
	
	static void _saveOne(GameObject *o)
	{
		SerialisableGameObject *obj = dynamic_cast<SerialisableGameObject*>(o);

		if (obj)
		{
			GameObjectRecord rec = obj->save();
			rec.mName = obj->getName();
			mRecords.push_back(rec);
		}
	}
};

std::vector<GameObjectRecord> Serialiser::mRecords = std::vector<GameObjectRecord>();

}

#endif //#ifndef __NGF_SERIALISER_H__
