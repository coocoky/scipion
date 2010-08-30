/***************************************************************************
 *
 * Authors:     Carlos Oscar S. Sorzano (coss@cnb.csic.es)
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

#include <data/phantom.h>

void Usage(char *argv[]);

int main(int argc, char *argv[])
{
    FileName          fn_phantom;
    FileName          fn_vol;
    Phantom           phantom;
    Image<double>     vol;

    // Read Parameters ......................................................
    try
    {
        fn_phantom = getParameter(argc, argv, "-i");
        fn_vol     = getParameter(argc, argv, "-o");
    }
    catch (XmippError XE)
    {
        std::cout << XE;
        Usage(argv);
    }


    try
    {
        // Read description file .............................................
        phantom.read(fn_phantom);

        // Generate volume and write .........................................
        phantom.draw_in(vol());
        vol.write(fn_vol);
    }
    catch (XmippError XE)
    {
        std::cout << XE;
        exit(1);
    }
    exit(0);
}

/* Usage ------------------------------------------------------------------- */
void Usage(char *argv[])
{
    printf(
        "\n\nPurpose: This program allows you to create phantom XMIPP volumes\n"
        "     from a phantom feature description file.\n"
    );
    printf(
        "\n\nUsage: %s [Parameters]"
        "\nOptions:"
        "\nParameter Values: (note space before value)"
        "\n    -i <description file>"
        "\n    -o <output file>"
        "\n"
        , argv[0]);
    exit(1);
}
