/***************************************************************************
 *
 * Authors:    Carlos Oscar            coss@cnb.csic.es (1999)
 *             Roberto Marabini        added bild option (2008)
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

#include <data/args.h>
#include <data/metadata.h>
#include <data/geometry.h>
#include <data/histogram.h>
#include <interface/spider.h>

#include <fstream>

void Usage();

int main(int argc, char *argv[])
{
    FileName         fn_ang, fn_hist, fn_ps, fn_bild;
    int              steps;
    int              tell;
    double           R, r, rmax, wmax = -99.e99;
    double           rot_view;
    double           tilt_view;
    int              up_down_correction;
    bool             solid_sphere;
    double           shift_center;

    // Check the command line ==================================================
    try
    {
        fn_ang = getParameter(argc, argv, "-ang");
        fn_hist = getParameter(argc, argv, "-hist", "");
        fn_ps = getParameter(argc, argv, "-ps", "");
        fn_bild = getParameter(argc, argv, "-bild", "");
        steps = textToInteger(getParameter(argc, argv, "-steps", "100"));
        tell = checkParameter(argc, argv, "-show_process");
        R = textToFloat(getParameter(argc, argv, "-R", "60"));
        rmax = textToFloat(getParameter(argc, argv, "-r", "1.5"));
        rot_view = textToFloat(getParameter(argc, argv, "-rot_view",  "0"));
        tilt_view = textToFloat(getParameter(argc, argv, "-tilt_view", "30"));
        up_down_correction = checkParameter(argc, argv, "-up_down_correction");
        solid_sphere = checkParameter(argc, argv, "-solid_sphere");
        shift_center= textToFloat(getParameter(argc, argv, "-shift_center", "0"));
    }
    catch (XmippError XE)
    {
        std::cout << XE;
        Usage();
        exit(1);
    }

    try
    {
        // Get angles ==============================================================
        MetaData angles;
        angles.read(fn_ang);
        int AngleNo = angles.size();
        if (AngleNo == 0 || !angles.containsLabel(MDL_ANGLEROT))
            EXIT_ERROR(1, "Angular distribution: Input file doesn't contain angular information");

        if (angles.containsLabel(MDL_WEIGHT))
        {
            // Find maximum weight
            FOR_ALL_OBJECTS_IN_METADATA(angles)
            {
                double w;
                angles.getValue(MDL_WEIGHT,w);
                wmax=XMIPP_MAX(w,wmax);
            }
        }

        // Build vector tables ======================================================
#define GET_ANGLES(i) \
    angles.getValue(MDL_ANGLEROT,rot,i); \
    angles.getValue(MDL_ANGLETILT,tilt,i); \
    angles.getValue(MDL_ANGLEPSI,psi,i); \
    if (up_down_correction && ABS(tilt)>90) \
        Euler_up_down(rot,tilt,psi,rot,tilt,psi);

        double rot, tilt, psi;
        std::vector< Matrix1D<double> > v, v_ang;
        v.reserve(AngleNo);
        v_ang.reserve(AngleNo);
        for (int i = 0; i < AngleNo; i++)
        {
            Matrix1D<double> aux(3);
            Matrix1D<double> aux_ang(6);

            GET_ANGLES(i + 1);
            Euler_direction(rot, tilt, psi, aux);
            v.push_back(aux);

            aux_ang = vectorR3(rot, tilt, psi);
            v_ang.push_back(aux_ang);
        }

        // Compute histogram of distances =============================================
        if (fn_hist != "")
        {
            MultidimArray<double> dist;

#define di A1D_ELEM(dist,i)
#define dj A1D_ELEM(dist,j)

#define SHOW {\
			GET_ANGLES(i+1); \
			std::cout << i << " " << rot << " " << tilt << " v[i]=" \
			<< v[i].transpose() << std::endl; \
			GET_ANGLES(j+1); \
			std::cout << j << " " << rot << " " << tilt << " v[j]=" \
			<< v[j].transpose() << std::endl; \
			std::cout << " d= " << d << std::endl << std::endl; \
        }

            // Compute minimum distance table
            dist.initZeros(AngleNo);
            for (int i = 0; i < AngleNo; i++)
                for (int j = i + 1; j < AngleNo; j++)
                {
                    double d = spherical_distance(v[i], v[j]);
                    if (di == 0 || d < di)
                    {
                        di = d;
                        if (tell)
                            SHOW;
                    }
                    if (dj == 0 || d < dj)
                    {
                        dj = d;
                        if (tell)
                            SHOW;
                    }
                }

            Histogram1D dist_hist;
            double min, max;
            dist.computeDoubleMinMax(min, max);
            dist_hist.init(min, max, steps);
            for (int i = 0; i < AngleNo; i++)
                dist_hist.insert_value(di);
            dist_hist.write(fn_hist);
        }

        // Show distribution in chimera as bild file ==========================================
        if (fn_bild != "")
        {
            std::ofstream fh_bild;
            fh_bild.open(fn_bild.c_str(), std::ios::out);
            if (!fh_bild)
                EXIT_ERROR(1, (std::string)"Ang_distribution: Cannot open " + fn_ps + " for output");
            fh_bild << ".color 1 0 0" << std::endl;

            for (int i = 0; i < AngleNo; i++)
            {
                // Triangle size depedent on w
                if (wmax>0)
                {
                    angles.getValue(MDL_WEIGHT,r, i+1);
                    r *= rmax / wmax;
                }
                else
                    r = rmax;
                fh_bild << ".sphere "
                << R*XX(v[i])  + shift_center << " "
                << R*YY(v[i])  + shift_center << " "
                << R*ZZ(v[i])  + shift_center << " "
                << r
                <<"\n";
            }
            fh_bild.close();
        }
        // Show distribution as triangles ==========================================
        if (fn_ps != "")
        {
            std::ofstream fh_ps;
            fh_ps.open(fn_ps.c_str(), std::ios::out);
            if (!fh_ps)
                EXIT_ERROR(1, (std::string)"Ang_distribution: Cannot open " + fn_ps + " for output");

            fh_ps << "%%!PS-Adobe-2.0\n";
            fh_ps << "%% Creator: Angular Distribution\n";
            fh_ps << "%% Title: Angular distribution of " << fn_ang << "\n";
            fh_ps << "%% Pages: 1\n";

#define TO_PS(x,y) \
    tmp=y; \
    y=400.0f-x*250.0f/60; \
    x=300.0f+tmp*250.0f/60;

            Matrix1D<double> p0(4), p1(4), p2(4), p3(4), origin(3);
            Matrix2D<double> A, euler_view;
            Euler_angles2matrix(rot_view, tilt_view, 0.0f, euler_view);
            origin.initZeros();
            double tmp;
            for (int i = 0; i < AngleNo; i++)
            {
                // Triangle size dependent on w
                if (wmax>0)
                {
                    angles.getValue(MDL_WEIGHT,r,i+1);
                    r *= rmax / wmax;
                }
                else
                    r = rmax;

                // Initially the triangle is on the floor of the projection plane
                VECTOR_R3(p0,    0   ,      0        , 0);
                VECTOR_R3(p1,    0   , r*2 / 3*SIND(60), 0);
                VECTOR_R3(p2, r / 2*0.6, -r*1 / 3*SIND(60), 0);
                VECTOR_R3(p3, -r / 2*0.6, -r*1 / 3*SIND(60), 0);

                // Convert to homogeneous coordinates
                p0(3) = 1;
                p1(3) = 1;
                p2(3) = 1;
                p3(3) = 1;

                // Compute Transformation matrix
                GET_ANGLES(i + 1);
                Euler_angles2matrix(rot, tilt, psi, A);

                // We go from the projeciton plane to the universal coordinates
                A = A.transpose();

                // Convert to homogeneous coordinates and apply a translation
                // to the sphere of radius R
                A.resize(4, 4);
                A(0, 3) = R * XX(v[i]);
                A(1, 3) = R * YY(v[i]);
                A(2, 3) = R * ZZ(v[i]);
                A(3, 3) = 1;

                // Convert triangle coordinates to universal ones
                p0 = A * p0;
                p1 = A * p1;
                p2 = A * p2;
                p3 = A * p3;

                // Check if this triangle must be drawn
                if (solid_sphere)
                {
                    Matrix1D<double> view_direction, p0p;
                    euler_view.getRow(2, view_direction);
                    p0p = p0;
                    p0p.resize(3);
                    if (point_plane_distance_3D(p0, origin, view_direction) < 0)
                        continue;
                }

                // Project this triangle onto the view plane and write in PS
                Matrix1D<double> pp(3);
                Uproject_to_plane(p1, euler_view, pp);
                TO_PS(XX(pp), YY(pp));
                fh_ps << "newpath\n";
                fh_ps << XX(pp) << " " << YY(pp) << " moveto\n";

                Uproject_to_plane(p2, euler_view, pp);
                TO_PS(XX(pp), YY(pp));
                fh_ps << XX(pp) << " " << YY(pp) << " lineto\n";

                Uproject_to_plane(p3, euler_view, pp);
                TO_PS(XX(pp), YY(pp));
                fh_ps << XX(pp) << " " << YY(pp) << " lineto\n";

                Uproject_to_plane(p1, euler_view, pp);
                TO_PS(XX(pp), YY(pp));
                fh_ps << XX(pp) << " " << YY(pp) << " lineto\n";

                fh_ps << "closepath\nstroke\n";
            }
            fh_ps << "showpage\n";
            fh_ps.close();
        }

    }
    catch (XmippError XE)
    {
        std::cout << XE;
    }
}

/* Usage ------------------------------------------------------------------- */
void Usage()
{
    std::cout << "Usage:\n";
    std::cout << "   ang_distribution <options>\n";
    std::cout << "   Where <options> are:\n";
    std::cout
    << "       -ang <metadata>              : Metadata file with the angles\n"
    << "      [-bild <-chimera file out>    : Chimera file\n"
    << "      [-hist <doc_file>]            : histogram of distances\n"
    << "      [-steps <stepno=100>]         : number of divisions in the histogram\n"
    << "      [-show_process]               : show distances.\n"
    << "      [-ps <PS file out>]           : PS file with the topological sphere\n"
    << "      [-R <big_sphere_radius=60>]   : sphere radius for the PS/bild file\n"
    << "      [-r <triangle side=1.5>]      : triangle size for the PS/bild file\n"
    << "      [-rot_view <rot angle=0>]     : rotational angle for the view\n"
    << "      [-tilt_view <tilt angle=30>]  : tilting angle for the view\n"
    << "      [-shift_center <shift_center=0>]: shift coordinates center for bild file\n"
    << "      [-up_down_correction]         : correct angles so that a semisphere\n"
    << "                                      is shown\n"
    << "      [-solid_sphere]               : projections in the back plane are\n"
    << "                                      not shown\n"
    ;
}
