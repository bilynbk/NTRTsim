/*
 * Copyright © 2015, United States Government, as represented by the
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
 * @file ScarrArmModel.cpp
 * @brief Contains the implementation of class ScarrArmModel. 
 * Measurements are scaled to average adult male
 * $Id$
 */

// This module
#include "ScarrArmModel.h"
// This library
#include "core/tgBasicActuator.h"
#include "core/tgRod.h"
#include "core/abstractMarker.h"
#include "tgcreator/tgBuildSpec.h"
#include "tgcreator/tgBasicActuatorInfo.h"
#include "tgcreator/tgRodInfo.h"
#include "tgcreator/tgStructure.h"
#include "tgcreator/tgStructureInfo.h"
// The Bullet Physics library
#include "LinearMath/btVector3.h"
// The C++ Standard Library
#include <stdexcept>

namespace
{
    // see tgBaseString.h for a descripton of some of these rod parameters
    // (specifically, those related to the motor moving the strings.)
    // NOTE that any parameter that depends on units of length will scale
    // with the current gravity scaling. E.g., with gravity as 98.1,
    // the length units below are in decimeters.

    // Note: This current model of the SUPERball rod is 1.5m long by 3 cm radius,
    // which is 0.00424 m^3.
    // For SUPERball v1.5, mass = 3.5kg per strut, which comes out to 
    // 0.825 kg / (decimeter^3).

    // similarly, frictional parameters are for the tgRod objects.
    const struct Config
    {
        double density;
        double radius;
        double stiffness;
        double damping;
        double rod_length;
        double rod_space;    
        double friction;
        double rollFriction;
        double restitution;
        double pretension;
        bool   history;  
        double maxTens;
        double targetVelocity;
    } c =
   {
     0.825,    // density (kg / length^3)
     0.31,     // radius (length)
     3000.0,   // stiffness (kg / sec^2)
     200.0,    // damping (kg / sec)
     15.0,     // rod_length (length)
     7.5,      // rod_space (length)
     1.0,      // friction (unitless)
     0.01,     // rollFriction (unitless)
     0.2,      // restitution (?)
     0,        // pretension (force)
     false,    // history (boolean)
     100000,   // maxTens
     10000    // targetVelocity
#if (0)
     20000     // maxAcc
#endif
     // Use the below values for earlier versions of simulation.
     // 1.006,    
     // 0.31,     
     // 300000.0, 
     // 3000.0,   
     // 15.0,     
     // 7.5,      
  };
} // namespace

/*
 * helper arrays for node and rod numbering schema
 */
/*returns the number of the rod for a given node */
//const int rodNumbersPerNode[11]={0,1,2,3,3,4,0,1,2,5,5,4,6};

/*returns the node that is at the other end of the given node */
//const int otherEndOfTheRod[11]={6,7,8,4,3,11,0,1,2,10,9,5,12};

/*returns the node that is at the parallel rod
 * and at the same end of the given node
 */
//const int parallelNode[11]={1,0,5,9,10,2,7,6,11,3,4,8,12};

ScarrArmModel::ScarrArmModel() : tgModel() 
{
}

ScarrArmModel::~ScarrArmModel()
{
}

void ScarrArmModel::addNodes(tgStructure& s)
{
    const double scale = 0.1;
    const size_t nNodes = 11;
    
    // Average Adult Male Measurements with scale
    // Lengths are in mm
    const double a = 22 * scale; //2.2cm
    const double b = 265 * scale;
    const double c = 334 * scale;
    const double d = 66 * scale;
    const double e = 246 * scale;
    const double f = 25 * scale;
    const double g = 17 * scale;
    const double sigma = 1; //TODO: tinker
    const double ulna_diameter = g*2; //TODO: tinker
    const double x = a/2;
    const double z = c/2;

    nodePositions.push_back(btVector3(a/2, 0, 0));
    nodePositions.push_back(btVector3(0, 0, -g));
    nodePositions.push_back(btVector3(-a/2, 0, 0));
    nodePositions.push_back(btVector3(0, f, 0));
    nodePositions.push_back(btVector3(0, -f, 0));
    nodePositions.push_back(btVector3(0, 0, d));
    nodePositions.push_back(btVector3(0, 0, c));
    nodePositions.push_back(btVector3(x, 0, z));
    nodePositions.push_back(btVector3(b+a/2, 0, 0));
    nodePositions.push_back(btVector3(a/2 + sigma, 0, -ulna_diameter));
    nodePositions.push_back(btVector3(a/2 + sigma + e, 0, -ulna_diameter));

    for(size_t i=0;i<nNodes;i++) {
		s.addNode(nodePositions[i][0],nodePositions[i][1],nodePositions[i][2]);
    }
}

void ScarrArmModel::addRods(tgStructure& s)
{
    s.addPair(5,  6,  "massless rod");
    s.addPair(5,  6,  "rod");
    s.addPair(3,  5,  "rod");
    s.addPair(4,  5,  "rod");

    s.addPair(0,  1,  "rod");
    s.addPair(0,  8,  "rod");
    s.addPair(1,  2,  "rod");

    s.addPair(9, 10,  "rod"); 
}

