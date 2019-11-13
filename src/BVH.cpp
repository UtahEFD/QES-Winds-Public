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



HitRecord* BVH::rayTriangleIntersect(Ray* ray){
   HitRecord hitrec = NULL;
   if(!isleaf){
      std::cout<<"Not an intersectable triangle."<<endl;
   }else{
      float beta, gamma, t, M, a,b,c,d,e,f,g,h,i,j,k,l;

      a = (tri.a)[0] - (tri.a)[1];       d = (tri.a)[0] - (tri.a)[2];   g = ray.getDirection()[0];
      b = (tri.b)[0] - (tri.b)[1];       e = (tri.b)[0] - (tri.b)[2];   h = ray.getDirection()[1];
      c = (tri.c)[0] - (tri.c)[1];       f = (tri.c)[0] - (tri.c)[2];   i = ray.getDirection()[2];

      j = (tri.a)[0] - ray.getOriginX();
      k = (tri.b)[0] - ray.getOriginY();
      l = (tri.c)[0] - ray.getOriginZ();

      float EIHF = (e*i) - (h*f);
      float GFDI = (g*f) - (d*i);
      float DHEG = (d*h) - (e*g);
      float JCAL = (j*c) - (a*l);
      float BLKC = (b*l) - (k*c);
      float AKJB = (a*k) - (j*b);

      M = (a*EIHF) + (b*GFDI) + (c*DHEG);

      beta = ((j*EIHF) + (k*GFDI) + (l*DHEG))/M;
      gamma = ((i*AKJB) + (h*JCAL) + (g*BLKC))/M;
      t = ((f*AKJB) + (e*JCAL) + d(BLKC))/M;

      if(t < c || t > f || gamma < 0 || gamma > 1 || beta < 0 || beta > (1- gamma)){
         std::cout<<"No intersection found."<<endl;
      }else{
         hitrec = new HitRecord(tri, t);
      }

   }

   return &hitrec;
}


HitRecord* BVH::rayBoxIntersect(Ray* ray){
   HitRecord* hitRec;

   if(isleaf){
      std::cout<<"This is a leaf node."<<endl;
   }else{
      float rOriginX = ray.getOriginX();
      float rOriginY = ray.getOriginY();
      float rOriginZ = ray.getOriginZ();
      Vector3<float> dir = ray.getDirection();

      float tMinX, tMaxX, tMinY, tMaxY, tMinZ, tMaxZ;

      float ax = 1/dir[0];
      if(aX >= 0){
         tMinX = aX*(xmin - rOriginX);
         tMaxX = aX*(xmax - rOriginX);
      }else{
         tMinX = aX*(xmax - rOriginX);
         tMaxX = aX*(xmin - rOriginX);
      }

      float aY = 1/dir[1];
      if(aY >= 0){
         tMinY = aY*(ymin - rOriginY);
         tMaxY = aY*(ymax - rOriginY);
      }else{
         tMinY = aY*(ymax - rOriginY);
         tMaxY = aY*(ymin - rOriginY);
      }

      float aZ = 1/dir[2];
      if(aZ >= 0){
         tMinZ = aZ*(zmin - rOriginZ);
         tMaxZ = aZ*(zmax - rOriginZ);
      }else{
         tMinZ = aZ*(zmax - rOriginZ);
         tMaxZ = aZ*(zmin - rOriginZ);
      }

      //check fail conditions
      if(tMinX > tMaxY || tMinX > tMaxZ || tMinY > tMaxX || tMinY >
         tMaxZ || tMinZ > tMaxX || tMinZ > tMaxY){
         hitRec = NULL;
      }else{
         magnitude = std::sqrt(pow((tMaxX - tMinX),2)*pow((tMaxY - yMinY),2)*pow((tMaxZ - tMinZ),2));
         hitRec = new HitRecord(this, magnitude);
      }

   }
   return hitsBox;
}
