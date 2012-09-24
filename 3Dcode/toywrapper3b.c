
  void toywrapper3b( const char* input_datfile = "datfiles/Input-met3-ht3-v5-newqcdsyst-model4-exp0lep.dat",
                const char* input_susyfile = "datfiles/T1bbbb-met3-ht3-v5.dat",
                double input_mgl=1100, double input_mlsp=300.,
                const char* input_deffdbtagfile = "datfiles/dummy_DeffDbtag-met3-ht3.dat",
                double input_nSusy0lep = 200.,
                const char* input_outputDir = "output-toymc3b-test4-qcdmodel4-nsig200",
                int nToy = 10,
                const char* input_mcvals_rootfile = "rootfiles/gi-plots-met3-ht3-v5.root",
                bool useExpected0lep = true,
                int qcdModelIndex = 4,
                bool input_doSignif = false,
                bool input_doUL = false,
                const char* blindBinsList = "null"
      ) {
     gROOT->LoadMacro("RooRatio.cxx+") ;
     gROOT->LoadMacro("RooPosDefCorrGauss.cxx+") ;
     gROOT->LoadMacro("toymc3b.c+") ;
     toymc3b( input_datfile, input_susyfile, input_mgl, input_mlsp, input_deffdbtagfile, input_nSusy0lep, input_outputDir, nToy, input_mcvals_rootfile, useExpected0lep, qcdModelIndex, input_doSignif, input_doUL, blindBinsList ) ;
  }

