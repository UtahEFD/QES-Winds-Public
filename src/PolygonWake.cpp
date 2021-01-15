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


#include "PolyBuilding.h"

// These take care of the circular reference
#include "WINDSInputData.h"
#include "WINDSGeneralData.h"



/**
*
* This function applies wake behind the building parameterization to buildings defined as polygons.
* The parameterization has two parts: near wake and far wake. This function reads in building features
* like nodes, building height and base height and uses features of the building defined in the class
* constructor ans setCellsFlag function. It defines cells in each wake area and applies the approperiate
* parameterization to them.
*
*/
void PolyBuilding::polygonWake (const WINDSInputData* WID, WINDSGeneralData* WGD, int building_id)
{

  std::vector<float> Lr_face, Lr_node;
  std::vector<int> perpendicular_flag;
  Lr_face.resize (polygonVertices.size(), -1.0);       // Length of wake for each face
  Lr_node.resize (polygonVertices.size(), 0.0);       // Length of wake for each node
  perpendicular_flag.resize (polygonVertices.size(), 0);
  upwind_rel_dir.resize (polygonVertices.size(), 0.0);      // Upwind reletive direction for each face
  float z_build;                  // z value of each building point from its base height
  float yc, xc;
  float Lr_local, Lr_local_u, Lr_local_v, Lr_local_w;   // Local length of the wake for each velocity component
  float x_wall, x_wall_u, x_wall_v, x_wall_w;
  float y_norm, canyon_factor;
  int x_id_min;

  float Lr_ave;                         // Average length of Lr
  float total_seg_length;               // Length of each edge
  int index_previous, index_next;       // Indices of previous and next nodes
  int stop_id = 0;
  int kk;
  float tol;
  float farwake_exp = 1.5;
  float farwake_factor = 3;
  float epsilon = 10e-10;
  int u_wake_flag, v_wake_flag, w_wake_flag;
  int i_u, j_u, i_v, j_v, i_w, j_w;          // i and j indices for x, y and z directions
  float xp, yp;
  float xu, yu, xv, yv, xw, yw;
  float dn_u, dn_v, dn_w;             // Length of cavity zone
  float farwake_vel;
  std::vector<double> u_temp, v_temp;
  u_temp.resize (WGD->nx*WGD->ny, 0.0);
  v_temp.resize (WGD->nx*WGD->ny, 0.0);
  std::vector<double> u0_modified, v0_modified;
  std::vector<int> u0_mod_id, v0_mod_id;
  float R_scale, R_cx, vd, hd, shell_height;
  int k_bottom, k_top;

  int index_building_face = i_building_cent + j_building_cent*WGD->nx + (k_end)*WGD->nx*WGD->ny;
  u0_h = WGD->u0[index_building_face];         // u velocity at the height of building at the centroid
  v0_h = WGD->v0[index_building_face];         // v velocity at the height of building at the centroid

  // Wind direction of initial velocity at the height of building at the centroid
  upwind_dir = atan2(v0_h,u0_h);

  xi.resize (polygonVertices.size(),0.0);      // Difference of x values of the centroid and each node
  yi.resize (polygonVertices.size(),0.0);     // Difference of y values of the centroid and each node
  polygon_area = 0.0;

  for (auto id=0; id<polygonVertices.size(); id++)
  {
    xi[id] = (polygonVertices[id].x_poly-building_cent_x)*cos(upwind_dir)+(polygonVertices[id].y_poly-building_cent_y)*sin(upwind_dir);
    yi[id] = -(polygonVertices[id].x_poly-building_cent_x)*sin(upwind_dir)+(polygonVertices[id].y_poly-building_cent_y)*cos(upwind_dir);
  }

  int counter = 0;
  // Loop through points to find the height added to the effective height because of rooftop parameterization
  for (auto id=0; id<polygonVertices.size()-1; id++)
  {
    // Calculate upwind reletive direction for each face
    upwind_rel_dir[id] = atan2(yi[id+1]-yi[id],xi[id+1]-xi[id])+0.5*M_PI;
    if (upwind_rel_dir[id] > M_PI+0.0001)
    {
      upwind_rel_dir[id] -= 2*M_PI;
    }
    // If there is a rooftop parameterization (only first face that is eligible)
    if (WID->simParams->rooftopFlag > 0 && counter == 0)
    {
      // Rooftop applies if upcoming wind angle is in -/+30 degrees of perpendicular direction of the face
      tol = 30*M_PI/180.0;
      if (abs(upwind_rel_dir[id]) >= M_PI-tol)
      {
        // Smaller of the effective height (height_eff) and the effective cross-wind width (Weff)
        small_dimension = MIN_S(width_eff, height_eff);
        // Larger of the effective height (height_eff) and the effective cross-wind width (Weff)
        long_dimension = MAX_S(width_eff, height_eff);
        R_scale = pow(small_dimension, (2.0/3.0))*pow(long_dimension, (1.0/3.0));     // Scaling length
        R_cx = 0.9*R_scale;               // Normalized cavity length
        vd = 0.5*0.22*R_scale;            // Cavity height
        // Smaller of the effective length (length_eff) and the effective cross-wind width (Weff)
        hd = MIN_S(width_eff, length_eff);
        if (hd < R_cx)
        {
          shell_height = vd*sqrt(1.0-pow((0.5*R_cx-hd)/(0.5*R_cx), 2.0));     // Additional height because of the rooftop
          if (shell_height > 0.0)
          {
            height_eff += shell_height;
          }
        }
        // Addition only happens once
        counter += 1;
      }
    }
  }

  L_over_H = length_eff/height_eff;           // Length over height
  W_over_H = width_eff/height_eff;            // Width over height

  // Checking bounds of length over height and width over height
  if (L_over_H > 3.0)
  {
    L_over_H = 3.0;
  }
  if (L_over_H < 0.3)
  {
    L_over_H = 0.3;
  }
  if (W_over_H > 10.0)
  {
    W_over_H = 10.0;
  }

  tol = 0.01*M_PI/180.0;
  // Calculating length of the downwind wake based on Fackrell (1984) formulation
  Lr = 1.8*height_eff*W_over_H/(pow(L_over_H,0.3)*(1+0.24*W_over_H));

  for (auto id=0; id<polygonVertices.size()-1; id++)
  {
    // Finding faces that are eligible for applying the far-wake parameterizations
    // angle between two points should be in -180 to 0 degree
    if ( abs(upwind_rel_dir[id]) < 0.5*M_PI)
    {
      // Calculate length of the far wake zone for each face
      Lr_face[id] = Lr*cos(upwind_rel_dir[id]);
    }
  }

  Lr_ave = total_seg_length = 0.0;
  // This loop interpolates the value of Lr for eligible faces to nodes of those faces
  for (auto id=0; id<polygonVertices.size()-1; id++)
  {
    // If the face is eligible for parameterization
    if (Lr_face[id] > 0.0)
    {
      index_previous = (id+polygonVertices.size()-2)%(polygonVertices.size()-1);     // Index of previous face
      index_next = (id+1)%(polygonVertices.size()-1);           // Index of next face
      if (Lr_face[index_previous] < 0.0 && Lr_face[index_next] < 0.0)
      {
        Lr_node[id] = Lr_face[id];
        Lr_node[id+1] = Lr_face[id];
      }
      else if (Lr_face[index_previous] < 0.0)
      {
        Lr_node[id] = Lr_face[id];
        Lr_node[id+1] = ((yi[index_next]-yi[index_next+1])*Lr_face[index_next]+(yi[id]-yi[index_next])*Lr_face[id])/(yi[id]-yi[index_next+1]);
      }
      else if (Lr_face[index_next] < 0.0)
      {
        Lr_node[id] = ((yi[id]-yi[index_next])*Lr_face[id]+(yi[index_previous]-yi[id])*Lr_face[index_previous])/(yi[index_previous]-yi[index_next]);
        Lr_node[id+1] = Lr_face[id];
      }
      else
      {
        Lr_node[id] = ((yi[id]-yi[index_next])*Lr_face[id]+(yi[index_previous]-yi[id])*Lr_face[index_previous])/(yi[index_previous]-yi[index_next]);
        Lr_node[id+1] = ((yi[index_next]-yi[index_next+1])*Lr_face[index_next]+(yi[id]-yi[index_next])*Lr_face[id])/(yi[id]-yi[index_next+1]);
      }
      Lr_ave += Lr_face[id]*(yi[id]-yi[index_next]);
      total_seg_length += (yi[id]-yi[index_next]);
    }

    if ((polygonVertices[id+1].x_poly > polygonVertices[0].x_poly-0.1) && (polygonVertices[id+1].x_poly < polygonVertices[0].x_poly+0.1)
         && (polygonVertices[id+1].y_poly > polygonVertices[0].y_poly-0.1) && (polygonVertices[id+1].y_poly < polygonVertices[0].y_poly+0.1))
    {
      stop_id = id;
      break;
    }
  }

  Lr = Lr_ave/total_seg_length;
  for (auto k = 1; k <= k_start; k++)
  {
    k_bottom = k;
    if (base_height <= WGD->z[k])
    {
      break;
    }
  }

  for (auto k = k_start; k < WGD->nz-2; k++)
  {
    k_top = k;
    if (height_eff < WGD->z[k+1])
    {
      break;
    }
  }

  for (auto k = k_start; k < k_end; k++)
  {
    kk = k;
    if (0.75*H+base_height <= WGD->z[k])
    {
      break;
    }
  }

  for (auto k=k_top; k>=k_bottom; k--)
  {
    z_build = WGD->z[k] - base_height;
    for (auto id=0; id<=stop_id; id++)
    {
      if (abs(upwind_rel_dir[id]) < 0.5*M_PI)
      {
        if (abs(upwind_rel_dir[id]) < tol)
        {
          perpendicular_flag[id]= 1;
          x_wall = xi[id];
        }
        for (auto y_id=0; y_id <= 2*ceil(abs(yi[id]-yi[id+1])/WGD->dxy); y_id++)
        {
          yc = yi[id]-0.5*y_id*WGD->dxy;
          Lr_local = Lr_node[id]+(yc-yi[id])*(Lr_node[id+1]-Lr_node[id])/(yi[id+1]-yi[id]);
          // Checking to see whether the face is perpendicular to the wind direction
          if(perpendicular_flag[id] == 0)
          {
            x_wall = ((xi[id+1]-xi[id])/(yi[id+1]-yi[id]))*(yc-yi[id])+xi[id];
          }
          if (yc >= 0.0)
          {
            y_norm = y2;
          }
          else
          {
            y_norm = y1;
          }
          canyon_factor = 1.0;
          x_id_min = -1;
          for (auto x_id=1; x_id <= ceil(Lr_local/WGD->dxy); x_id++)
          {
            xc = x_id*WGD->dxy;
            int i = ((xc+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)+building_cent_x)/WGD->dx;
            int j = ((xc+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)+building_cent_y)/WGD->dy;
            if ( i >= WGD->nx-2 && i <= 0 && j >= WGD->ny-2 && j <= 0)
            {
              break;
            }
            int icell_cent = i+j*(WGD->nx-1)+kk*(WGD->nx-1)*(WGD->ny-1);
            if ( WGD->icellflag[icell_cent] != 0 && WGD->icellflag[icell_cent] != 2 && x_id_min < 0)
            {
              x_id_min = x_id;
            }
            if ( (WGD->icellflag[icell_cent] == 0 || WGD->icellflag[icell_cent] == 2) && x_id_min > 0)
            {
              canyon_factor = xc/Lr;

              break;
            }
          }
          x_id_min = -1;
          for (auto x_id=1; x_id <= 2*ceil(farwake_factor*Lr_local/WGD->dxy); x_id++)
          {
            u_wake_flag = 1;
            v_wake_flag = 1;
            w_wake_flag = 1;
            xc = 0.5*x_id*WGD->dxy;
            int i = ((xc+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)+building_cent_x)/WGD->dx;
            int j = ((xc+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)+building_cent_y)/WGD->dy;
            if (i >= WGD->nx-2 && i <= 0 && j >= WGD->ny-2 && j <= 0)
            {
              break;
            }
            icell_cent = i+j*(WGD->nx-1)+k*(WGD->nx-1)*(WGD->ny-1);
            if (WGD->icellflag[icell_cent] != 0 && WGD->icellflag[icell_cent] != 2 && x_id_min < 0)
            {
              x_id_min = x_id;
            }
            if (WGD->icellflag[icell_cent] == 0 || WGD->icellflag[icell_cent] == 2)
            {
              if (x_id_min >= 0)
              {
                if (WGD->ibuilding_flag[icell_cent] == building_id)
                {
                  x_id_min = -1;
                }
                else if (canyon_factor < 1.0)
                {
                  break;
                }
                else if (WGD->icellflag[i+j*(WGD->nx-1)+kk*(WGD->nx-1)*(WGD->ny-1)] == 0 || WGD->icellflag[i+j*(WGD->nx-1)+kk*(WGD->nx-1)*(WGD->ny-1)] == 2)
                {
                  break;
                }

              }
            }

            if (WGD->icellflag[icell_cent] != 0 && WGD->icellflag[icell_cent] != 2)
            {
              i_u = std::round(((xc+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)+building_cent_x)/WGD->dx);
              j_u = ((xc+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)+building_cent_y)/WGD->dy;
              if (i_u < WGD->nx-1 && i_u > 0 && j_u < WGD->ny-1 && j_u > 0)
              {
                xp = i_u*WGD->dx-building_cent_x;
                yp = (j_u+0.5)*WGD->dy-building_cent_y;
                xu = xp*cos(upwind_dir)+yp*sin(upwind_dir);
                yu = -xp*sin(upwind_dir)+yp*cos(upwind_dir);
                Lr_local_u = Lr_node[id]+(yu-yi[id])*(Lr_node[id+1]-Lr_node[id])/(yi[id+1]-yi[id]);
                if (perpendicular_flag[id] > 0)
                {
                  x_wall_u = xi[id];

                }
                else
                {
                  x_wall_u = ((xi[id+1]-xi[id])/(yi[id+1]-yi[id]))*(yu-yi[id])+ xi[id];
                }

                xu -= x_wall_u;
                if (abs(yu) < abs(y_norm) && abs(y_norm) > epsilon && z_build < height_eff && height_eff > epsilon)
                {
                  dn_u = sqrt((1.0-pow((yu/y_norm), 2.0))*(1.0-pow((z_build/height_eff),2.0))*pow((canyon_factor*Lr_local_u),2.0));
                }
                else
                {
                  dn_u = 0.0;
                }
                if (xu > farwake_factor*dn_u)
                {
                  u_wake_flag = 0;
                }
                icell_cent = i_u + j_u*(WGD->nx-1)+k*(WGD->nx-1)*(WGD->ny-1);
                icell_face = i_u + j_u*WGD->nx+k*WGD->nx*WGD->ny;
                if (dn_u > 0.0 && u_wake_flag == 1 && yu <= yi[id] && yu >= yi[id+1] && WGD->icellflag[icell_cent] != 0 && WGD->icellflag[icell_cent] != 2)
                {
                  // Far wake zone
                  if (xu > dn_u)
                  {
                    farwake_vel = WGD->u0[icell_face]*(1.0-pow((dn_u/(xu+WGD->wake_factor*dn_u)),farwake_exp));
                    if (canyon_factor == 1.0)
                    {
                      //std::cout << "farwake_vel:   " << farwake_vel << std::endl;
                      //std::cout << "i_u:   " << i_u << "\t\t"<< "j_u:   " << j_u << "\t\t"<< "k:   " << k << std::endl;
                      u0_modified.push_back(farwake_vel);
                      u0_mod_id.push_back(icell_face);

                      WGD->w0[i+j*WGD->nx+k*WGD->nx*WGD->ny] = 0.0;
                    }
                  }
                  // Cavity zone
                  else
                  {
                    WGD->u0[icell_face] = -u0_h*MIN_S(pow((1.0-xu/(WGD->cavity_factor*dn_u)),2.0),1.0)*MIN_S(sqrt(1.0-abs(yu/y_norm)),1.0);
                    /*if (i_u == 115 && j_u == 96 && k == 1)
                    {
                      std::cout << "WGD->u0[icell_face]:   " << WGD->u0[icell_face] << std::endl;
                    }*/
                    //std::cout << "WGD->u0[icell_face]:   " << WGD->u0[icell_face] << std::endl;
                    //std::cout << "i:   " << i << "\t\t"<< "j:   " << j << "\t\t"<< "k:   " << k << std::endl;
                    WGD->w0[i+j*WGD->nx+k*WGD->nx*WGD->ny] = 0.0;
                  }
                }
              }

              i_v = ((xc+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)+building_cent_x)/WGD->dx;
              j_v = std::round(((xc+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)+building_cent_y)/WGD->dy);
              if (i_v<WGD->nx-1 && i_v>0 && j_v<WGD->ny-1 && j_v>0)
              {
                xp = (i_v+0.5)*WGD->dx-building_cent_x;
                yp = j_v*WGD->dy-building_cent_y;
                xv = xp*cos(upwind_dir)+yp*sin(upwind_dir);
                yv = -xp*sin(upwind_dir)+yp*cos(upwind_dir);
                Lr_local_v = Lr_node[id]+(yv-yi[id])*(Lr_node[id+1]-Lr_node[id])/(yi[id+1]-yi[id]);
                if (perpendicular_flag[id] > 0)
                {
                  x_wall_v = xi[id];
                }
                else
                {
                  x_wall_v = ((xi[id+1]-xi[id])/(yi[id+1]-yi[id]))*(yv-yi[id]) + xi[id];
                }
                xv -= x_wall_v;

                if (abs(yv) < abs(y_norm) && abs(y_norm) > epsilon && z_build < height_eff && height_eff > epsilon)
                {
                  dn_v = sqrt((1.0-pow((yv/y_norm), 2.0))*(1.0-pow((z_build/height_eff),2.0))*pow((canyon_factor*Lr_local_v),2.0));
                }
                else
                {
                  dn_v = 0.0;
                }
                if (xv > farwake_factor*dn_v)
                {
                  v_wake_flag = 0;
                }
                icell_cent = i_v + j_v*(WGD->nx-1)+k*(WGD->nx-1)*(WGD->ny-1);
                icell_face = i_v + j_v*WGD->nx+k*WGD->nx*WGD->ny;
                if (dn_v > 0.0 && v_wake_flag == 1 && yv <= yi[id] && yv >= yi[id+1] && WGD->icellflag[icell_cent] != 0 && WGD->icellflag[icell_cent] != 2)
                {
                  // Far wake zone
                  if (xv > dn_v)
                  {
                    farwake_vel = WGD->v0[icell_face]*(1.0-pow((dn_v/(xv+WGD->wake_factor*dn_v)),farwake_exp));
                    if (canyon_factor == 1)
                    {
                      v0_modified.push_back(farwake_vel);
                      v0_mod_id.push_back(icell_face);
                      WGD->w0[i+j*WGD->nx+k*WGD->nx*WGD->ny] = 0.0;
                    }
                  }
                  // Cavity zone
                  else
                  {
                    WGD->v0[icell_face] = -v0_h*MIN_S(pow((1.0-xv/(WGD->cavity_factor*dn_v)),2.0),1.0)*MIN_S(sqrt(1.0-abs(yv/y_norm)),1.0);
                    /*if (i_v == 115 && j_v == 96 && k == 1)
                    {
                      std::cout << "v0_h:   " << v0_h << std::endl;
                      std::cout << "MIN_S(pow((1.0-xv/(WGD->cavity_factor*dn_v)),2.0),1.0):   " << MIN_S(pow((1.0-xv/(WGD->cavity_factor*dn_v)),2.0),1.0) << std::endl;
                      std::cout << "MIN_S(sqrt(1.0-abs(yv/y_norm)),1.0):   " << MIN_S(sqrt(1.0-abs(yv/y_norm)),1.0) << std::endl;
                      std::cout << "WGD->v0[icell_face]:   " << WGD->v0[icell_face] << std::endl;
                    }*/
                    //std::cout << "WGD->v0[icell_face]:   " << WGD->v0[icell_face] << std::endl;
                    //std::cout << "i:   " << i << "\t\t"<< "j:   " << j << "\t\t"<< "k:   " << k << std::endl;
                    WGD->w0[icell_face] = 0.0;
                  }
                }
              }

              i_w = ceil(((xc+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)+building_cent_x)/WGD->dx)-1;
              j_w = ceil(((xc+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)+building_cent_y)/WGD->dy)-1;
              if (i_w<WGD->nx-2 && i_w>0 && j_w<WGD->ny-2 && j_w>0)
              {
                xp = (i_w+0.5)*WGD->dx-building_cent_x;
                yp = (j_w+0.5)*WGD->dy-building_cent_y;
                xw = xp*cos(upwind_dir)+yp*sin(upwind_dir);
                yw = -xp*sin(upwind_dir)+yp*cos(upwind_dir);
                Lr_local_w = Lr_node[id]+(yw-yi[id])*(Lr_node[id+1]-Lr_node[id])/(yi[id+1]-yi[id]);
                if (perpendicular_flag[id] > 0)
                {
                  x_wall_w = xi[id];
                }
                else
                {
                  x_wall_w = ((xi[id+1]-xi[id])/(yi[id+1]-yi[id]))*(yw-yi[id]) + xi[id];
                }
                xw -= x_wall_w;
                if (abs(yw) < abs(y_norm) && abs(y_norm) > epsilon && z_build < height_eff && height_eff > epsilon)
                {
                  dn_w = sqrt((1.0-pow(yw/y_norm, 2.0))*(1.0-pow(z_build/height_eff,2.0))*pow(canyon_factor*Lr_local_w,2.0));
                }
                else
                {
                  dn_w = 0.0;
                }

                if (xw > farwake_factor*dn_w)
                {
                  w_wake_flag = 0;
                }
                icell_cent = i_w + j_w*(WGD->nx-1)+k*(WGD->nx-1)*(WGD->ny-1);
                icell_face = i_w + j_w*WGD->nx+k*WGD->nx*WGD->ny;
                if (dn_w > 0.0 && w_wake_flag == 1 && yw <= yi[id] && yw >= yi[id+1] && WGD->icellflag[icell_cent] != 0 && WGD->icellflag[icell_cent] != 2)
                {
                  if (xw > dn_w)
                  {
                    if (canyon_factor == 1)
                    {
                      if ((WGD->icellflag[icell_cent] != 7) && (WGD->icellflag[icell_cent] != 8))
                      {
                        WGD->icellflag[icell_cent] = 5;
                      }
                    }
                  }
                  else
                  {
                    if ((WGD->icellflag[icell_cent] != 7) && (WGD->icellflag[icell_cent] != 8))
                    {
                      WGD->icellflag[icell_cent] = 4;
                    }
                  }
                }
                if (u_wake_flag == 0 && v_wake_flag == 0 && w_wake_flag == 0)
                {
                  break;
                }
              }
            }
          }
        }
      }
    }
  }

  for (auto x_id=0; x_id < u0_mod_id.size(); x_id++)
  {
    WGD->u0[u0_mod_id[x_id]] = u0_modified[x_id];
  }

  for (auto y_id=0; y_id < v0_mod_id.size(); y_id++)
  {
    WGD->v0[v0_mod_id[y_id]] = v0_modified[y_id];
  }

  u0_mod_id.clear();
  v0_mod_id.clear();
  u0_modified.clear();
  v0_modified.clear();
}
