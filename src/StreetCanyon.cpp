#include "PolyBuilding.h"

// These take care of the circular reference
#include "URBInputData.h"
#include "URBGeneralData.h"



/**
*
* This function applies the street canyon parameterization to the qualified space between buildings defined as polygons.
* This function reads in building features like nodes, building height and base height and uses
* features of the building defined in the class constructor and setCellsFlag function. It defines
* cells qualified in the space between buildings and applies the approperiate parameterization to them.
* More information: "Improvements to a fast-response urban wind model, M. Nelson et al. (2008)"
*
*/
void PolyBuilding::streetCanyon (URBGeneralData *UGD)
{
  float tol = 0.01*M_PI/180.0;
  float angle_tol = 3*M_PI/4;
  float x_wall, x_wall_u, x_wall_v, x_wall_w;
  float xc, yc;
  int top_flag, canyon_flag;
  int k_ref;
  int reverse_flag;
  int x_id_min, x_id_max;
  int number_u, number_v;
  float u_component, v_component;
  float s;                         // Distance between two buildings
  float velocity_mag;
  float canyon_dir;
  int d_build;                    // Downwind building number
  int i_u, j_v;
  float x_u, y_u, x_v, y_v, x_w, y_w;
  float x_pos;
  float x_p, y_p;
  float cross_dir, x_ave, y_ave;
  float x_down, y_down;
  float segment_length;                     // Face length
  float downwind_rel_dir, along_dir;
  float cross_vel_mag, along_vel_mag;
  std::vector<int> perpendicular_flag;
  std::vector<float> perpendicular_dir;

  xi.resize (polygonVertices.size(),0.0);      // Difference of x values of the centroid and each node
  yi.resize (polygonVertices.size(),0.0);     // Difference of y values of the centroid and each node
  upwind_rel_dir.resize (polygonVertices.size(), 0.0);      // Upwind reletive direction for each face
  perpendicular_flag.resize (polygonVertices.size(), 0);
  perpendicular_dir.resize (polygonVertices.size(), 0.0);

  int index_building_face = i_building_cent + j_building_cent*UGD->nx + (k_end)*UGD->nx*UGD->ny;
  u0_h = UGD->u0[index_building_face];         // u velocity at the height of building at the centroid
  v0_h = UGD->v0[index_building_face];         // v velocity at the height of building at the centroid

  // Wind direction of initial velocity at the height of building at the centroid
  upwind_dir = atan2(v0_h,u0_h);
  // Loop through to calculate projected location for each polygon node in rotated coordinates
  for (auto id=0; id<polygonVertices.size(); id++)
  {
    xi[id] = (polygonVertices[id].x_poly-building_cent_x)*cos(upwind_dir)
            +(polygonVertices[id].y_poly-building_cent_y)*sin(upwind_dir);
    yi[id] = -(polygonVertices[id].x_poly-building_cent_x)*sin(upwind_dir)
            +(polygonVertices[id].y_poly-building_cent_y)*cos(upwind_dir);
  }


  for (auto id=0; id<polygonVertices.size()-1; id++)
  {
    // Calculate upwind reletive direction for each face
    upwind_rel_dir[id] = atan2(yi[id+1]-yi[id],xi[id+1]-xi[id])+0.5*M_PI;

    if (upwind_rel_dir[id] > M_PI)
    {
      upwind_rel_dir[id] -= 2*M_PI;
    }

    // angle between two points should be in -180 to 0 degree
    if ( abs(upwind_rel_dir[id]) < 0.5*M_PI-0.0001)
    {
      // Checking to see whether the face is perpendicular to the wind direction
      if (abs(upwind_rel_dir[id]) > M_PI-tol || abs(upwind_rel_dir[id]) < tol)
      {
        perpendicular_flag[id] = 1;
        x_wall = xi[id];
      }
      // Calculating perpendicula direction to each face
      perpendicular_dir[id] = atan2(polygonVertices[id+1].y_poly-polygonVertices[id].y_poly,
                              polygonVertices[id+1].x_poly-polygonVertices[id].x_poly)+0.5*M_PI;
      if (perpendicular_dir[id] > M_PI)
      {
        perpendicular_dir[id] -= 2*M_PI;      // Force the angle to be between -pi and pi
      }
      // Loop through y locations along each face in rotated coordinates
      for (auto y_id=0; y_id <= 2*ceil(abs(yi[id]-yi[id+1])/UGD->dxy); y_id++)
      {
        yc = MIN_S(yi[id],yi[id+1])+0.5*y_id*UGD->dxy;      // y locations along each face in rotated coordinates
        top_flag = 0;                               // If we are inside the canyon
        if(perpendicular_flag[id] == 0)
        {
          x_wall = ((xi[id+1]-xi[id])/(yi[id+1]-yi[id]))*(yc-yi[id])+xi[id];      // x location of each yc point parallel to the face
        }

        for (auto k = k_end-1; k >= k_start; k--)
        {
          canyon_flag = 0;
          s = 0.0;
          reverse_flag = 0;
          x_id_min = -1;
          // Loop through x locations along perpendicular direction of each face
          for (auto x_id = 1; x_id <= 2*ceil(Lr/UGD->dxy); x_id++)
          {
            xc = 0.5*x_id*UGD->dxy;              // x locations along perpendicular direction of each face
            // Finding i and j indices of the cell (xc, yc) located in
            int i = ceil(((xc+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)
                            +building_cent_x)/UGD->dx)-1;
            int j = ceil(((xc+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)
                            +building_cent_y)/UGD->dy)-1;
            icell_cent = i+j*(UGD->nx-1)+k*(UGD->nx-1)*(UGD->ny-1);
            // Making sure i and j are inside the domain
            if (i>=UGD->nx-2 && i<=0 && j>=UGD->ny-2 && j<=0)
            {
              break;
            }
            icell_cent = i+j*(UGD->nx-1)+k*(UGD->nx-1)*(UGD->ny-1);
            // Finding id of the first cell in perpendicular direction of the face that is outside of the building
            if (UGD->icellflag[icell_cent] != 0 && UGD->icellflag[icell_cent] != 2 && x_id_min < 0)
            {
              x_id_min = x_id;
            }
            // Finding id of the last cell in perpendicular direction of the face that is outside of the building
            if ( (UGD->icellflag[icell_cent] == 0 || UGD->icellflag[icell_cent] == 2) && x_id_min >= 0)
            {
              canyon_flag = 1;
              x_id_max = x_id-1;
              s = 0.5*(x_id_max-x_id_min)*UGD->dxy;           // Distance between two buildings
              if (top_flag == 0)            // If inside the street canyon
              {
                k_ref = k+1;
                int ic = ceil(((0.5*x_id_max*UGD->dxy+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)
                                +building_cent_x-0.001)/UGD->dx)-1;
                int jc = ceil(((0.5*x_id_max*UGD->dxy+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)
                                +building_cent_y-0.001)/UGD->dy)-1;
                icell_cent = ic+jc*(UGD->nx-1)+k_ref*(UGD->nx-1)*(UGD->ny-1);
                int icell_face = ic+jc*UGD->nx+k_ref*UGD->nx*UGD->ny;
                if (UGD->icellflag[icell_cent] != 0 && UGD->icellflag[icell_cent] != 2)
                {
                  number_u = 0;
                  number_v = 0;
                  u_component = 0.0;
                  v_component = 0.0;
                  if (UGD->icellflag[icell_cent-1] != 0 && UGD->icellflag[icell_cent-1] != 2)
                  {
                    number_u += 1;
                    u_component += UGD->u0[icell_face];
                  }
                  if (UGD->icellflag[icell_cent+1] != 0 && UGD->icellflag[icell_cent+1] != 2)
                  {
                    number_u += 1;
                    u_component += UGD->u0[icell_face+1];
                  }
                  if (UGD->icellflag[icell_cent-(UGD->nx-1)] != 0 && UGD->icellflag[icell_cent-(UGD->nx-1)] != 2)
                  {
                    number_v += 1;
                    v_component += UGD->v0[icell_face];
                  }
                  if (UGD->icellflag[icell_cent+(UGD->nx-1)] != 0 && UGD->icellflag[icell_cent+(UGD->nx-1)] != 2)
                  {
                    number_v += 1;
                    v_component += UGD->v0[icell_face+UGD->nx];
                  }

                  if ( u_component != 0.0 && number_u > 0)
                  {
                    u_component /= number_u;
                  }
                  else
                  {
                    u_component = 0.0;
                  }
                  if ( v_component != 0.0 && number_v > 0)
                  {
                    v_component /= number_v;
                  }
                  else
                  {
                    v_component = 0.0;
                  }

                  if (number_u == 0 && number_v == 0)
                  {
                    canyon_flag = 0;
                    top_flag = 0;
                    s = 0.0;
                    break;
                  }
                  else if (number_u > 0 && number_v > 0)
                  {
                    velocity_mag = sqrt(pow(u_component,2.0)+pow(v_component,2.0));
                    canyon_dir = atan2(v_component,u_component);
                  }
                  else if (number_u > 0)
                  {
                    velocity_mag = abs(u_component);
                    if (u_component > 0.0)
                    {
                      canyon_dir = 0.0;
                    }
                    else
                    {
                      canyon_dir = M_PI;
                    }
                  }
                  else
                  {
                    velocity_mag = abs(v_component);
                    if (v_component > 0.0)
                    {
                      canyon_dir = 0.5*M_PI;
                    }
                    else
                    {
                      canyon_dir = -0.5*M_PI;
                    }
                  }

                  top_flag = 1;
                  icell_cent = i+j*(UGD->nx-1)+k*(UGD->nx-1)*(UGD->ny-1);

                  if (abs(s) > 0.0)
                  {
                    if ( (UGD->ibuilding_flag[icell_cent] >= 0) && (UGD->allBuildingsV[UGD->ibuilding_flag[icell_cent]]->height_eff < height_eff) && (UGD->z_face[k]/s < 0.65) )
                    {
                      canyon_flag = 0;
                      top_flag = 0;
                      s = 0.0;
                      break;
                    }
                  }
                }
                else
                {
                  canyon_flag = 0;
                  top_flag = 0;
                  s = 0.0;
                  break;
                }
                if (velocity_mag > UGD->max_velmag)
                {
                  canyon_flag = 0;
                  top_flag = 0;
                  s = 0.0;
                  break;
                }
              }

              icell_cent = i+j*(UGD->nx-1)+k*(UGD->nx-1)*(UGD->ny-1);
              if (UGD->ibuilding_flag[icell_cent] >= 0)
              {
                d_build = UGD->ibuilding_flag[icell_cent];
                int i = ceil(((xc-0.5*UGD->dxy+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)
                              +building_cent_x-0.001)/UGD->dx)-1;
                int j = ceil(((xc-0.5*UGD->dxy+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)
                              +building_cent_y-0.001)/UGD->dy)-1;
                for (auto j_id = 0; j_id < UGD->allBuildingsV[d_build]->polygonVertices.size()-1; j_id++)
                {
                  cross_dir = atan2(UGD->allBuildingsV[d_build]->polygonVertices[j_id+1].y_poly-UGD->allBuildingsV[d_build]->polygonVertices[j_id].y_poly,
                                    UGD->allBuildingsV[d_build]->polygonVertices[j_id+1].x_poly-UGD->allBuildingsV[d_build]->polygonVertices[j_id].x_poly)+0.5*M_PI;

                  if (cross_dir > M_PI+0.001)
                  {
                    cross_dir -= 2*M_PI;
                  }
                  x_ave = 0.5*(UGD->allBuildingsV[d_build]->polygonVertices[j_id+1].x_poly+UGD->allBuildingsV[d_build]->polygonVertices[j_id].x_poly);
                  y_ave = 0.5*(UGD->allBuildingsV[d_build]->polygonVertices[j_id+1].y_poly+UGD->allBuildingsV[d_build]->polygonVertices[j_id].y_poly);

                  x_down = ((i+0.5)*UGD->dx-x_ave)*cos(cross_dir) + ((j+0.5)*UGD->dy-y_ave)*sin(cross_dir);
                  y_down = -((i+0.5)*UGD->dx-x_ave)*sin(cross_dir) + ((j+0.5)*UGD->dy-y_ave)*cos(cross_dir);

                  if (abs(x_down) < 0.75*UGD->dxy)
                  {
                    segment_length = sqrt(pow(UGD->allBuildingsV[d_build]->polygonVertices[j_id+1].x_poly-UGD->allBuildingsV[d_build]->polygonVertices[j_id].x_poly, 2.0)
                                          +pow(UGD->allBuildingsV[d_build]->polygonVertices[j_id+1].y_poly-UGD->allBuildingsV[d_build]->polygonVertices[j_id].y_poly, 2.0));
                    if (abs(y_down) <= 0.5*segment_length)
                    {
                      downwind_rel_dir = canyon_dir-cross_dir;
                      if (downwind_rel_dir > M_PI+0.001)
                      {
                        downwind_rel_dir -= 2*M_PI;
                        if (abs(downwind_rel_dir) < 0.001)
                        {
                          downwind_rel_dir = 0.0;
                        }
                      }
                      if (downwind_rel_dir <= -M_PI)
                      {
                        downwind_rel_dir += 2*M_PI;
                        if (abs(downwind_rel_dir) < 0.001)
                        {
                          downwind_rel_dir = 0.0;
                        }
                      }
                      if (abs(downwind_rel_dir) < 0.5*M_PI)
                      {
                        reverse_flag = 1;
                        if (downwind_rel_dir >= 0.0)
                        {
                          along_dir = cross_dir-0.5*M_PI;
                        }
                        else
                        {
                          along_dir = cross_dir+0.5*M_PI;
                        }
                      }
                      else
                      {
                        reverse_flag = 0;
                        if (downwind_rel_dir >= 0.0)
                        {
                          along_dir = cross_dir+0.5*M_PI;
                        }
                        else
                        {
                          along_dir = cross_dir-0.5*M_PI;
                        }
                      }
                      if (along_dir > M_PI+0.001)
                      {
                        along_dir -= 2*M_PI;
                      }
                      if (along_dir <= -M_PI)
                      {
                        along_dir += 2*M_PI;
                      }
                      break;
                    }
                  }
                }
                if (cross_dir <= -M_PI)
                {
                  cross_dir += 2*M_PI;
                }
                if (reverse_flag == 1)
                {
                  if (cos(cross_dir-perpendicular_dir[id]) < -cos(angle_tol))
                  {
                    canyon_flag = 0;
                    s = 0;
                    top_flag = 0;
                  }
                }
                else
                {
                  if (cos(cross_dir-perpendicular_dir[id]) > cos(angle_tol))
                  {
                    canyon_flag = 0;
                    s = 0;
                    top_flag = 0;
                  }
                }
                break;
              }
            }

          }


          //std::cout << "along_dir:   " << along_dir << std::endl;
          if (canyon_flag == 1 && s > 0.9*UGD->dxy)
          {
            along_vel_mag = abs(velocity_mag*cos(canyon_dir-along_dir))*log(UGD->z[k]/UGD->z0)/log(UGD->z[k_ref]/UGD->z0);
            cross_vel_mag = abs(velocity_mag*cos(canyon_dir-cross_dir));
            for (auto x_id = x_id_min; x_id <= x_id_max; x_id++)
            {
              xc = 0.5*x_id*UGD->dxy;
              int i = ceil(((xc+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)
                              +building_cent_x-0.001)/UGD->dx)-1;
              int j = ceil(((xc+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)
                              +building_cent_y-0.001)/UGD->dy)-1;
              icell_cent = i+j*(UGD->nx-1)+k*(UGD->nx-1)*(UGD->ny-1);
              if (UGD->icellflag[icell_cent] != 0 && UGD->icellflag[icell_cent] != 2)
              {
                i_u = std::round(((xc+x_wall)*cos(upwind_dir)-yc*sin(upwind_dir)
                                    +building_cent_x)/UGD->dx);

                x_p = i_u*UGD->dx-building_cent_x;
                y_p = (j+0.5)*UGD->dy-building_cent_y;
                x_u = x_p*cos(upwind_dir)+y_p*sin(upwind_dir);
                y_u = -x_p*sin(upwind_dir)+y_p*cos(upwind_dir);

                if(perpendicular_flag[id] == 0)
                {
                  x_wall_u = ((xi[id+1]-xi[id])/(yi[id+1]-yi[id]))*(y_u-yi[id])+xi[id];
                }
                else
                {
                  x_wall_u = xi[id];
                }
                x_pos = x_u-x_wall_u;
                if (x_pos <= s && x_pos > -0.5*UGD->dxy)
                {
                  icell_face = i_u+j*UGD->nx+k*UGD->nx*UGD->ny;
                  UGD->u0[icell_face] = along_vel_mag*cos(along_dir)+cross_vel_mag*(2*x_pos/s)*2*(1-x_pos/s)*cos(cross_dir);
                }

                j_v = std::round(((xc+x_wall)*sin(upwind_dir)+yc*cos(upwind_dir)
                                    +building_cent_y)/UGD->dy);
                x_p = (i+0.5)*UGD->dx-building_cent_x;
                y_p = j_v*UGD->dy-building_cent_y;
                x_v = x_p*cos(upwind_dir)+y_p*sin(upwind_dir);
                y_v = -x_p*sin(upwind_dir)+y_p*cos(upwind_dir);
                if(perpendicular_flag[id] == 0)
                {
                  x_wall_v = ((xi[id+1]-xi[id])/(yi[id+1]-yi[id]))*(y_v-yi[id])+xi[id];
                }
                else
                {
                  x_wall_v = xi[id];
                }
                x_pos = x_v-x_wall_v;
                if (x_pos <= s && x_pos > -0.5*UGD->dxy)
                {
                  icell_face = i+j_v*UGD->nx+k*UGD->nx*UGD->ny;
                  UGD->v0[icell_face] = along_vel_mag*sin(along_dir)+cross_vel_mag*(2*x_pos/s)*2*(1-x_pos/s)*sin(cross_dir);
                }

                x_p = (i+0.5)*UGD->dx-building_cent_x;
                y_p = (j+0.5)*UGD->dy-building_cent_y;
                x_w = x_p*cos(upwind_dir)+y_p*sin(upwind_dir);
                y_w = -x_p*sin(upwind_dir)+y_p*cos(upwind_dir);
                if(perpendicular_flag[id] == 0)
                {
                  x_wall_w = ((xi[id+1]-xi[id])/(yi[id+1]-yi[id]))*(y_w-yi[id])+xi[id];
                }
                else
                {
                  x_wall_w = xi[id];
                }
                x_pos = x_w-x_wall_w;

                if (x_pos <= s && x_pos > -0.5*UGD->dxy)
                {
                  icell_cent = i+j*(UGD->nx-1)+k*(UGD->nx-1)*(UGD->ny-1);
                  if (UGD->icellflag[icell_cent-(UGD->nx-1)*(UGD->ny-1)] != 0 && UGD->icellflag[icell_cent-(UGD->nx-1)*(UGD->ny-1)] != 2)
                  {
                    icell_face = i+j*UGD->nx+k*UGD->nx*UGD->ny;
                    if (reverse_flag == 0)
                    {
                      UGD->w0[icell_face] = -abs(0.5*cross_vel_mag*(1-2*x_pos/s))*(1-2*(s-x_pos)/s);
                    }
                    else
                    {
                      UGD->w0[icell_face] = abs(0.5*cross_vel_mag*(1-2*x_pos/s))*(1-2*(s-x_pos)/s);
                    }
                  }
                  if ((UGD->icellflag[icell_cent] != 7) && (UGD->icellflag[icell_cent] != 8))
                  {
                    UGD->icellflag[icell_cent] = 6;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

}
