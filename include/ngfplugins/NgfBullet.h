/*
 * =====================================================================================
 *
 *       Filename:  NgfBullet.h
 *
 *    Description:  NGF-Bullet connection. Allows BulletGameObjects to be notified of
 *                  Bullet collision events.
 *
 *        Version:  1.0
 *        Created:  02/27/2009 11:40:53 PM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#ifndef __NGF_BULLET_H__
#define __NGF_BULLET_H__

#include <Ogre.h> //Change this to only include specific headers when done.
#include <Ngf.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionDispatch/btManifoldResult.h>
#include <BulletCollision/NarrowPhaseCollision/btManifoldPoint.h>

namespace NGF { namespace Bullet {

class BulletGameObject;

/*
 * =====================================================================================
 * Free Functions
 * =====================================================================================
 */

//Register a BulletGameObject for collision. It is associated with the given btCollisionObject.
//The given BulletGameObject will be notified of any collision events involving the given
//btCollisionObject. You can also use 'setBulletObject' from within a BulletGameObject.
void registerForCollision(BulletGameObject *object, btCollisionObject *physic);

//Returns the BulletGameObject associated with the given btCollisionObject. An association is
//made with the 'registerForCollision' function.
BulletGameObject* getObjectFromPhysicsObject(btCollisionObject *physic);

/*
 * =====================================================================================
 *        Class:  BulletGameObject
 *
 *  Description:  An interface for GameObject that can receive 'collide' events.
 * =====================================================================================
 */

class BulletGameObject : virtual public GameObject
{
    public:
        BulletGameObject()
            : GameObject(Ogre::Vector3(), Ogre::Quaternion(), ID(), PropertyList(), "")
        {
        }

        //Called on collision with a physics object.
        //
        //other: The GameObject collided with.
        //otherPhysicsObject: The btCollisionObject collided with.
        //contact: The mainfold contact point.
        virtual void collide(GameObject *other, btCollisionObject *otherPhysicsObject, btManifoldPoint &contact) { }

        //Register self for collision event. We're associated with the given GameObject.
        inline void setBulletObject(btCollisionObject *physic) { registerForCollision(this, physic); }
};

} //namespace Bullet

} //namespace NGF

#endif //ifndef __NGF_BULLET_H__

//=-= .CPP File =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=

namespace NGF { namespace Bullet {

    //Forward declaration of the callback.
    static bool _contactAdded(btManifoldPoint& cp, const btCollisionObject* colObj0, 
            int partId0, int index0, const btCollisionObject* colObj1, int partId1, int index1);

/*
 * =====================================================================================
 * Free Functions
 * =====================================================================================
 */

    void registerForCollision(BulletGameObject *object, btCollisionObject *physic)
    {
        gContactAddedCallback = _contactAdded;
        physic->setUserPointer(object);
        physic->setCollisionFlags(physic->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    }
    //----------------------------------------------------------------------------------   
    BulletGameObject* getObjectFromPhysicsObject(btCollisionObject *physic)
    {
        void *usr = physic->getUserPointer();

        if (!usr)
            return 0;

        return (BulletGameObject*) usr;
    }
    //----------------------------------------------------------------------------------   
    bool _contactAdded(btManifoldPoint& cp, const btCollisionObject* colObj0, 
            int partId0, int index0, const btCollisionObject* colObj1, int partId1, int index1)
    {
        //Get the OgreBullet objects.
        btCollisionObject *phy1 = const_cast<btCollisionObject *>(colObj0);
        btCollisionObject *phy2 = const_cast<btCollisionObject *>(colObj1);

        //Get the NGF GameObjects, and call the relevant methods.
        BulletGameObject *obj1 = getObjectFromPhysicsObject(phy1);
        BulletGameObject *obj2 = getObjectFromPhysicsObject(phy2);

        if (obj1 && obj2)
        {
            obj1->collide(obj2, phy2, cp);
            obj2->collide(obj1, phy1, cp);
        }
        else if (obj1)
        {
            obj1->collide(0, phy2, cp);
        }
        else if (obj2)
        {
            obj2->collide(0, phy1, cp);
        }

        return true;
    }

} //namespace Bullet

} //namespace NGF