/*
void ScarrArmModel::addMarkers(tgStructure &s)
{
    std::vector<tgRod *> rods=find<tgRod>("rod");

	for(int i=0;i<10;i++)
	{
		const btRigidBody* bt = rods[rodNumbersPerNode[i]]->getPRigidBody();
		btTransform inverseTransform = bt->getWorldTransform().inverse();
		btVector3 pos = inverseTransform * (nodePositions[i]);
		abstractMarker tmp=abstractMarker(bt,pos,btVector3(0.08*i,1.0 - 0.08*i,.0),i);
		this->addMarker(tmp);
	}
}
*/

void ScarrArmModel::addMuscles(tgStructure& s)
{
    s.addPair(0, 3, "muscle");
    s.addPair(0, 4, "muscle");
    s.addPair(1, 3, "muscle");
    s.addPair(1, 4, "muscle");
    s.addPair(2, 3, "muscle");
    s.addPair(2, 4, "muscle");
    s.addPair(3, 9, "muscle");
    s.addPair(4, 9, "muscle");
    //s.addPair(5, 7, "muscle");
    //s.addPair(6, 7, "muscle");
    //s.addPair(7, 8, "muscle"); 
    s.addPair(6, 8, "muscle"); 
    s.addPair(8, 10, "muscle"); 
}

void ScarrArmModel::setup(tgWorld& world)
{

    const tgRod::Config rodConfig(c.radius, c.density, c.friction, c.rollFriction, c.restitution);
    const tgRod::Config rodConfigMassless(c.radius, 0.001/*c.density*/, c.friction, c.rollFriction, c.restitution); //TODO: Fix
    /// @todo acceleration constraint was removed on 12/10/14 Replace with tgKinematicActuator as appropreate
    tgBasicActuator::Config muscleConfig(c.stiffness, c.damping, c.pretension, c.history, c.maxTens, c.targetVelocity);
            
    // Start creating the structure
    tgStructure s;
    addNodes(s);
    addRods(s);
    addMuscles(s);

//    // Add a rotation. This is needed if the ground slopes too much, otherwise glitches put a rod below the ground.
//    btVector3 rotationPoint = btVector3(0, 0, 0); // origin
//    btVector3 rotationAxis = btVector3(0, 1, 0);  // y-axis
//    double rotationAngle = M_PI/2;
//    s.addRotation(rotationPoint, rotationAxis, rotationAngle);
//    s.move(btVector3(0,30,0));

    // Create the build spec that uses tags to turn the structure into a real model
    tgBuildSpec spec;
    spec.addBuilder("massless rod", new tgRodInfo(rodConfigMassless));
    spec.addBuilder("rod", new tgRodInfo(rodConfig));
    spec.addBuilder("muscle", new tgBasicActuatorInfo(muscleConfig));
    
    // Create your structureInfo
    tgStructureInfo structureInfo(s, spec);

    // Use the structureInfo to build ourselves
    structureInfo.buildInto(*this, world);

    // We could now use tgCast::filter or similar to pull out the
    // models (e.g. muscles) that we want to control. 
    allMuscles = tgCast::filter<tgModel, tgBasicActuator> (getDescendants());

    // call the onSetup methods of all observed things e.g. controllers
    notifySetup();

    // Actually setup the children
    tgModel::setup(world);

    //map the rods and add the markers to them
    //addMarkers(s);

    btVector3 location(0,10.0,0);
    //btVector3 rotation(M_PI/2,0,0);
    btVector3 rotation(0,0,0);
  	btVector3 speed(0,0,0);
    this->moveModel(location,rotation,speed);
}

void ScarrArmModel::step(double dt)
{
    // Precondition
    if (dt <= 0.0)
    {
        throw std::invalid_argument("dt is not positive");
    }
    else
    {
        // Notify observers (controllers) of the step so that they can take action
        notifyStep(dt);
        tgModel::step(dt);  // Step any children
    }
}

void ScarrArmModel::onVisit(tgModelVisitor& r)
{
    tgModel::onVisit(r);
}

const std::vector<tgBasicActuator*>& ScarrArmModel::getAllMuscles() const
{
    return allMuscles;
}
    
void ScarrArmModel::teardown()
{
    tgModel::teardown();
}

void ScarrArmModel::moveModel(btVector3 positionVector,btVector3 rotationVector,btVector3 speedVector)
{
    std::vector<tgRod *> rods=find<tgRod>("rod");

	btQuaternion initialRotationQuat;
	initialRotationQuat.setEuler(rotationVector[0],rotationVector[1],rotationVector[2]);
	btTransform initialTransform;
	initialTransform.setIdentity();
	initialTransform.setRotation(initialRotationQuat);
	initialTransform.setOrigin(positionVector);
	for(size_t i=0;i<rods.size();i++) {
			rods[i]->getPRigidBody()->setLinearVelocity(speedVector);
			rods[i]->getPRigidBody()->setWorldTransform(initialTransform * rods[i]->getPRigidBody()->getWorldTransform());
	}
}
