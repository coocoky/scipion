/***************************************************************************
 *
 * Authors:    Sjors Scheres
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
#include <data/image.h>
#include <data/selfile.h>
#include <data/docfile.h>
#include <reconstruction/angular_class_average.h>
#include <data/metadata.h>

/* MAIN -------------------------------------------------------------------- */
int main(int argc, char **argv)
{
/*
	MetaDataContainer mdc1;
	mdc1.addValue(MDL_IMAGE,std::string("imag"));
	mdc1.addValue(MDL_ANGLEPSI,2.3);
	std::cerr<<"before" << std::endl;
	MetaDataContainer mdc2 (mdc1);
    double * resultd = (double *)mdc2.getValue( MDL_ANGLEPSI );
	std::cout<< "psi1= " << *resultd<<std::endl;
	mdc1.addValue(MDL_ANGLEPSI,3.14);
    resultd = (double *)mdc2.getValue( MDL_ANGLEPSI );
	std::cout<< "psi2= " << *resultd <<std::endl;
    resultd = (double *)mdc1.getValue( MDL_ANGLEPSI );
	std::cout<< "mdc1.psi2= " << *resultd <<std::endl;

	//std::string * results = (std::string *)mdc2.getValue( MDL_IMAGE );
	//std::cout<< "image= " << *results;

	MetaData MD1,MD2;
	FileName infile("a.doc");
	MD1.read(infile);
    MD2=MD1;
    MD1.setAnglePsi(3.1234);
    MD2.write("cc.doc");
    MD1.write("ccc.doc");
	exit(1);

*/

    Prog_angular_class_average_prm  prm;

    int              i, nmax, nr_ref, nr_images, reserve;
    double           ref_number, rot, tilt, psi, xshift, yshift, mirror;
    double           w, w1, w2;
    Matrix1D<double> dataline(8);

    // Get input parameters
    try
    {
        // Read command line & produce side info
        prm.read(argc, argv);
        prm.produceSideInfo();
        prm.show();

        // Only for do_add: append input docfile to add_to docfile
        if (prm.do_add)
        {
            FileName fn_tmp=prm.fn_out+".doc";
            if (exists(fn_tmp)) 
            {
                DocFile DFaux = prm.DF;
                // Don't do any fancy merging or sorting because those functions are really slow... 
                DFaux.append(fn_tmp);
                DFaux.remove_multiple_strings("Headerinfo");
                DFaux.write(fn_tmp);
            }
            else
            {
                prm.DF.write(fn_tmp); 
            }
        }
    }

    catch (Xmipp_error XE)
    {
        std::cout << XE;
        prm.usage();
        exit(0);
    }

    // Making class averages
    try
    {

        // Reserve memory for output from class realignment
        if (prm.nr_iter > 0) reserve = prm.DF.dataLineNo();
        else reserve = 0;
        double output_values[AVG_OUPUT_SIZE*reserve+1];

        nr_ref = prm.DFlib.dataLineNo();
        init_progress_bar(nr_ref);

        // Loop over all classes

        for (int dirno = 1; dirno <= nr_ref; dirno++)
        {   
            // Do the actual work
            prm.processOneClass(dirno, output_values);
            
            // Output classes sel and doc files
            w = output_values[1];
            w1 = output_values[2];
            w2 = output_values[3];
            prm.addClassAverage(dirno,w,w1,w2);

            // Fill new docfile (with params after realignment)
            if (prm.nr_iter > 0)
            {
                nr_images = ROUND(output_values[4] / AVG_OUPUT_SIZE);
                for (int i = 0; i < nr_images; i++)
                {
                    int this_image = ROUND(output_values[i*AVG_OUPUT_SIZE+5]);
                    if (!(this_image < 0))
                    {
                        prm.DF.locate(this_image);
                        prm.DF.set(0,output_values[i*AVG_OUPUT_SIZE+6]);
                        prm.DF.set(1,output_values[i*AVG_OUPUT_SIZE+7]);
                        prm.DF.set(2,output_values[i*AVG_OUPUT_SIZE+8]);
                        prm.DF.set(3,output_values[i*AVG_OUPUT_SIZE+9]);
                        prm.DF.set(4,output_values[i*AVG_OUPUT_SIZE+10]);
                        prm.DF.set(5,output_values[i*AVG_OUPUT_SIZE+11]);
                        prm.DF.set(6,output_values[i*AVG_OUPUT_SIZE+12]);
                        prm.DF.set(7,output_values[i*AVG_OUPUT_SIZE+13]);
                    }
                }
            }

            progress_bar(dirno);
            
        }
        progress_bar(nr_ref);
        
        // Write selfiles and docfiles with all class averages
        prm.finalWriteToDisc();

    }
    catch (Xmipp_error XE)
    {
        std::cout << XE;
    }

}
