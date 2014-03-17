#!/usr/bin/env xmipp_python
#------------------------------------------------------------------------------------------------
#Wrapper for relion 1.2

#   Author: Roberto Marabini
#

from os.path import join, exists
from os import remove, rename, environ
from protlib_base import protocolMain
from config_protocols import protDict
from xmipp import *
from xmipp import MetaDataInfo
from protlib_utils import runShowJ, getListFromVector, getListFromRangeString, \
                          runJob, runChimera, which, runChimeraClient
from protlib_parser import ProtocolParser
from protlib_xmipp import redStr, cyanStr
from protlib_gui_ext import showWarning, showTable, showError
from protlib_filesystem import xmippExists, findAcquisitionInfo, moveFile, \
                               replaceBasenameExt
from protocol_ml2d import lastIteration
from protlib_filesystem import createLink
from protlib_gui_figure import XmippPlotter
from protlib_gui_ext import showWarning, showTable, showError

from protlib_relion import *


class ProtRelionRefinner( ProtRelionBase):
    def __init__(self, scriptname, project):
        ProtRelionBase.__init__(self, protDict.relion_refine.name, scriptname, project)
        self.Import = 'from protocol_relion_refine import *'
        self.NumberOfClasses = 1
        self.NumberOfIterations = 1
        self.DisplayRef3DNo = 1
        
        if self.DoContinue:
            #if optimizer has not been properly selected this will 
            #fail, let us go ahead and handle the situation in verify
            try:
                self.inputProperty('MaskRadiusA')
                self.inputProperty('NumberOfClasses')
                self.inputProperty('Ref3D')
            except:
                print "Can not access the parameters from the original relion run"

    def _summary(self):
        lastIteration = self.lastIter()
        return ['Performed <%d> iterations ' % lastIteration]

    def _summaryContinue(self):
        lastIteration = self.lastIter()
        firstIteration = getIteration(self.optimiserFileName)
        lines = ['Continuation from run: <%s>, iter: <%d>' % (self.PrevRunName, firstIteration)]
        if (lastIteration - firstIteration) < 0 :
            performedIteration=0
        else:
            performedIteration=lastIteration - firstIteration
        lines.append('Performed <%d> iterations (number estimated from the files in working directory)' % performedIteration)
        lines.append('Input fileName = <%s>'%self.optimiserFileName)
        return lines

    def papers(self):
        return ProtRelionBase.papers(self) + ['Gold FSC: Scheres & Chen, Nat Meth. (2012) [http://www.nature.com/nmeth/journal/v9/n9/abs/nmeth.2115.html]']

    def validate(self):
        errors = ProtRelionBase.validate(self)
        if self.NumberOfMpi < 3:
            errors.append('Relion refine requires at least 3 MPI processes to compute gold standard')
        return errors 
    
    def _validateContinue(self):
        lastIterationPrecRun = self.PrevRun.lastIter()
        errors = []
        if '3D/RelionRef' not in self.optimiserFileName:
            message = 'File <%s> has not been generated by a Relion refine protocol' % self.optimiserFileName
            if '3D/RelionClass' in self.optimiserFileName:
                message += " but by relion classify"
            errors += [message]
        return errors 
                       
    def _insertSteps(self):
        args = {#'--iter': self.NumberOfIterations,
                #'--tau2_fudge': self.RegularisationParamT,
                '--flatten_solvent': '',
                #'--zero_mask': '',# this is an option but is almost always true
                '--norm': '',
                '--scale': '',
                '--o': '%s/relion' % self.ExtraDir
                }
        if getattr(self, 'ReferenceMask', '').strip():
            args['--solvent_mask'] = self.ReferenceMask.strip()
            
        if getattr(self, 'SolventMask', '').strip():
            args['--solvent_mask2'] = self.SolventMask.strip()
            
        args.update({'--i': self.ImgStar,
                     '--particle_diameter': self.MaskRadiusA*2.,
                     '--angpix': self.SamplingRate,
                     '--ref': self.Ref3D,
                     '--oversampling': '1'
                     })
        
        if not self.IsMapAbsoluteGreyScale:
            args[' --firstiter_cc'] = '' 
            
        if self.InitialLowPassFilterA > 0:
            args['--ini_high'] = self.InitialLowPassFilterA
            
        # CTF stuff
        if self.DoCTFCorrection:
            args['--ctf'] = ''
        
        if self.HasReferenceCTFCorrected:
            args['--ctf_corrected_ref'] = ''
            
        if self.HaveDataPhaseFlipped:
            args['--ctf_phase_flipped'] = ''
            
        if self.IgnoreCTFUntilFirstPeak:
            args['--ctf_intact_first_peak'] = ''
            
        args['--sym'] = self.SymmetryGroup.upper()
        args['--auto_refine']=''
        args['--split_random_halves']=''
        
        if args['--sym'][:1] == 'C':
            args['--low_resol_join_halves'] = "40";
        #args['--K'] = self.NumberOfClasses
            
        # Sampling stuff
        # Find the index(starting at 0) of the selected
        # sampling rate, as used in relion program
        iover = 1 #TODO: check this DROP THIS
        
        index = HEALPIX_LIST.index(self.AngularSamplingDeg)
        args['--healpix_order'] = index + 1 - iover
        
        index = HEALPIX_LIST.index(self.LocalSearchAutoSamplingDeg)
        args['--auto_local_healpix_order'] = index + 1 - iover
        
        #if self.PerformLocalAngularSearch:
        #    args['--sigma_ang'] = self.LocalAngularSearchRange / 3.
            
        args['--offset_range'] = self.OffsetSearchRangePix
        args['--offset_step']  = self.OffsetSearchStepPix * pow(2, iover)

        args['--j'] = self.NumberOfThreads
        
        # Join in a single line all key, value pairs of the args dict    
        params = ' '.join(['%s %s' % (k, str(v)) for k, v in args.iteritems()])
        params += ' ' + self.AdditionalArguments

        self.insertRunJobStep(self.program, params, self._getVerifyFiles())

    def _insertStepsContinue(self):
        args = {
                '--o': '%s/relion' % self.ExtraDir,
                '--continue': self.optimiserFileName,
                
                '--flatten_solvent': '',# use always
                #'--zero_mask': '',# use always. This is confussing since Sjors gui creates the command line
                #                  # with this option
                #                  # but then the program complains about it. 
                '--oversampling': '1',
                '--norm': '',
                '--scale': '',
                }
        iover = 1 #TODO: check this DROP THIS

        args['--j'] = self.NumberOfThreads
        
        # Join in a single line all key, value pairs of the args dict    
        params = ' '.join(['%s %s' % (k, str(v)) for k, v in args.iteritems()])
        params += ' ' + self.AdditionalArguments
        
        self.insertRunJobStep(self.program, params, self._getVerifyFiles())

    def createFilenameTemplates(self):
        myDict = ProtRelionBase.createFilenameTemplates(self)
        
        extra = self.workingDirPath('extra')
        self.extraIter2 = join(extra, 'relion_')
        #myDict['volumeFinal']      = self.extraIter2 + "class%(ref3d)03d.spi"
        #myDict['volumeMRCFinal']   = self.extraIter2 + "class%(ref3d)03d.mrc:mrc"
        myDict['volume_final'] = self.extraPath("relion_class%(ref3d)03d.mrc:mrc")
        myDict['model_final'] = self.extraPath('relion_model.star')
        myDict['data_final'] = self.extraPath('relion_data.star')
        myDict['model_final_xmipp'] = self.extraPath("relion_model_xmipp.xmd")

        return myDict
 
    def _getVerifyFiles(self):
        """ Return the final files that should be produced. """
        return [self.getFilename(k + '_final', ref3d=1) for k in ['volume', 'data', 'model']]
            
    def _getFinalSuffix(self):
        return '_final'
    
    def _getExtraOutputs(self):
        #TODO we need extra inputs?
        ExtraInputs  = [ self.extraPath('relion_model.star')] 
        ExtraOutputs = [ self.extraPath('relion_model.xmd')]
        return ExtraOutputs         
        
    def _getChangeLabels(self):
        return [MDL_AVG_CHANGES_ORIENTATIONS, MDL_AVG_CHANGES_OFFSETS]                

    def _getPrefixes(self):
        return ['half1_', 'half2_']
    
    def _writeIterAngularDist(self, it):
        """ Write the angular distribution. Should be overriden in subclasses. """
        data_angularDist = self.getFilename('angularDist_xmipp', iter=it)
        data_star = self.getFilename('data', iter=it)
        md = MetaData(data_star)
        
        for group in [1, 2]:
            mdGroup = MetaData()
            mdGroup.importObjects(md, MDValueEQ(MDL_RANDOMSEED, group))
            mdDist = MetaData()
            mdDist.aggregateMdGroupBy(mdGroup, AGGR_COUNT, [MDL_ANGLE_ROT, MDL_ANGLE_TILT], MDL_ANGLE_ROT, MDL_WEIGHT)
            mdDist.setValueCol(MDL_ANGLE_PSI, 0.0)
            blockName = 'half%d_class%06d_angularDist@' % (group, 1)
            mdDist.write(blockName + data_angularDist, MD_APPEND)
          
    def _visualizeDisplayResolutionPlotsSSNR(self):
        ProtRelionBase._visualizeDisplayResolutionPlotsSSNR(self)
        finalModel = self.getFilename('model_final')
        if xmippExists(finalModel):
            xplotter = XmippPlotter()
            plot_title = 'SSNR for all images, final iteration'
            a = xplotter.createSubPlot(plot_title, 'Armstrongs^-1', 'log(SSNR)', yformat=False)
            blockName = 'model_class_%d@' % 1
            fn = blockName + finalModel
            self._plotSSNR(a, fn)
            a.grid(True)
            xplotter.show(True)
            
    def _visualizeDisplayResolutionPlotsFSC(self):
        ProtRelionBase._visualizeDisplayResolutionPlotsFSC(self)
        finalModel = self.getFilename('model_final')
        if xmippExists(finalModel):
            xplotter = XmippPlotter(windowTitle='FSC Final')
            plot_title = 'FSC for all images, final iteration'
            a = xplotter.createSubPlot(plot_title, 'Armstrongs^-1', 'FSC', yformat=False)
            model_star = 'model_class_1@' + finalModel
            self._plotFSC(a, model_star)
            if self.ResolutionThreshold < self.maxFrc:
                a.plot([self.minInv, self.maxInv],[self.ResolutionThreshold, self.ResolutionThreshold], color='black', linestyle='--')
            a.grid(True)
             
    def _visualizeDisplayFinalReconstruction(self):
        """ Visualize final 3D map.
        If show in slices, create a temporal single metadata
        for avoid opening multiple windows. 
        """
        prefixes = self._getPrefixes()
        _visualizeVolumesMode = self.parser.getTkValue('DisplayFinalReconstruction')
        filename = self.extraPath('relion_class001.mrc:mrc')
        if xmippExists(filename):
            #Chimera
            if _visualizeVolumesMode == 'chimera':
                print "filename", filename
                runChimeraClient(filename)
            else:
                try:
                    runShowJ(filename, extraParams=' --dont_wrap ')
                except Exception, e:
                    showError("Error launching xmipp_showj: ", str(e), self.master)
        else:
            showError("Can not visualize volume file <%s>\nit does not exists." % filename, self.master)
