/*
 * Copyright © 2012, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All rights reserved.
 * 
 * The NASA Tensegrity Robotics Toolkit (NTRT) v1 platform is licensed
 * under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
*/

/**
 * @file MuscleAnchor.cpp
 * @brief Definitions of class MuscleAnchor.
 * $Id$
 */
 
#include "muscleAnchor.h"

// The BulletPhysics library
#include "BulletDynamics/Dynamics/btRigidBody.h"

// The C++ Standard Library
#include <iostream>
 
muscleAnchor::muscleAnchor(btRigidBody * body,
               btVector3 worldPos,
               btVector3 cn,
               bool perm,
               bool slide,
               btPersistentManifold* m) :
  attachedBody(body),
  // Find relative position
  // This should give relative position in a default orientation.
  attachedRelativeOriginalPosition(attachedBody->getWorldTransform().inverse() *
                   worldPos),
  contactNormal(attachedBody->getWorldTransform().inverse().getBasis() * cn),
  height(999.0),
  permanent(perm),
  sliding(slide),
  manifold(m)
{
}

muscleAnchor::~muscleAnchor()
{
    // World will delete attached body
    
}

// This returns current position relative to the rigidbody.
btVector3 muscleAnchor::getRelativePosition() const
{
    const btTransform tr = attachedBody->getWorldTransform();
    const btVector3 worldPos = tr * attachedRelativeOriginalPosition;
    return worldPos-this->attachedBody->getCenterOfMassPosition();
}

btVector3 muscleAnchor::getWorldPosition() const
{
    const btTransform tr = attachedBody->getWorldTransform();
    return tr * attachedRelativeOriginalPosition;
}

void muscleAnchor::setWorldPosition(btVector3& newPos)
{
	if (sliding)
	{
		attachedRelativeOriginalPosition = attachedBody->getWorldTransform().inverse() *
                   newPos;
	}
	else
	{
		std::cerr << "Tried to update a static anchor" << std::endl;
	}
}

btVector3 muscleAnchor::getContactNormal() const
{
	const btTransform tr = attachedBody->getWorldTransform();
    btVector3 newNormal = (tr.getBasis() * contactNormal);
    newNormal = newNormal.length() > 0.0 ? newNormal.normalize() : btVector3(0.0, 0.0, 0.0);
    //assert(newNormal.length() == 1.0);
    return newNormal;
}
