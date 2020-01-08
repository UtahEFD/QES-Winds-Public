#pragma once

/*
 *Used to store information about intersections
 *Can add other information about the BVH node it hits as needed 
 */

#ifndef HR_H
#define HR_H

#include "Vector3.h"
#include <limits>

class HitRecord{
  public:
   bool isHit;
   void* hitNode;          //reference to BVH node that was hit
   float hitDist;          //distance from ray origin to hit point 
   float t;                
   Vector3<float> endpt;   //the intersection point   
   
   HitRecord();
   HitRecord(void* hitNode, bool isHit);
   HitRecord(void* hitNode, bool isHit, float hitDist);
   
   void* getHitNode();
   float getHitDist();
   bool getIsHit();
};

#endif
