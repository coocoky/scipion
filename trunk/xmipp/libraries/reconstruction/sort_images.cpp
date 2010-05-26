/***************************************************************************
 *
 * Authors:    Carlos Oscar           coss@cnb.csic.es (2010)
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

#include "sort_images.h"
#include <data/args.h>
#include <data/filters.h>
#include <data/mask.h>

// Read arguments ==========================================================
void Prog_sort_images_prm::read(int argc, char **argv)
{
    fnSel  = getParameter(argc, argv, "-i");
    fnRoot = getParameter(argc, argv, "-oroot");
    processSelfiles=checkParameter(argc, argv, "-processSelfiles");
}

// Show ====================================================================
void Prog_sort_images_prm::show()
{
    std::cerr << "Input selfile:    " << fnSel           << std::endl
              << "Output rootname:  " << fnRoot          << std::endl
              << "Process selfiles: " << processSelfiles << std::endl
    ;
}

// usage ===================================================================
void Prog_sort_images_prm::usage()
{
    std::cerr << "Usage:  " << std::endl
              << "   -i <selfile>       : selfile of images\n"
              << "   -oroot <rootname>  : output rootname\n"
              << "  [-processSelfiles]  : process selfiles\n"
    ;
}

// Produce side info  ======================================================
//#define DEBUG
void Prog_sort_images_prm::produceSideInfo()
{
    // Read input selfile and reference
    MetaData SF;
    SF.read(fnSel);
    int idx=0;
    FOR_ALL_OBJECTS_IN_METADATA(SF)
    {
        if (idx==0)
        {
            FileName fnFirst;
            SF.getValue(MDL_IMAGE,fnFirst);
            SFoutOriginal.addObject();
            SFoutOriginal.setValue(MDL_IMAGE,fnFirst);
            lastImage.read(fnFirst);
            centerImage(lastImage());
            lastImage.write(fnRoot+integerToString(1,5)+".xmp");
            SFout.addObject();
            SFout.setValue(MDL_IMAGE,lastImage.name());
        }
        else
        {
            FileName fnFirst;
            SF.getValue(MDL_IMAGE,fnFirst);
            toClassify.push_back(fnFirst);
        }
        idx++;
    }

    // Prepare mask
    mask.resize(lastImage());
    mask.setXmippOrigin();
    BinaryCircularMask(mask,XSIZE(lastImage())/2, INNER_MASK);
}

// Choose next image =======================================================
void Prog_sort_images_prm::chooseNextImage()
{
    int imax=toClassify.size();
    Image<double> bestImage;
    double bestCorr=-1;
    int bestIdx=-1;
    for (int i=0; i<imax; i++)
    {
        Image<double> Iaux;
        Iaux.read(toClassify[i]);
        Iaux().setXmippOrigin();
        
        // Choose between this image and its mirror
        MultidimArray<double> I, Imirror;
        I=Iaux();
        Imirror=I;
        Imirror.selfReverseX();
        Imirror.setXmippOrigin();
        
        Matrix2D<double> M;
        alignImages(lastImage(),I,M);
        alignImages(lastImage(),Imirror,M);
        double corr=correlation_index(lastImage(),I,&mask);
        double corrMirror=correlation_index(lastImage(),Imirror,&mask);
        if (corr>bestCorr)
        {
            bestCorr=corr;
            bestImage()=I;
            bestIdx=i;
        }
        if (corrMirror>bestCorr)
        {
            bestCorr=corrMirror;
            bestImage()=Imirror;
            bestIdx=i;
        }
    }
    
    SFoutOriginal.addObject();
    SFoutOriginal.setValue(MDL_IMAGE,toClassify[bestIdx]);
    bestImage.write(fnRoot+integerToString(SFoutOriginal.size(),5)+".xmp");
    toClassify.erase(toClassify.begin()+bestIdx);
    lastImage=bestImage;
    SFout.addObject();
    SFout.setValue(MDL_IMAGE,bestImage.name());
}

// Run  ====================================================================
void Prog_sort_images_prm::run()
{
    std::cout << "Images to go: ";
    while (toClassify.size()>0) 
    {
        chooseNextImage();
        std::cout << toClassify.size() << " ";
        std::cout.flush();
    }
    std::cout << toClassify.size() << std::endl;
    SFoutOriginal.write(fnRoot+".sel");
    SFout.write(fnRoot+"_aligned.sel");
    
    if (processSelfiles)
    {
        MetaData SFInfo;
        SFoutOriginal.firstObject();
        FOR_ALL_OBJECTS_IN_METADATA(SFout)
        {
            FileName fnOutOrig; SFoutOriginal.getValue(MDL_IMAGE,fnOutOrig);
            FileName fnOut; SFout.getValue(MDL_IMAGE,fnOut);
            FileName fnSel=fnOutOrig.without_extension()+".sel";
            MetaData SFaux;
            SFaux.read(fnSel);
            SFInfo.addObject();
            SFInfo.setValue(MDL_IMAGE,fnOut);
            SFInfo.setValue(MDL_IMAGE_ORIGINAL,fnOutOrig);
            SFInfo.setValue(MDL_IMAGE_CLASS_GROUP,fnSel);
            SFInfo.setValue(MDL_IMAGE_CLASS_COUNT,SFaux.size());
            SFoutOriginal.nextObject();
        }
        SFInfo.write(fnRoot+"_info.txt");
    }
}
