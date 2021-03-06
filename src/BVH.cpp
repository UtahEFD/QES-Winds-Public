/*
 * QES-Winds
 *
 * Copyright (c) 2021 University of Utah
 * Copyright (c) 2021 University of Minnesota Duluth
 *
 * Copyright (c) 2021 Behnam Bozorgmehr
 * Copyright (c) 2021 Jeremy A. Gibbs
 * Copyright (c) 2021 Fabien Margairaz
 * Copyright (c) 2021 Eric R. Pardyjak
 * Copyright (c) 2021 Zachary Patterson
 * Copyright (c) 2021 Rob Stoll
 * Copyright (c) 2021 Pete Willemsen
 *
 * This file is part of QES-Winds
 *
 * GPL-3.0 License
 *
 * QES-Winds is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * QES-Winds is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QES-Winds. If not, see <https://www.gnu.org/licenses/>.
 *
 */


#include "BVH.h"

void BVH::mergeSort(std::vector<BVH*>& list, const int type)
{
   if (list.size() <= 1)
      return;

   std::vector<BVH *> l, r;
   int midPoint = list.size() / 2;
   for (int i = 0; i < midPoint; i++)
      l.push_back(list[i]);
   for (int i = midPoint; i < list.size(); i++)
      r.push_back(list[i]);

   mergeSort(l, type);
   mergeSort(r, type);

   int lSize = l.size(), rSize = r.size();
   int j = 0, k = 0;
   for (auto i = 0u; i < list.size(); i++)
   {
      if ( j == lSize)
         list[i] = r[k++];
      else if ( k == rSize)
         list[i] = l[j++];
      else
      {
         float lMid, rMid;
         if (type == 0)
         {
            lMid = (l[j]->xmin + l[j]->xmax) / 2.0f;
            rMid = (r[k]->xmin + r[k]->xmax) / 2.0f;
         }
         else
         {
            lMid = (l[j]->ymin + l[j]->ymax) / 2.0f;
            rMid = (r[k]->ymin + r[k]->ymax) / 2.0f;
         }
         list[i] = (lMid < rMid ? l[j++] : r[k++]);
      }
   }

}

BVH::BVH(BVH* l, BVH* r)
{
   leftBox = l;
   rightBox = r;
   if (r != 0)
   {
      xmin = GETMIN(l->xmin, r->xmin);
      xmax = GETMAX(l->xmax, r->xmax);
      ymin = GETMIN(l->ymin, r->ymin);
      ymax = GETMAX(l->ymax, r->ymax);
      zmin = GETMIN(l->zmin, r->zmin);
      zmax = GETMAX(l->zmax, r->zmax);
   }
   else
   {
      xmin = l->xmin;
      xmax = l->xmax;
      ymin = l->ymin;
      ymax = l->ymax;
      zmin = l->zmin;
      zmax = l->zmax;
   }

   isLeaf = false;
   tri = 0;
}

BVH::BVH(Triangle* t)
{
   tri = t;
   isLeaf = true;

   t->getBoundaries(xmin, xmax, ymin, ymax, zmin, zmax);

   leftBox = rightBox = 0;
}

BVH::BVH(std::vector<BVH *> m, int height)
{
   isLeaf = false;
   if (m.size() == 1)
   {
      leftBox = m[0];
      rightBox = 0;

      xmin = leftBox->xmin;
      xmax = leftBox->xmax;
      ymin = leftBox->ymin;
      ymax = leftBox->ymax;
      zmin = leftBox->zmin;
      zmax = leftBox->zmax;

      tri = 0;
      return;
   }
   else if (m.size() == 2)
   {
      leftBox = m[0];
      rightBox = m[1];

      xmin = GETMIN(leftBox->xmin, rightBox->xmin);
      xmax = GETMAX(leftBox->xmax, rightBox->xmax);
      ymin = GETMIN(leftBox->ymin, rightBox->ymin);
      ymax = GETMAX(leftBox->ymax, rightBox->ymax);
      zmin = GETMIN(leftBox->zmin, rightBox->zmin);
      zmax = GETMAX(leftBox->zmax, rightBox->zmax);

      tri = 0;
      return;
   }

   mergeSort(m, height % 2);


   std::vector<BVH *> l, r;
   int midPoint = m.size() / 2;
   for (int i = 0; i < midPoint; i++)
      l.push_back(m[i]);
   for (int i = midPoint; i < m.size(); i++)
      r.push_back(m[i]);

   leftBox = new BVH(l, height + 1);
   rightBox = new BVH(r, height + 1);

   xmin = GETMIN(leftBox->xmin, rightBox->xmin);
   xmax = GETMAX(leftBox->xmax, rightBox->xmax);
   ymin = GETMIN(leftBox->ymin, rightBox->ymin);
   ymax = GETMAX(leftBox->ymax, rightBox->ymax);
   zmin = GETMIN(leftBox->zmin, rightBox->zmin);
   zmax = GETMAX(leftBox->zmax, rightBox->zmax);

   tri = 0;
}


BVH* BVH::createBVH(const std::vector<Triangle*> tris)
{
   std::vector<BVH *> boxes;

   for (int i = 0; i < tris.size(); i++)
   {
      BVH* b = new BVH(tris[i]);
      boxes.push_back(b);
   }

   BVH* root = new BVH(boxes, 0);
   return root;
}

float BVH::heightToTri(float x, float y)
{
   if (isLeaf)
   {
      return tri->getHeightTo(x, y);
   }
   else
   {
      float toL = -1.0f, toR = -1.0f;

      if (leftBox && leftBox->xmin <= x && leftBox->xmax >= x &&
          leftBox->ymin <= y && leftBox->ymax >= y)
         toL = leftBox->heightToTri(x, y);

      if (rightBox && rightBox->xmin <= x && rightBox->xmax >= x &&
          rightBox->ymin <= y && rightBox->ymax >= y)
         toR = rightBox->heightToTri(x, y);

      return toL > toR ? toL : toR;
   }
}
