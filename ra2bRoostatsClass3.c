
//
//   Owen Long, UCR
//   Harrison Prosper, FSU
//   Sezen Sekmen, FSU
//
//


#include "ra2bRoostatsClass3.h"

#include <iostream>


#include "TCanvas.h"
#include "TStyle.h"
#include "THStack.h"
#include "TLegend.h"
#include "TText.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom1.h"
#include "TH2F.h"
#include "TGaxis.h"
#include "TLine.h"
#include "TStringLong.h"

#include "RooArgSet.h"
#include "RooConstVar.h"

#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooStats/LikelihoodIntervalPlot.h"

#include "LikelihoodIntervalPlot.cxx"

  using namespace RooFit ;
  using namespace RooStats ;


  //=====================================================================================================


   ra2bRoostatsClass3::ra2bRoostatsClass3( bool ArgUseSigTtwjVar, bool ArgUseLdpVars ) {

      gStyle->SetOptStat(0) ;

      useSigTtwjVar = ArgUseSigTtwjVar ;
      useLdpVars = ArgUseLdpVars ;

     //--- Tell RooFit to shut up about anything less important than an ERROR.
      RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
  /// RooMsgService::instance().addStream(DEBUG,Topic(Tracing),OutputFile("debug.log")) ;
      printf("\n\n ==== RooFit output configuration ===============\n") ;
      RooMsgService::instance().Print("v") ;
      printf("\n\n ================================================\n") ;

      varsAtFitVals = false ;
      initialized = false ;

   }




////==================================================================================================

//  bool ra2bRoostatsClass3::generateToyDatasetsFromLikelihood( const char* outputRootFile, int nToys ) {

//     if ( ! initialized ) {
//        printf("\n\n *** Call initialize first.\n\n") ;
//        return false ;
//     }

//     TTree* toyOutputTrueTree = new TTree("toytruetree", "ra2b toy mean true value tree" ) ;

//     toyOutputTrueTree->Branch("mu0_ttbar_sig"      , &toy_mu0_ttbar_sig, "mu0_ttbar_sig/D" ) ;
//     toyOutputTrueTree->Branch("mu0_qcd_sig"        , &toy_mu0_qcd_sig, "mu0_qcd_sig/D" ) ;
//     toyOutputTrueTree->Branch("mu0_ttbar_sb"       , &toy_mu0_ttbar_sb, "mu0_ttbar_sb/D" ) ;
//     toyOutputTrueTree->Branch("mu0_qcd_sb"         , &toy_mu0_qcd_sb, "mu0_qcd_sb/D" ) ;
//     toyOutputTrueTree->Branch("mu0_susy_sig"       , &toy_mu0_susy_sig       , "mu0_susy_sig/D" ) ;
//     toyOutputTrueTree->Branch("mu0_allbg_sig"      , &toy_mu0_allbg_sig      , "mu0_allbg_sig/D" ) ;


//    //--- Set the true values.
//     if ( useSigTtwjVar ) {
//        toy_mu0_ttbar_sig = rrv_mu_ttbar_sig->getVal() ;
//        toy_mu0_qcd_sig   = rrv_mu_qcd_sig->getVal() ;
//        toy_mu0_ttbar_sb  = rfv_mu_ttbar_sb->getVal() ;
//        toy_mu0_qcd_sb    = rfv_mu_qcd_sb->getVal() ;
//     } else {
//        toy_mu0_ttbar_sig = rfv_mu_ttbar_sig->getVal() ;
//        toy_mu0_qcd_sig   = rfv_mu_qcd_sig->getVal() ;
//        toy_mu0_ttbar_sb  = rrv_mu_ttbar_sb->getVal() ;
//        toy_mu0_qcd_sb    = rrv_mu_qcd_sb->getVal() ;
//     }
//     toy_mu0_susy_sig = rv_mu_susy_sig->getVal() ;
//     toy_mu0_allbg_sig = rv_mu_ew_sig->getVal() + toy_mu0_ttbar_sig + toy_mu0_qcd_sig ;
//     printf("\n\n") ;
//     printf("  Mean true values used in toy generation:\n") ;
//     printf("  mu0_ttbar_sig : %6.1f\n", toy_mu0_ttbar_sig ) ;
//     printf("  mu0_qcd_sig   : %6.1f\n", toy_mu0_qcd_sig ) ;
//     printf("  mu0_ttbar_sb  : %6.1f\n", toy_mu0_ttbar_sb ) ;
//     printf("  mu0_qcd_sb    : %6.1f\n", toy_mu0_qcd_sb ) ;
//     printf("  mu0_susy_sig  : %6.1f\n", toy_mu0_susy_sig ) ;
//     printf("  mu0_allbg_sig : %6.1f\n", toy_mu0_allbg_sig ) ;

//   //--- generate some toy datasets.
//     printf("\n\n Opening output toy datasets root file : %s\n", outputRootFile ) ;
//     TFile toyOutputFile( outputRootFile, "recreate" ) ;
//     RooDataSet* toyDatasets = likelihood->generate( observedParametersList, nToys ) ;
//     const TTree* toyTree = toyDatasets->tree() ;
//     toyTree->Write() ;
//     toyOutputTrueTree->Fill() ;
//     toyOutputTrueTree->Write() ;
//     printf("\n\n Closing output toy datasets root file : %s\n", outputRootFile ) ;
//     toyOutputFile.Close() ;

//     return true ;

//  } // generateToyDatasetsFromLikelihood

////==================================================================================================

//  bool ra2bRoostatsClass3::generateToyDatasetsFromNVals( const char* outputRootFile, int nToys ) {

//     if ( ! initialized ) {
//        printf("\n\n *** Call initialize first.\n\n") ;
//        return false ;
//     }

//     TTree* toyOutputTrueTree = new TTree("toytruetree", "ra2b toy mean true value tree" ) ;

//     toyOutputTrueTree->Branch("mu0_ttbar_sig"      , &toy_mu0_ttbar_sig, "mu0_ttbar_sig/D" ) ;
//     toyOutputTrueTree->Branch("mu0_qcd_sig"        , &toy_mu0_qcd_sig, "mu0_qcd_sig/D" ) ;
//     toyOutputTrueTree->Branch("mu0_ttbar_sb"       , &toy_mu0_ttbar_sb, "mu0_ttbar_sb/D" ) ;
//     toyOutputTrueTree->Branch("mu0_qcd_sb"         , &toy_mu0_qcd_sb, "mu0_qcd_sb/D" ) ;
//     toyOutputTrueTree->Branch("mu0_susy_sig"       , &toy_mu0_susy_sig       , "mu0_susy_sig/D" ) ;
//     toyOutputTrueTree->Branch("mu0_allbg_sig"      , &toy_mu0_allbg_sig      , "mu0_allbg_sig/D" ) ;


//    //--- Set the true values.
//     if ( useSigTtwjVar ) {
//        toy_mu0_ttbar_sig = rrv_mu_ttbar_sig->getVal() ;
//        toy_mu0_qcd_sig   = rrv_mu_qcd_sig->getVal() ;
//        toy_mu0_ttbar_sb  = rfv_mu_ttbar_sb->getVal() ;
//        toy_mu0_qcd_sb    = rfv_mu_qcd_sb->getVal() ;
//     } else {
//        toy_mu0_ttbar_sig = rfv_mu_ttbar_sig->getVal() ;
//        toy_mu0_qcd_sig   = rfv_mu_qcd_sig->getVal() ;
//        toy_mu0_ttbar_sb  = rrv_mu_ttbar_sb->getVal() ;
//        toy_mu0_qcd_sb    = rrv_mu_qcd_sb->getVal() ;
//     }
//     toy_mu0_susy_sig = rv_mu_susy_sig->getVal() ;
//     toy_mu0_allbg_sig = rv_mu_ew_sig->getVal() + toy_mu0_ttbar_sig + toy_mu0_qcd_sig ;
//     printf("\n\n") ;
//     printf("  Mean true values used in toy generation:\n") ;
//     printf("  mu0_ttbar_sig : %6.1f\n", toy_mu0_ttbar_sig ) ;
//     printf("  mu0_qcd_sig   : %6.1f\n", toy_mu0_qcd_sig ) ;
//     printf("  mu0_ttbar_sb  : %6.1f\n", toy_mu0_ttbar_sb ) ;
//     printf("  mu0_qcd_sb    : %6.1f\n", toy_mu0_qcd_sb ) ;
//     printf("  mu0_susy_sig  : %6.1f\n", toy_mu0_susy_sig ) ;
//     printf("  mu0_allbg_sig : %6.1f\n", toy_mu0_allbg_sig ) ;

//   //--- generate some toy datasets.
//     printf("\n\n Opening output toy datasets root file : %s\n", outputRootFile ) ;
//     TTree* toyTree = new TTree("likelihoodData", "ra2b observed values generated from Nvals") ;

//     double Nsig, Na, Nd,
//            Nsb1, Nsb2, Nsb3, Nsb4, Nsb5,
//            Nlsb1, Nlsb2, Nlsb3, Nlsb4, Nlsb5,
//            Nslsig1, Nslsig2, Nslsig3, Nslsig4, Nslsig5,
//            Nslsb1, Nslsb2, Nslsb3, Nslsb4, Nslsb5,
//            Nslmsb1, Nslmsb2, Nslmsb3, Nslmsb4, Nslmsb5,
//            Nqcdmca, Nqcdmcd, Nqcdmcsig, Nqcdmcsb ;

//     toyTree->Branch("Nsig"       , &Nsig      , "Nsig/D" ) ;
//     toyTree->Branch("Na"         , &Na        , "Na/D" ) ;
//     toyTree->Branch("Nd"         , &Nd        , "Nd/D" ) ;
//     toyTree->Branch("Nsb1"       , &Nsb1      , "Nsb1/D" ) ;
//     toyTree->Branch("Nsb2"       , &Nsb2      , "Nsb2/D" ) ;
//     toyTree->Branch("Nsb3"       , &Nsb3      , "Nsb3/D" ) ;
//     toyTree->Branch("Nsb4"       , &Nsb4      , "Nsb4/D" ) ;
//     toyTree->Branch("Nsb5"       , &Nsb5      , "Nsb5/D" ) ;
//     toyTree->Branch("Nlsb1"      , &Nlsb1     , "Nlsb1/D" ) ;
//     toyTree->Branch("Nlsb2"      , &Nlsb2     , "Nlsb2/D" ) ;
//     toyTree->Branch("Nlsb3"      , &Nlsb3     , "Nlsb3/D" ) ;
//     toyTree->Branch("Nlsb4"      , &Nlsb4     , "Nlsb4/D" ) ;
//     toyTree->Branch("Nlsb5"      , &Nlsb5     , "Nlsb5/D" ) ;
//     toyTree->Branch("Nslsig1"    , &Nslsig1   , "Nslsig1/D" ) ;
//     toyTree->Branch("Nslsig2"    , &Nslsig2   , "Nslsig2/D" ) ;
//     toyTree->Branch("Nslsig3"    , &Nslsig3   , "Nslsig3/D" ) ;
//     toyTree->Branch("Nslsig4"    , &Nslsig4   , "Nslsig4/D" ) ;
//     toyTree->Branch("Nslsig5"    , &Nslsig5   , "Nslsig5/D" ) ;
//     toyTree->Branch("Nslsb1"     , &Nslsb1    , "Nslsb1/D" ) ;
//     toyTree->Branch("Nslsb2"     , &Nslsb2    , "Nslsb2/D" ) ;
//     toyTree->Branch("Nslsb3"     , &Nslsb3    , "Nslsb3/D" ) ;
//     toyTree->Branch("Nslsb4"     , &Nslsb4    , "Nslsb4/D" ) ;
//     toyTree->Branch("Nslsb5"     , &Nslsb5    , "Nslsb5/D" ) ;
//     toyTree->Branch("Nslmsb1"    , &Nslmsb1   , "Nslmsb1/D" ) ;
//     toyTree->Branch("Nslmsb2"    , &Nslmsb2   , "Nslmsb2/D" ) ;
//     toyTree->Branch("Nslmsb3"    , &Nslmsb3   , "Nslmsb3/D" ) ;
//     toyTree->Branch("Nslmsb4"    , &Nslmsb4   , "Nslmsb4/D" ) ;
//     toyTree->Branch("Nslmsb5"    , &Nslmsb5   , "Nslmsb5/D" ) ;
//     toyTree->Branch("Nqcdmca"    , &Nqcdmca   , "Nqcdmca/D" ) ;
//     toyTree->Branch("Nqcdmcd"    , &Nqcdmcd   , "Nqcdmcd/D" ) ;
//     toyTree->Branch("Nqcdmcsig"  , &Nqcdmcsig , "Nqcdmcsig/D" ) ;
//     toyTree->Branch("Nqcdmcsb"   , &Nqcdmcsb  , "Nqcdmcsb/D" ) ;

//     TRandom1 rg(12345) ;

//     for ( int dsi=0; dsi<nToys; dsi++ ) {

//        Nsig      = rg.Poisson( rv_Nsig      ->getVal() ) ;
//        Na        = rg.Poisson( rv_Na        ->getVal() ) ;
//        Nd        = rg.Poisson( rv_Nd        ->getVal() ) ;
//        Nsb1      = rg.Poisson( rv_Nsb1      ->getVal() ) ;
//        Nsb2      = rg.Poisson( rv_Nsb2      ->getVal() ) ;
//        Nsb3      = rg.Poisson( rv_Nsb3      ->getVal() ) ;
//        Nsb4      = rg.Poisson( rv_Nsb4      ->getVal() ) ;
//        Nsb5      = rg.Poisson( rv_Nsb5      ->getVal() ) ;
//        Nlsb1     = rg.Poisson( rv_Nlsb1     ->getVal() ) ;
//        Nlsb2     = rg.Poisson( rv_Nlsb2     ->getVal() ) ;
//        Nlsb3     = rg.Poisson( rv_Nlsb3     ->getVal() ) ;
//        Nlsb4     = rg.Poisson( rv_Nlsb4     ->getVal() ) ;
//        Nlsb5     = rg.Poisson( rv_Nlsb5     ->getVal() ) ;
//        Nslsig1   = rg.Poisson( rv_Nslsig1   ->getVal() ) ;
//        Nslsig2   = rg.Poisson( rv_Nslsig2   ->getVal() ) ;
//        Nslsig3   = rg.Poisson( rv_Nslsig3   ->getVal() ) ;
//        Nslsig4   = rg.Poisson( rv_Nslsig4   ->getVal() ) ;
//        Nslsig5   = rg.Poisson( rv_Nslsig5   ->getVal() ) ;
//        Nslsb1    = rg.Poisson( rv_Nslsb1    ->getVal() ) ;
//        Nslsb2    = rg.Poisson( rv_Nslsb2    ->getVal() ) ;
//        Nslsb3    = rg.Poisson( rv_Nslsb3    ->getVal() ) ;
//        Nslsb4    = rg.Poisson( rv_Nslsb4    ->getVal() ) ;
//        Nslsb5    = rg.Poisson( rv_Nslsb5    ->getVal() ) ;
//        Nslmsb1   = rg.Poisson( rv_Nslmsb1   ->getVal() ) ;
//        Nslmsb2   = rg.Poisson( rv_Nslmsb2   ->getVal() ) ;
//        Nslmsb3   = rg.Poisson( rv_Nslmsb3   ->getVal() ) ;
//        Nslmsb4   = rg.Poisson( rv_Nslmsb4   ->getVal() ) ;
//        Nslmsb5   = rg.Poisson( rv_Nslmsb5   ->getVal() ) ;
//        Nqcdmca   = rg.Poisson( rv_Nqcdmca   ->getVal() ) ;
//        Nqcdmcd   = rg.Poisson( rv_Nqcdmcd   ->getVal() ) ;
//        Nqcdmcsig = rg.Poisson( rv_Nqcdmcsig ->getVal() ) ;
//        Nqcdmcsb  = rg.Poisson( rv_Nqcdmcsb  ->getVal() ) ;

//        toyTree->Fill() ;

//     } // dsi.


//     TFile toyOutputFile( outputRootFile, "recreate" ) ;
//     toyTree->Write() ;
//     toyOutputTrueTree->Fill() ;
//     toyOutputTrueTree->Write() ;
//     printf("\n\n Closing output toy datasets root file : %s\n", outputRootFile ) ;
//     toyOutputFile.Close() ;

//     return true ;

//  } // generateToyDatasetsFromNVals

  //==================================================================================================

    bool ra2bRoostatsClass3::doFit( ) {

       if ( ! initialized ) {
          printf("\n\n *** Call initialize first.\n\n") ;
          return false ;
       }

       printf("\n\n") ;
       printf("  Fitting with these values for the observables.\n") ;
       dsObserved->printMultiline(cout, 1, kTRUE, "") ;
       printf("\n\n") ;

       fitResult = likelihood->fitTo(*dsObserved, Save(true));

       printf("\n\n----- Constant parameters:\n") ;
       RooArgList constPars = fitResult->constPars() ;
       for ( int pi=0; pi<constPars.getSize(); pi++ ) {
          constPars[pi].Print() ;
       } // pi.

       printf("\n\n----- Floating parameters:\n") ;
       RooArgList floatPars = fitResult->floatParsFinal() ;
       for ( int pi=0; pi<floatPars.getSize(); pi++ ) {
          floatPars[pi].Print() ;
       } // pi.
       printf("\n\n") ;



//     float Amc = rv_mu_qcdmc_a->getVal() ;
//     float Dmc = rv_mu_qcdmc_d->getVal() ;
//     float Bmc = rv_mu_qcdmc_sb->getVal() ;
//     float Cmc = rv_mu_qcdmc_sig->getVal() ;

//     float K = Amc * Cmc / ( Bmc * Dmc ) ;

//     qcdCorrection = K ;

//     float covAA = pow( rv_mu_qcdmc_a->getError(), 2 ) ;
//     float covDD = pow( rv_mu_qcdmc_d->getError(), 2 ) ;
//     float covBB = pow( rv_mu_qcdmc_sb->getError(), 2 ) ;
//     float covCC = pow( rv_mu_qcdmc_sig->getError(), 2 ) ;

//     float rhoAB = fitResult->correlation( "mu_qcdmc_a", "mu_qcdmc_sb" ) ;
//     float rhoAC = fitResult->correlation( "mu_qcdmc_a", "mu_qcdmc_sig" ) ;
//     float rhoAD = fitResult->correlation( "mu_qcdmc_a", "mu_qcdmc_d" ) ;

//     float rhoBC = fitResult->correlation( "mu_qcdmc_sb", "mu_qcdmc_sig" ) ;
//     float rhoBD = fitResult->correlation( "mu_qcdmc_sb", "mu_qcdmc_d" ) ;

//     float rhoCD = fitResult->correlation( "mu_qcdmc_sig", "mu_qcdmc_d" ) ;

//     float covAB = rhoAB * sqrt( covAA * covBB ) ;
//     float covAC = rhoAC * sqrt( covAA * covCC ) ;
//     float covAD = rhoAD * sqrt( covAA * covDD ) ;

//     float covBC = rhoBC * sqrt( covBB * covCC ) ;
//     float covBD = rhoBD * sqrt( covBB * covDD ) ;

//     float covCD = rhoCD * sqrt( covCC * covDD ) ;

//     qcdCorrectionErr = K * sqrt( 
//            covAA/(Amc*Amc) + covBB/(Bmc*Bmc) + covCC/(Cmc*Cmc) + covDD/(Dmc*Dmc)
//            -2 * covAB / (Amc*Bmc) +2 * covAC / (Amc*Cmc) -2 * covAD / (Amc*Dmc)
//            +2 * covBC / (Bmc*Cmc) -2 * covBD / (Bmc*Dmc) 
//            -2 * covCD / (Cmc*Dmc)
//     ) ;


//     printf("  QCD bias correction: %4.2f +/- %4.2f\n", qcdCorrection, qcdCorrectionErr ) ;

       varsAtFitVals = true ;

       return true ;

     } // doFit .


  //==================================================================================================

     bool ra2bRoostatsClass3::profileSusySig( float& susySigLow, float& susySigHigh, bool makePlot ) {

         if ( ! initialized ) {
            printf("\n\n *** Call initialize first.\n\n") ;
            return false ;
         }

      //--- Profile likelihood for signal susy yield.

         ProfileLikelihoodCalculator plc_susy_sig( *dsObserved, *likelihood, RooArgSet( *rv_mu_susy_sig ) ) ;
         plc_susy_sig.SetTestSize(0.05) ;
         ConfInterval* plinterval_susy_sig = plc_susy_sig.GetInterval() ;
         susySigLow  = ((LikelihoodInterval*) plinterval_susy_sig)->LowerLimit(*rv_mu_susy_sig) ;
         susySigHigh = ((LikelihoodInterval*) plinterval_susy_sig)->UpperLimit(*rv_mu_susy_sig) ;
         printf("\n\n") ;
         printf("    susy, SIG 95%% CL interval  [%5.1f, %5.1f]\n\n", susySigLow, susySigHigh ) ;

         if ( makePlot ) {
            TCanvas* plcplot_susy_sig = new TCanvas("plcplot_susy_sig", "susy sig, Profile likelihood", 500, 400 ) ;
            LikelihoodIntervalPlot plotInt_susy_sig((LikelihoodInterval*)plinterval_susy_sig);
            plotInt_susy_sig.Draw() ;
            plcplot_susy_sig->SaveAs("plscan_susy_sig.pdf") ;
            plcplot_susy_sig->SaveAs("plscan_susy_sig.png") ;
         }

         varsAtFitVals = false ;

         delete plinterval_susy_sig ; // can I safely do this???

         return true ;

     }

  //==================================================================================================

     bool ra2bRoostatsClass3::profileZnnSig( float& znnSigLow, float& znnSigHigh, bool makePlot ) {

         if ( ! initialized ) {
            printf("\n\n *** Call initialize first.\n\n") ;
            return false ;
         }

      //--- Profile likelihood for signal Z to nunu yield.

         ProfileLikelihoodCalculator plc_znn_sig( *dsObserved, *likelihood, RooArgSet( *rv_mu_znn_sig ) ) ;
         plc_znn_sig.SetTestSize(0.05) ;
         ConfInterval* plinterval_znn_sig = plc_znn_sig.GetInterval() ;
         znnSigLow  = ((LikelihoodInterval*) plinterval_znn_sig)->LowerLimit(*rv_mu_znn_sig) ;
         znnSigHigh = ((LikelihoodInterval*) plinterval_znn_sig)->UpperLimit(*rv_mu_znn_sig) ;
         printf("\n\n") ;
         printf("    znn, SIG 95%% CL interval  [%5.1f, %5.1f]\n\n", znnSigLow, znnSigHigh ) ;

         if ( makePlot ) {
            TCanvas* plcplot_znn_sig = new TCanvas("plcplot_znn_sig", "znn sig, Profile likelihood", 500, 400 ) ;
            LikelihoodIntervalPlot plotInt_znn_sig((LikelihoodInterval*)plinterval_znn_sig);
            plotInt_znn_sig.Draw() ;
            plcplot_znn_sig->SaveAs("plscan_znn_sig.pdf") ;
            plcplot_znn_sig->SaveAs("plscan_znn_sig.png") ;
         }

         varsAtFitVals = false ;

         delete plinterval_znn_sig ; // can I safely do this???

         return true ;

     }

  //==================================================================================================

     bool ra2bRoostatsClass3::profileqcdSig( float& qcdSigLow, float& qcdSigHigh, bool makePlot ) {

         if ( ! initialized ) {
            printf("\n\n *** Call initialize first.\n\n") ;
            return false ;
         }

         if ( useLdpVars ) {
            printf("\n\n *** Try again with useLdpVars set to false in the constructor.\n\n") ;
            return false ;
         }

      //--- Profile likelihood for signal Z to nunu yield.

         ProfileLikelihoodCalculator plc_qcd_sig( *dsObserved, *likelihood, RooArgSet( *rrv_mu_qcd_sig ) ) ;
         plc_qcd_sig.SetTestSize(0.05) ;
         ConfInterval* plinterval_qcd_sig = plc_qcd_sig.GetInterval() ;
         qcdSigLow  = ((LikelihoodInterval*) plinterval_qcd_sig)->LowerLimit(*rrv_mu_qcd_sig) ;
         qcdSigHigh = ((LikelihoodInterval*) plinterval_qcd_sig)->UpperLimit(*rrv_mu_qcd_sig) ;
         printf("\n\n") ;
         printf("    qcd, SIG 95%% CL interval  [%5.1f, %5.1f]\n\n", qcdSigLow, qcdSigHigh ) ;

         if ( makePlot ) {
            TCanvas* plcplot_qcd_sig = new TCanvas("plcplot_qcd_sig", "qcd sig, Profile likelihood", 500, 400 ) ;
            LikelihoodIntervalPlot plotInt_qcd_sig((LikelihoodInterval*)plinterval_qcd_sig);
            plotInt_qcd_sig.Draw() ;
            plcplot_qcd_sig->SaveAs("plscan_qcd_sig.pdf") ;
            plcplot_qcd_sig->SaveAs("plscan_qcd_sig.png") ;
         }

         varsAtFitVals = false ;

         delete plinterval_qcd_sig ; // can I safely do this???

         return true ;

     }

  //==================================================================================================
     bool ra2bRoostatsClass3::profileqcdSb( float& qcdSbLow, float& qcdSbHigh, bool makePlot ) {

         if ( ! initialized ) {
            printf("\n\n *** Call initialize first.\n\n") ;
            return false ;
         }

         if ( useLdpVars ) {
            printf("\n\n *** Try again with useLdpVars set to false in the constructor.\n\n") ;
            return false ;
         }

      //--- Profile likelihood for signal Z to nunu yield.

         ProfileLikelihoodCalculator plc_qcd_sb( *dsObserved, *likelihood, RooArgSet( *rrv_mu_qcd_sb ) ) ;
         plc_qcd_sb.SetTestSize(0.05) ;
         ConfInterval* plinterval_qcd_sb = plc_qcd_sb.GetInterval() ;
         qcdSbLow  = ((LikelihoodInterval*) plinterval_qcd_sb)->LowerLimit(*rrv_mu_qcd_sb) ;
         qcdSbHigh = ((LikelihoodInterval*) plinterval_qcd_sb)->UpperLimit(*rrv_mu_qcd_sb) ;
         printf("\n\n") ;
         printf("    qcd, sb 95%% CL interval  [%5.1f, %5.1f]\n\n", qcdSbLow, qcdSbHigh ) ;

         if ( makePlot ) {
            TCanvas* plcplot_qcd_sb = new TCanvas("plcplot_qcd_sb", "qcd sb, Profile likelihood", 500, 400 ) ;
            LikelihoodIntervalPlot plotInt_qcd_sb((LikelihoodInterval*)plinterval_qcd_sb);
            plotInt_qcd_sb.Draw() ;
            plcplot_qcd_sb->SaveAs("plscan_qcd_sb.pdf") ;
            plcplot_qcd_sb->SaveAs("plscan_qcd_sb.png") ;
         }

         varsAtFitVals = false ;

         delete plinterval_qcd_sb ; // can I safely do this???

         return true ;

     }

  //==================================================================================================

     bool ra2bRoostatsClass3::profilettwjSig( float& ttwjSigLow, float& ttwjSigHigh, bool makePlot ) {

         if ( ! initialized ) {
            printf("\n\n *** Call initialize first.\n\n") ;
            return false ;
         }

         if ( !useSigTtwjVar ) {
            printf("\n\n *** Try again with useSigTtwjVar set to true in the constructor.\n\n") ;
            return false ;
         }

      //--- Profile likelihood for signal Z to nunu yield.

         ProfileLikelihoodCalculator plc_ttwj_sig( *dsObserved, *likelihood, RooArgSet( *rrv_mu_ttwj_sig ) ) ;
         plc_ttwj_sig.SetTestSize(0.05) ;
         ConfInterval* plinterval_ttwj_sig = plc_ttwj_sig.GetInterval() ;
         ttwjSigLow  = ((LikelihoodInterval*) plinterval_ttwj_sig)->LowerLimit(*rrv_mu_ttwj_sig) ;
         ttwjSigHigh = ((LikelihoodInterval*) plinterval_ttwj_sig)->UpperLimit(*rrv_mu_ttwj_sig) ;
         printf("\n\n") ;
         printf("    ttwj, SIG 95%% CL interval  [%5.1f, %5.1f]\n\n", ttwjSigLow, ttwjSigHigh ) ;

         if ( makePlot ) {
            TCanvas* plcplot_ttwj_sig = new TCanvas("plcplot_ttwj_sig", "ttwj sig, Profile likelihood", 500, 400 ) ;
            LikelihoodIntervalPlot plotInt_ttwj_sig((LikelihoodInterval*)plinterval_ttwj_sig);
            plotInt_ttwj_sig.Draw() ;
            plcplot_ttwj_sig->SaveAs("plscan_ttwj_sig.pdf") ;
            plcplot_ttwj_sig->SaveAs("plscan_ttwj_sig.png") ;
         }

         varsAtFitVals = false ;

         delete plinterval_ttwj_sig ; // can I safely do this???

         return true ;

     }

  //==================================================================================================
     bool ra2bRoostatsClass3::profilettwjSb( float& ttwjSbLow, float& ttwjSbHigh, bool makePlot ) {

         if ( ! initialized ) {
            printf("\n\n *** Call initialize first.\n\n") ;
            return false ;
         }

         if ( useSigTtwjVar ) {
            printf("\n\n *** Try again with useSigTtwjVar set to false in the constructor.\n\n") ;
            return false ;
         }

      //--- Profile likelihood for signal Z to nunu yield.

         ProfileLikelihoodCalculator plc_ttwj_sb( *dsObserved, *likelihood, RooArgSet( *rrv_mu_ttwj_sb ) ) ;
         plc_ttwj_sb.SetTestSize(0.05) ;
         ConfInterval* plinterval_ttwj_sb = plc_ttwj_sb.GetInterval() ;
         ttwjSbLow  = ((LikelihoodInterval*) plinterval_ttwj_sb)->LowerLimit(*rrv_mu_ttwj_sb) ;
         ttwjSbHigh = ((LikelihoodInterval*) plinterval_ttwj_sb)->UpperLimit(*rrv_mu_ttwj_sb) ;
         printf("\n\n") ;
         printf("    ttwj, sb 95%% CL interval  [%5.1f, %5.1f]\n\n", ttwjSbLow, ttwjSbHigh ) ;

         if ( makePlot ) {
            TCanvas* plcplot_ttwj_sb = new TCanvas("plcplot_ttwj_sb", "ttwj sb, Profile likelihood", 500, 400 ) ;
            LikelihoodIntervalPlot plotInt_ttwj_sb((LikelihoodInterval*)plinterval_ttwj_sb);
            plotInt_ttwj_sb.Draw() ;
            plcplot_ttwj_sb->SaveAs("plscan_ttwj_sb.pdf") ;
            plcplot_ttwj_sb->SaveAs("plscan_ttwj_sb.png") ;
         }

         varsAtFitVals = false ;

         delete plinterval_ttwj_sb ; // can I safely do this???

         return true ;

     }

  //==================================================================================================

     bool ra2bRoostatsClass3::profileZnnSb( float& znnSbLow, float& znnSbHigh, bool makePlot ) {

         if ( ! initialized ) {
            printf("\n\n *** Call initialize first.\n\n") ;
            return false ;
         }

      //--- Profile likelihood for Z to nunu SB yield.

         ProfileLikelihoodCalculator plc_znn_sb( *dsObserved, *likelihood, RooArgSet( *rv_mu_znn_sb ) ) ;
         plc_znn_sb.SetTestSize(0.05) ;
         ConfInterval* plinterval_znn_sb = plc_znn_sb.GetInterval() ;
         znnSbLow  = ((LikelihoodInterval*) plinterval_znn_sb)->LowerLimit(*rv_mu_znn_sb) ;
         znnSbHigh = ((LikelihoodInterval*) plinterval_znn_sb)->UpperLimit(*rv_mu_znn_sb) ;
         printf("\n\n") ;
         printf("    znn, SB 95%% CL interval  [%5.1f, %5.1f]\n\n", znnSbLow, znnSbHigh ) ;

         if ( makePlot ) {
            TCanvas* plcplot_znn_sb = new TCanvas("plcplot_znn_sb", "znn sb, Profile likelihood", 500, 400 ) ;
            LikelihoodIntervalPlot plotInt_znn_sb((LikelihoodInterval*)plinterval_znn_sb);
            plotInt_znn_sb.Draw() ;
            plcplot_znn_sb->SaveAs("plscan_znn_sb.pdf") ;
            plcplot_znn_sb->SaveAs("plscan_znn_sb.png") ;
         }

         varsAtFitVals = false ;

         delete plinterval_znn_sb ; // can I safely do this???

         return true ;

     }

////==================================================================================================

//   bool ra2bRoostatsClass3::profileTtbarSig( float& ttbarSigLow, float& ttbarSigHigh ) {

//       if ( ! initialized ) {
//          printf("\n\n *** Call initialize first.\n\n") ;
//          return false ;
//       }

//       if ( ! useSigTtwjVar ) {
//          printf("\n\n\n *** Can't do it.  Need to use ra2bRoostatsClass3 with constructor arg useSigTtwjVar set to true.\n\n") ;
//          return false ;
//       }

//    //--- Profile likelihood for SIG ttbar yield.

//       ProfileLikelihoodCalculator plc_ttbar_sig( *dsObserved, *likelihood, RooArgSet( *rv_mu_ttbar_sig ) ) ;
//       plc_ttbar_sig.SetTestSize(0.32) ;
//       ConfInterval* plinterval_ttbar_sig = plc_ttbar_sig.GetInterval() ;
//       ttbarSigLow  = ((LikelihoodInterval*) plinterval_ttbar_sig)->LowerLimit(*rrv_mu_ttbar_sig) ;
//       ttbarSigHigh = ((LikelihoodInterval*) plinterval_ttbar_sig)->UpperLimit(*rrv_mu_ttbar_sig) ;
//       printf("\n\n") ;
//       printf("    ttbar, SIG 68%% CL interval  [%5.1f, %5.1f]\n\n", ttbarSigLow, ttbarSigHigh ) ;
//       TCanvas* plcplot_ttbar_sig = new TCanvas("plcplot_ttbar_sig", "ttbar sig, Profile likelihood", 500, 400 ) ;
//       LikelihoodIntervalPlot plotInt_ttbar_sig((LikelihoodInterval*)plinterval_ttbar_sig);
//       plotInt_ttbar_sig.Draw() ;
//       plcplot_ttbar_sig->SaveAs("plscan_ttbar_sig.pdf") ;
//       plcplot_ttbar_sig->SaveAs("plscan_ttbar_sig.png") ;

//       varsAtFitVals = false ;

//       return true ;

//   }

////==================================================================================================

//   bool ra2bRoostatsClass3::profileQcdSig( float& qcdSigLow, float& qcdSigHigh ) {

//       if ( ! initialized ) {
//          printf("\n\n *** Call initialize first.\n\n") ;
//          return false ;
//       }


//    //--- Profile likelihood for SIG qcd yield.

//       ProfileLikelihoodCalculator plc_qcd_sig( *dsObserved, *likelihood, RooArgSet( *rv_mu_qcd_sig ) ) ;
//       plc_qcd_sig.SetTestSize(0.32) ;
//       ConfInterval* plinterval_qcd_sig = plc_qcd_sig.GetInterval() ;
//       qcdSigLow  = ((LikelihoodInterval*) plinterval_qcd_sig)->LowerLimit(*rv_mu_qcd_sig) ;
//       qcdSigHigh = ((LikelihoodInterval*) plinterval_qcd_sig)->UpperLimit(*rv_mu_qcd_sig) ;
//       printf("\n\n") ;
//       printf("    qcd, SIG 68%% CL interval  [%5.1f, %5.1f]\n\n", qcdSigLow, qcdSigHigh ) ;
//       TCanvas* plcplot_qcd_sig = new TCanvas("plcplot_qcd_sig", "qcd sig, Profile likelihood", 500, 400 ) ;
//       LikelihoodIntervalPlot plotInt_qcd_sig((LikelihoodInterval*)plinterval_qcd_sig);
//       plotInt_qcd_sig.Draw() ;
//       plcplot_qcd_sig->SaveAs("plscan_qcd_sig.pdf") ;
//       plcplot_qcd_sig->SaveAs("plscan_qcd_sig.png") ;

//       varsAtFitVals = false ;

//       return true ;

//   }

  //==================================================================================================


//   bool ra2bRoostatsClass3::profileTtbarSb( float& ttbarSbLow, float& ttbarSbHigh ) {

//       if ( ! initialized ) {
//          printf("\n\n *** Call initialize first.\n\n") ;
//          return false ;
//       }

//       if ( useSigTtwjVar ) {
//          printf("\n\n\n *** Can't do it.  Need to use ra2bRoostatsClass3 with constructor arg useSigTtwjVar set to false.\n\n") ;
//          return false ;
//       }

//    //--- Profile likelihood for SB ttbar yield.

//       ProfileLikelihoodCalculator plc_ttbar_sb( *dsObserved, *likelihood, RooArgSet( *rv_mu_ttbar_sb ) ) ;
//       plc_ttbar_sb.SetTestSize(0.32) ;
//       ConfInterval* plinterval_ttbar_sb = plc_ttbar_sb.GetInterval() ;
//       ttbarSbLow  = ((LikelihoodInterval*) plinterval_ttbar_sb)->LowerLimit(*rrv_mu_ttbar_sb) ;
//       ttbarSbHigh = ((LikelihoodInterval*) plinterval_ttbar_sb)->UpperLimit(*rrv_mu_ttbar_sb) ;
//       printf("\n\n") ;
//       printf("    ttbar, SB 68%% CL interval  [%5.1f, %5.1f]\n\n", ttbarSbLow, ttbarSbHigh ) ;
//       TCanvas* plcplot_ttbar_sb = new TCanvas("plcplot_ttbar_sb", "ttbar sb, Profile likelihood", 500, 400 ) ;
//       LikelihoodIntervalPlot plotInt_ttbar_sb((LikelihoodInterval*)plinterval_ttbar_sb);
//       plotInt_ttbar_sb.Draw() ;
//       plcplot_ttbar_sb->SaveAs("plscan_ttbar_sb.pdf") ;
//       plcplot_ttbar_sb->SaveAs("plscan_ttbar_sb.png") ;

//       varsAtFitVals = false ;

//       return true ;

//   }

////==================================================================================================


//   bool ra2bRoostatsClass3::profileQcdSb( float& qcdSbLow, float& qcdSbHigh ) {

//       if ( ! initialized ) {
//          printf("\n\n *** Call initialize first.\n\n") ;
//          return false ;
//       }

//       if ( useSigTtwjVar ) {
//          printf("\n\n\n *** Can't do it.  Need to use ra2bRoostatsClass3 with constructor arg useSigTtwjVar set to false.\n\n") ;
//          return false ;
//       }

//    //--- Profile likelihood for SB qcd yield.

//       ProfileLikelihoodCalculator plc_qcd_sb( *dsObserved, *likelihood, RooArgSet( *rv_mu_qcd_sb ) ) ;
//       plc_qcd_sb.SetTestSize(0.32) ;
//       ConfInterval* plinterval_qcd_sb = plc_qcd_sb.GetInterval() ;
//       qcdSbLow  = ((LikelihoodInterval*) plinterval_qcd_sb)->LowerLimit(*rrv_mu_qcd_sb) ;
//       qcdSbHigh = ((LikelihoodInterval*) plinterval_qcd_sb)->UpperLimit(*rrv_mu_qcd_sb) ;
//       printf("\n\n") ;
//       printf("    qcd, SB 68%% CL interval  [%5.1f, %5.1f]\n\n", qcdSbLow, qcdSbHigh ) ;
//       TCanvas* plcplot_qcd_sb = new TCanvas("plcplot_qcd_sb", "qcd sb, Profile likelihood", 500, 400 ) ;
//       LikelihoodIntervalPlot plotInt_qcd_sb((LikelihoodInterval*)plinterval_qcd_sb);
//       plotInt_qcd_sb.Draw() ;
//       plcplot_qcd_sb->SaveAs("plscan_qcd_sb.pdf") ;
//       plcplot_qcd_sb->SaveAs("plscan_qcd_sb.png") ;

//       varsAtFitVals = false ;

//       return true ;

//   }

////==================================================================================================



///    float ttbar_sb_val(0.) ;
///    float qcd_sb_val(0.) ;
///    float ttbar_sb_err(0.) ;
///    float qcd_sb_err(0.) ;

///    float ttbar_sig_val(0.) ;
///    float qcd_sig_val(0.) ;
///    float ttbar_sig_err(0.) ;
///    float qcd_sig_err(0.) ;

///    if ( useSigTtwjVar ) {
///       ttbar_sb_val = ((RooFormulaVar*)rv_mu_ttbar_sb) ->getVal() ;
///       qcd_sb_val = ((RooFormulaVar*)rv_mu_qcd_sb) ->getVal() ;
///       ttbar_sig_val = ((RooRealVar*)rv_mu_ttbar_sig) ->getVal() ;
///       qcd_sig_val = ((RooRealVar*)rv_mu_qcd_sig) ->getVal() ;
///       ttbar_sig_err = ((RooRealVar*)rv_mu_ttbar_sig) ->getError() ;
///       qcd_sig_err = ((RooRealVar*)rv_mu_qcd_sig) ->getError() ;
///    } else {
///       ttbar_sig_val = ((RooFormulaVar*)rv_mu_ttbar_sig) ->getVal() ;
///       qcd_sig_val = ((RooFormulaVar*)rv_mu_qcd_sig) ->getVal() ;
///       ttbar_sb_val = ((RooRealVar*)rv_mu_ttbar_sb) ->getVal() ;
///       qcd_sb_val = ((RooRealVar*)rv_mu_qcd_sb) ->getVal() ;
///       ttbar_sb_err = ((RooRealVar*)rv_mu_ttbar_sb) ->getError() ;
///       qcd_sb_err = ((RooRealVar*)rv_mu_qcd_sb) ->getError() ;
///    }


///    if ( useSigTtwjVar ) {

///    //--- Profile likelihood for signal ttbar yield.

///        ProfileLikelihoodCalculator plc_ttbar_sig( *dsObserved, *likelihood, RooArgSet( *rv_mu_ttbar_sig ) ) ;
///        plc_ttbar_sig.SetTestSize(0.32) ;
///        ConfInterval* plinterval_ttbar_sig = plc_ttbar_sig.GetInterval() ;
///        float ttbar_sig_p1sig = ((LikelihoodInterval*) plinterval_ttbar_sig)->UpperLimit(*((RooRealVar*)rv_mu_ttbar_sig)) ;
///        float ttbar_sig_m1sig = ((LikelihoodInterval*) plinterval_ttbar_sig)->LowerLimit(*((RooRealVar*)rv_mu_ttbar_sig)) ;
///        printf("\n\n") ;
///        printf("    ttbar, SIG 68%% CL interval  [%5.1f, %5.1f]\n\n", ttbar_sig_m1sig, ttbar_sig_p1sig) ;
///        TCanvas* plcplot_ttbar_sig = new TCanvas("plcplot_ttbar_sig", "ttbar sig, Profile likelihood", 500, 400 ) ;
///        LikelihoodIntervalPlot plotInt_ttbar_sig((LikelihoodInterval*)plinterval_ttbar_sig);
///        plotInt_ttbar_sig.Draw() ;
///        plcplot_ttbar_sig->SaveAs("plscan_ttbar_sig.pdf") ;
///        plcplot_ttbar_sig->SaveAs("plscan_ttbar_sig.png") ;

///    //--- Profile likelihood for signal qcd yield.

///        ProfileLikelihoodCalculator plc_qcd_sig( *dsObserved, *likelihood, RooArgSet( *rv_mu_qcd_sig ) ) ;
///        plc_qcd_sig.SetTestSize(0.32) ;
///        ConfInterval* plinterval_qcd_sig = plc_qcd_sig.GetInterval() ;
///        float qcd_sig_p1sig = ((LikelihoodInterval*) plinterval_qcd_sig)->UpperLimit(*((RooRealVar*)rv_mu_qcd_sig)) ;
///        float qcd_sig_m1sig = ((LikelihoodInterval*) plinterval_qcd_sig)->LowerLimit(*((RooRealVar*)rv_mu_qcd_sig)) ;
///        printf("\n\n") ;
///        printf("    qcd, SIG 68%% CL interval  [%5.1f, %5.1f]\n\n", qcd_sig_m1sig, qcd_sig_p1sig) ;
///        TCanvas* plcplot_qcd_sig = new TCanvas("plcplot_qcd_sig", "qcd sig, Profile likelihood", 500, 400 ) ;
///        LikelihoodIntervalPlot plotInt_qcd_sig((LikelihoodInterval*)plinterval_qcd_sig);
///        plotInt_qcd_sig.Draw() ;
///        plcplot_qcd_sig->SaveAs("plscan_qcd_sig.pdf") ;
///        plcplot_qcd_sig->SaveAs("plscan_qcd_sig.png") ;

///    } else {

///    //--- Profile likelihood for sideband ttbar yield.

///        ProfileLikelihoodCalculator plc_ttbar_sb( *dsObserved, *likelihood, RooArgSet( *rv_mu_ttbar_sb ) ) ;
///        plc_ttbar_sb.SetTestSize(0.32) ;
///        ConfInterval* plinterval_ttbar_sb = plc_ttbar_sb.GetInterval() ;
///        float ttbar_sb_p1sb = ((LikelihoodInterval*) plinterval_ttbar_sb)->UpperLimit(*((RooRealVar*)rv_mu_ttbar_sb)) ;
///        float ttbar_sb_m1sb = ((LikelihoodInterval*) plinterval_ttbar_sb)->LowerLimit(*((RooRealVar*)rv_mu_ttbar_sb)) ;
///        printf("\n\n") ;
///        printf("    ttbar, SB 68%% CL interval  [%5.1f, %5.1f]\n\n", ttbar_sb_m1sb, ttbar_sb_p1sb) ;
///        TCanvas* plcplot_ttbar_sb = new TCanvas("plcplot_ttbar_sb", "ttbar sb, Profile likelihood", 500, 400 ) ;
///        LikelihoodIntervalPlot plotInt_ttbar_sb((LikelihoodInterval*)plinterval_ttbar_sb);
///        plotInt_ttbar_sb.Draw() ;
///        plcplot_ttbar_sb->SaveAs("plscan_ttbar_sb.pdf") ;
///        plcplot_ttbar_sb->SaveAs("plscan_ttbar_sb.png") ;

///    //--- Profile likelihood for sideband qcd yield.

///        ProfileLikelihoodCalculator plc_qcd_sb( *dsObserved, *likelihood, RooArgSet( *rv_mu_qcd_sb ) ) ;
///        plc_qcd_sb.SetTestSize(0.32) ;
///        ConfInterval* plinterval_qcd_sb = plc_qcd_sb.GetInterval() ;
///        float qcd_sb_p1sb = ((LikelihoodInterval*) plinterval_qcd_sb)->UpperLimit(*((RooRealVar*)rv_mu_qcd_sb)) ;
///        float qcd_sb_m1sb = ((LikelihoodInterval*) plinterval_qcd_sb)->LowerLimit(*((RooRealVar*)rv_mu_qcd_sb)) ;
///        printf("\n\n") ;
///        printf("    sb, SB 68%% CL interval  [%5.1f, %5.1f]\n\n", qcd_sb_m1sb, qcd_sb_p1sb) ;
///        TCanvas* plcplot_qcd_sb = new TCanvas("plcplot_qcd_sb", "qcd sb, Profile likelihood", 500, 400 ) ;
///        LikelihoodIntervalPlot plotInt_qcd_sb((LikelihoodInterval*)plinterval_qcd_sb);
///        plotInt_qcd_sb.Draw() ;
///        plcplot_qcd_sb->SaveAs("plscan_qcd_sb.pdf") ;
///        plcplot_qcd_sb->SaveAs("plscan_qcd_sb.png") ;

///    }



//  //====================================================================================================================

//     bool ra2bRoostatsClass3::sbPlotsUniformBins( const char* plotBaseName ) {

//        if ( ! initialized ) {
//           printf("\n\n *** Call initialize first.\n\n") ;
//           return false ;
//        }


//        if ( ! varsAtFitVals ) {
//           printf("\n\n *** Try this right after calling doFit.\n\n") ;
//           return false ;
//        }

//     //--  Drawn with same-width bins.

//        TH1F* hm3j_sb_data  = new TH1F("hm3j_sb_data" ,"3-jet mass, SB data" , 5, 0.5, 5.5 ) ;
//        TH1F* hm3j_sb_ttbar = new TH1F("hm3j_sb_ttbar","3-jet mass, SB ttbar", 5, 0.5, 5.5 ) ;
//        TH1F* hm3j_sb_qcd   = new TH1F("hm3j_sb_qcd"  ,"3-jet mass, SB qcd"  , 5, 0.5, 5.5 ) ;
//        TH1F* hm3j_sb_ew    = new TH1F("hm3j_sb_ew"   ,"3-jet mass, SB ew"   , 5, 0.5, 5.5 ) ;

//        hm3j_sb_data->SetLineWidth(2) ;
//        hm3j_sb_data->SetMarkerStyle(20) ;
//        hm3j_sb_ew->SetFillColor(49) ;
//        hm3j_sb_qcd->SetFillColor(46) ;
//        hm3j_sb_ttbar->SetFillColor(42) ;

//        hm3j_sb_data->SetBinContent( 1, rv_Nsb1->getVal() ) ;
//        hm3j_sb_data->SetBinContent( 2, rv_Nsb2->getVal() ) ;
//        hm3j_sb_data->SetBinContent( 3, rv_Nsb3->getVal() ) ;
//        hm3j_sb_data->SetBinContent( 4, rv_Nsb4->getVal() ) ;
//        hm3j_sb_data->SetBinContent( 5, rv_Nsb5->getVal() ) ;

//        hm3j_sb_ttbar->SetBinContent( 1, rv_mu_ttbar_sb1->getVal() ) ;
//        hm3j_sb_ttbar->SetBinContent( 2, rv_mu_ttbar_sb2->getVal() ) ;
//        hm3j_sb_ttbar->SetBinContent( 3, rv_mu_ttbar_sb3->getVal() ) ;
//        hm3j_sb_ttbar->SetBinContent( 4, rv_mu_ttbar_sb4->getVal() ) ;
//        hm3j_sb_ttbar->SetBinContent( 5, rv_mu_ttbar_sb5->getVal() ) ;

//        hm3j_sb_qcd->SetBinContent( 1, rv_mu_qcd_sb1->getVal() ) ;
//        hm3j_sb_qcd->SetBinContent( 2, rv_mu_qcd_sb2->getVal() ) ;
//        hm3j_sb_qcd->SetBinContent( 3, rv_mu_qcd_sb3->getVal() ) ;
//        hm3j_sb_qcd->SetBinContent( 4, rv_mu_qcd_sb4->getVal() ) ;
//        hm3j_sb_qcd->SetBinContent( 5, rv_mu_qcd_sb5->getVal() ) ;

//        hm3j_sb_ew->SetBinContent( 1, rv_mu_ew_sb1->getVal() ) ;
//        hm3j_sb_ew->SetBinContent( 2, rv_mu_ew_sb2->getVal() ) ;
//        hm3j_sb_ew->SetBinContent( 3, rv_mu_ew_sb3->getVal() ) ;
//        hm3j_sb_ew->SetBinContent( 4, rv_mu_ew_sb4->getVal() ) ;
//        hm3j_sb_ew->SetBinContent( 5, rv_mu_ew_sb5->getVal() ) ;

//        THStack* hstack_m3j_sb_fit = new THStack( "hstack_m3j_sb_fit", "SB fit, 3-jet mass" ) ;
//        hstack_m3j_sb_fit->Add( hm3j_sb_ew ) ;
//        hstack_m3j_sb_fit->Add( hm3j_sb_qcd ) ;
//        hstack_m3j_sb_fit->Add( hm3j_sb_ttbar ) ;


//        TCanvas* can_sbfit = new TCanvas("can_sbfit", "SB 3-jet mass fit", 700, 500 ) ;

//        hm3j_sb_data->SetMaximum( 1.4*(hm3j_sb_data->GetMaximum()) ) ;
//        hm3j_sb_data->SetLabelSize(0.06,"x") ;
//        hm3j_sb_data->GetXaxis()->SetBinLabel(1, "Bin 1") ;
//        hm3j_sb_data->GetXaxis()->SetBinLabel(2, "Bin 2") ;
//        hm3j_sb_data->GetXaxis()->SetBinLabel(3, "Bin 3") ;
//        hm3j_sb_data->GetXaxis()->SetBinLabel(4, "Bin 4") ;
//        hm3j_sb_data->GetXaxis()->SetBinLabel(5, "Bin 5") ;


//        hm3j_sb_data->Draw("histpe") ;
//        hstack_m3j_sb_fit->Draw( "same" ) ;
//        hm3j_sb_data->Draw("samehistpe") ;

//        TLegend* m3j_legend = new TLegend(0.62,0.7,0.97,0.95) ;
//        m3j_legend->SetFillColor( kWhite ) ;
//        m3j_legend->AddEntry( hm3j_sb_data, "Data" ) ;
//        m3j_legend->AddEntry( hm3j_sb_ttbar, "ttbar" ) ;
//        m3j_legend->AddEntry( hm3j_sb_qcd, "QCD" ) ;
//        m3j_legend->AddEntry( hm3j_sb_ew, "EW" ) ;
//        m3j_legend->Draw() ;

//        TText* fittext = new TText() ;
//        fittext->SetTextSize(0.04) ;
//        char fitlabel[1000] ;

//        float ew_sb(0.) ;
//        ew_sb += rv_mu_ew_sb1->getVal() ;
//        ew_sb += rv_mu_ew_sb2->getVal() ;
//        ew_sb += rv_mu_ew_sb3->getVal() ;
//        ew_sb += rv_mu_ew_sb4->getVal() ;
//        ew_sb += rv_mu_ew_sb5->getVal() ;
//        int Nsb = rv_Nsb1->getVal() +rv_Nsb2->getVal() +rv_Nsb3->getVal() +rv_Nsb4->getVal() +rv_Nsb5->getVal()  ;

//        float ttbar_sb_val(0.) ;
//        float qcd_sb_val(0.) ;
//        float ttbar_sb_err(0.) ;
//        float qcd_sb_err(0.) ;

//        float ttbar_sig_val(0.) ;
//        float qcd_sig_val(0.) ;
//        float ttbar_sig_err(0.) ;
//        float qcd_sig_err(0.) ;

//        if ( useSigTtwjVar ) {
//           ttbar_sb_val = ((RooFormulaVar*)rv_mu_ttbar_sb) ->getVal() ;
//           qcd_sb_val = ((RooFormulaVar*)rv_mu_qcd_sb) ->getVal() ;
//           ttbar_sig_val = ((RooRealVar*)rv_mu_ttbar_sig) ->getVal() ;
//           qcd_sig_val = ((RooRealVar*)rv_mu_qcd_sig) ->getVal() ;
//           ttbar_sig_err = ((RooRealVar*)rv_mu_ttbar_sig) ->getError() ;
//           qcd_sig_err = ((RooRealVar*)rv_mu_qcd_sig) ->getError() ;
//        } else {
//           ttbar_sig_val = ((RooFormulaVar*)rv_mu_ttbar_sig) ->getVal() ;
//           qcd_sig_val = ((RooFormulaVar*)rv_mu_qcd_sig) ->getVal() ;
//           ttbar_sb_val = ((RooRealVar*)rv_mu_ttbar_sb) ->getVal() ;
//           qcd_sb_val = ((RooRealVar*)rv_mu_qcd_sb) ->getVal() ;
//           ttbar_sb_err = ((RooRealVar*)rv_mu_ttbar_sb) ->getError() ;
//           qcd_sb_err = ((RooRealVar*)rv_mu_qcd_sb) ->getError() ;
//        }

//        float ltop = 0.90 ;
//        float lx = 0.78 ;
//        float dy = 0.06 ;
//        if ( useSigTtwjVar ) {
//           sprintf( fitlabel, "%5d", Nsb ) ;
//           fittext->DrawTextNDC( lx, ltop, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f", ttbar_sb_val ) ;
//           fittext->DrawTextNDC( lx, ltop-dy, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f", qcd_sb_val ) ;
//           fittext->DrawTextNDC( lx, ltop-2*dy, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f", ew_sb ) ;
//           fittext->DrawTextNDC( lx, ltop-3*dy, fitlabel ) ;
//        } else {
//           sprintf( fitlabel, "%5d", Nsb ) ;
//           fittext->DrawTextNDC( lx, ltop, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f +/- %4.0f", ttbar_sb_val, ttbar_sb_err ) ;
//           fittext->DrawTextNDC( lx, ltop-dy, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f +/- %4.0f", qcd_sb_val, qcd_sb_err ) ;
//           fittext->DrawTextNDC( lx, ltop-2*dy, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f", ew_sb ) ;
//           fittext->DrawTextNDC( lx, ltop-3*dy, fitlabel ) ;
//        }

//        char outfile[1000] ;
//        sprintf( outfile, "%s-sb.png", plotBaseName ) ;
//        can_sbfit->SaveAs( outfile ) ;

//        return true ;

//     } // sbPlotsUniformBins

////==================================================================================================================

//     bool ra2bRoostatsClass3::sbPlotsVariableBins( const char* plotBaseName ) {

//        if ( ! initialized ) {
//           printf("\n\n *** Call initialize first.\n\n") ;
//           return false ;
//        }

//        if ( ! varsAtFitVals ) {
//           printf("\n\n *** Try this right after calling doFit.\n\n") ;
//           return false ;
//        }

//     //--  Drawn with variable-width bins.

//        int nxbins = 5 ;
//        float xbinedges[6] = { 0., 160., 180., 260., 400., 800. } ;
//        float xbinwid[5] ;
//        for ( int bi=0; bi<nxbins; bi++ ) { xbinwid[bi] = xbinedges[bi+1] - xbinedges[bi] ; }

//        TH1F* hvbm3j_sb_data  = new TH1F("hvbm3j_sb_data" ,"3-jet mass, SB data" , nxbins, xbinedges ) ;
//        TH1F* hvbm3j_sb_ttbar = new TH1F("hvbm3j_sb_ttbar","3-jet mass, SB ttbar", nxbins, xbinedges ) ;
//        TH1F* hvbm3j_sb_qcd   = new TH1F("hvbm3j_sb_qcd"  ,"3-jet mass, SB qcd"  , nxbins, xbinedges ) ;
//        TH1F* hvbm3j_sb_ew    = new TH1F("hvbm3j_sb_ew"   ,"3-jet mass, SB ew"   , nxbins, xbinedges ) ;

//        hvbm3j_sb_data->SetLineWidth(2) ;
//        hvbm3j_sb_data->SetMarkerStyle(20) ;
//        hvbm3j_sb_ew->SetFillColor(49) ;
//        hvbm3j_sb_qcd->SetFillColor(46) ;
//        hvbm3j_sb_ttbar->SetFillColor(42) ;

//        hvbm3j_sb_data->SetBinContent( 1, (rv_Nsb1->getVal()) * (xbinwid[1]/xbinwid[0]) ) ;
//        hvbm3j_sb_data->SetBinContent( 2, (rv_Nsb2->getVal()) * (xbinwid[1]/xbinwid[1]) ) ;
//        hvbm3j_sb_data->SetBinContent( 3, (rv_Nsb3->getVal()) * (xbinwid[1]/xbinwid[2]) ) ;
//        hvbm3j_sb_data->SetBinContent( 4, (rv_Nsb4->getVal()) * (xbinwid[1]/xbinwid[3]) ) ;
//        hvbm3j_sb_data->SetBinContent( 5, (rv_Nsb5->getVal()) * (xbinwid[1]/xbinwid[4]) ) ;

//        hvbm3j_sb_data->SetBinError( 1, sqrt(rv_Nsb1->getVal()) * (xbinwid[1]/xbinwid[0]) ) ;
//        hvbm3j_sb_data->SetBinError( 2, sqrt(rv_Nsb2->getVal()) * (xbinwid[1]/xbinwid[1]) ) ;
//        hvbm3j_sb_data->SetBinError( 3, sqrt(rv_Nsb3->getVal()) * (xbinwid[1]/xbinwid[2]) ) ;
//        hvbm3j_sb_data->SetBinError( 4, sqrt(rv_Nsb4->getVal()) * (xbinwid[1]/xbinwid[3]) ) ;
//        hvbm3j_sb_data->SetBinError( 5, sqrt(rv_Nsb5->getVal()) * (xbinwid[1]/xbinwid[4]) ) ;

//        hvbm3j_sb_ttbar->SetBinContent( 1, (rv_mu_ttbar_sb1->getVal()) * (xbinwid[1]/xbinwid[0]) ) ;
//        hvbm3j_sb_ttbar->SetBinContent( 2, (rv_mu_ttbar_sb2->getVal()) * (xbinwid[1]/xbinwid[1]) ) ;
//        hvbm3j_sb_ttbar->SetBinContent( 3, (rv_mu_ttbar_sb3->getVal()) * (xbinwid[1]/xbinwid[2]) ) ;
//        hvbm3j_sb_ttbar->SetBinContent( 4, (rv_mu_ttbar_sb4->getVal()) * (xbinwid[1]/xbinwid[3]) ) ;
//        hvbm3j_sb_ttbar->SetBinContent( 5, (rv_mu_ttbar_sb5->getVal()) * (xbinwid[1]/xbinwid[4]) ) ;

//        hvbm3j_sb_qcd->SetBinContent( 1, (rv_mu_qcd_sb1->getVal()) * (xbinwid[1]/xbinwid[0]) ) ;
//        hvbm3j_sb_qcd->SetBinContent( 2, (rv_mu_qcd_sb2->getVal()) * (xbinwid[1]/xbinwid[1]) ) ;
//        hvbm3j_sb_qcd->SetBinContent( 3, (rv_mu_qcd_sb3->getVal()) * (xbinwid[1]/xbinwid[2]) ) ;
//        hvbm3j_sb_qcd->SetBinContent( 4, (rv_mu_qcd_sb4->getVal()) * (xbinwid[1]/xbinwid[3]) ) ;
//        hvbm3j_sb_qcd->SetBinContent( 5, (rv_mu_qcd_sb5->getVal()) * (xbinwid[1]/xbinwid[4]) ) ;

//        hvbm3j_sb_ew->SetBinContent( 1, (rv_mu_ew_sb1->getVal()) * (xbinwid[1]/xbinwid[0]) ) ;
//        hvbm3j_sb_ew->SetBinContent( 2, (rv_mu_ew_sb2->getVal()) * (xbinwid[1]/xbinwid[1]) ) ;
//        hvbm3j_sb_ew->SetBinContent( 3, (rv_mu_ew_sb3->getVal()) * (xbinwid[1]/xbinwid[2]) ) ;
//        hvbm3j_sb_ew->SetBinContent( 4, (rv_mu_ew_sb4->getVal()) * (xbinwid[1]/xbinwid[3]) ) ;
//        hvbm3j_sb_ew->SetBinContent( 5, (rv_mu_ew_sb5->getVal()) * (xbinwid[1]/xbinwid[4]) ) ;

//        THStack* hvbstack_m3j_sb_fit = new THStack( "hvbstack_m3j_sb_fit", "SB fit, 3-jet mass" ) ;
//        hvbstack_m3j_sb_fit->Add( hvbm3j_sb_ew ) ;
//        hvbstack_m3j_sb_fit->Add( hvbm3j_sb_qcd ) ;
//        hvbstack_m3j_sb_fit->Add( hvbm3j_sb_ttbar ) ;

//        TCanvas* can_vbsbfit = new TCanvas("can_vbsbfit", "SB 3-jet mass fit", 700, 500 ) ;

//        //hvbm3j_sb_data->SetMaximum( 1.4*(hvbm3j_sb_data->GetMaximum()) ) ;


//        hvbm3j_sb_data->Draw("histpe") ;
//        hvbstack_m3j_sb_fit->Draw( "same" ) ;
//        hvbm3j_sb_data->Draw("samehistpe") ;

//        TLegend* vbm3j_legend = new TLegend(0.62,0.7,0.97,0.95) ;
//        vbm3j_legend->SetFillColor( kWhite ) ;
//        vbm3j_legend->AddEntry( hvbm3j_sb_data, "Data" ) ;
//        vbm3j_legend->AddEntry( hvbm3j_sb_ttbar, "ttbar" ) ;
//        vbm3j_legend->AddEntry( hvbm3j_sb_qcd, "QCD" ) ;
//        vbm3j_legend->AddEntry( hvbm3j_sb_ew, "EW" ) ;
//        vbm3j_legend->Draw() ;

//        TText* fittext = new TText() ;
//        fittext->SetTextSize(0.04) ;
//        char fitlabel[1000] ;

//        float ew_sb(0.) ;
//        ew_sb += rv_mu_ew_sb1->getVal() ;
//        ew_sb += rv_mu_ew_sb2->getVal() ;
//        ew_sb += rv_mu_ew_sb3->getVal() ;
//        ew_sb += rv_mu_ew_sb4->getVal() ;
//        ew_sb += rv_mu_ew_sb5->getVal() ;
//        int Nsb = rv_Nsb1->getVal() +rv_Nsb2->getVal() +rv_Nsb3->getVal() +rv_Nsb4->getVal() +rv_Nsb5->getVal()  ;
//        float ltop = 0.90 ;
//        float lx = 0.78 ;
//        float dy = 0.06 ;

//        float ttbar_sb_val(0.) ;
//        float qcd_sb_val(0.) ;
//        float ttbar_sb_err(0.) ;
//        float qcd_sb_err(0.) ;

//        float ttbar_sig_val(0.) ;
//        float qcd_sig_val(0.) ;
//        float ttbar_sig_err(0.) ;
//        float qcd_sig_err(0.) ;

//        if ( useSigTtwjVar ) {
//           ttbar_sb_val = ((RooFormulaVar*)rv_mu_ttbar_sb) ->getVal() ;
//           qcd_sb_val = ((RooFormulaVar*)rv_mu_qcd_sb) ->getVal() ;
//           ttbar_sig_val = ((RooRealVar*)rv_mu_ttbar_sig) ->getVal() ;
//           qcd_sig_val = ((RooRealVar*)rv_mu_qcd_sig) ->getVal() ;
//           ttbar_sig_err = ((RooRealVar*)rv_mu_ttbar_sig) ->getError() ;
//           qcd_sig_err = ((RooRealVar*)rv_mu_qcd_sig) ->getError() ;
//        } else {
//           ttbar_sig_val = ((RooFormulaVar*)rv_mu_ttbar_sig) ->getVal() ;
//           qcd_sig_val = ((RooFormulaVar*)rv_mu_qcd_sig) ->getVal() ;
//           ttbar_sb_val = ((RooRealVar*)rv_mu_ttbar_sb) ->getVal() ;
//           qcd_sb_val = ((RooRealVar*)rv_mu_qcd_sb) ->getVal() ;
//           ttbar_sb_err = ((RooRealVar*)rv_mu_ttbar_sb) ->getError() ;
//           qcd_sb_err = ((RooRealVar*)rv_mu_qcd_sb) ->getError() ;
//        }

//        if ( useSigTtwjVar ) {
//           sprintf( fitlabel, "%5d", Nsb ) ;
//           fittext->DrawTextNDC( lx, ltop, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f", ttbar_sb_val ) ;
//           fittext->DrawTextNDC( lx, ltop-dy, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f", qcd_sb_val ) ;
//           fittext->DrawTextNDC( lx, ltop-2*dy, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f", ew_sb ) ;
//           fittext->DrawTextNDC( lx, ltop-3*dy, fitlabel ) ;
//        } else {
//           sprintf( fitlabel, "%5d", Nsb ) ;
//           fittext->DrawTextNDC( lx, ltop, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f +/- %4.0f", ttbar_sb_val, ttbar_sb_err ) ;
//           fittext->DrawTextNDC( lx, ltop-dy, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f +/- %4.0f", qcd_sb_val, qcd_sb_err ) ;
//           fittext->DrawTextNDC( lx, ltop-2*dy, fitlabel ) ;
//           sprintf( fitlabel, "%4.0f", ew_sb ) ;
//           fittext->DrawTextNDC( lx, ltop-3*dy, fitlabel ) ;
//        }

//        char outfile[1000] ;
//        sprintf( outfile, "%s-sb-vb.png", plotBaseName ) ;
//        can_vbsbfit->SaveAs( outfile ) ;

//        return true ;

//     } // sbPlotsVariableBins

  //===================================================================

   ra2bRoostatsClass3::~ra2bRoostatsClass3() {

      if ( !initialized ) return ;

//    delete dsObserved ;
//    //delete plinterval_susy_sig ;

//    delete rv_Nsig ;
//    delete rv_Na ;
//    delete rv_Nd ;
//    delete rv_Nsb1 ;
//    delete rv_Nsb2 ;
//    delete rv_Nsb3 ;
//    delete rv_Nsb4 ;
//    delete rv_Nsb5 ;
//    delete rv_Nlsb1 ;
//    delete rv_Nlsb2 ;
//    delete rv_Nlsb3 ;
//    delete rv_Nlsb4 ;
//    delete rv_Nlsb5 ;
//    delete rv_Nslsig1 ;
//    delete rv_Nslsig2 ;
//    delete rv_Nslsig3 ;
//    delete rv_Nslsig4 ;
//    delete rv_Nslsig5 ;
//    delete rv_Nslsb1 ;
//    delete rv_Nslsb2 ;
//    delete rv_Nslsb3 ;
//    delete rv_Nslsb4 ;
//    delete rv_Nslsb5 ;
//    delete rv_Nslmsb1 ;
//    delete rv_Nslmsb2 ;
//    delete rv_Nslmsb3 ;
//    delete rv_Nslmsb4 ;
//    delete rv_Nslmsb5 ;
//    delete rv_mu_ttbar_sig ;
//    delete rv_mu_qcd_sig ;
//    delete rv_mu_ttbar_sb ;
//    delete rv_mu_qcd_sb ;
//    delete rv_mu_ew_sig    ;
//    delete rv_mu_susy_sig  ;
//    delete rv_mu_ew_sb1 ;
//    delete rv_mu_ew_sb2 ;
//    delete rv_mu_ew_sb3 ;
//    delete rv_mu_ew_sb4 ;
//    delete rv_mu_ew_sb5 ;

//    delete rv_mu_susy_sb1 ;
//    delete rv_mu_susy_sb2 ;
//    delete rv_mu_susy_sb3 ;
//    delete rv_mu_susy_sb4 ;
//    delete rv_mu_susy_sb5 ;
//    delete rv_mu_ttbar_a ;
//    delete rv_mu_qcd_a   ;
//    delete rv_mu_ew_a    ;
//    delete rv_mu_susy_a  ;
//    delete rv_mu_ttbar_d ;
//    delete rv_mu_qcd_d   ;
//    delete rv_mu_ew_d    ;
//    delete rv_mu_susy_d  ;
//    delete rv_mu_qcd_lsb1 ;
//    delete rv_mu_qcd_lsb2 ;
//    delete rv_mu_qcd_lsb3 ;
//    delete rv_mu_qcd_lsb4 ;
//    delete rv_mu_qcd_lsb5 ;
//    delete rv_mu_qcdmc_sig ;
//    delete rv_mu_qcdmc_sb  ;
//    delete rv_mu_qcdmc_a   ;
//    delete rv_mu_qcdmc_d   ;

//    delete rv_mu_sl_ttbar_sig1 ;
//    delete rv_mu_sl_ttbar_sig2 ;
//    delete rv_mu_sl_ttbar_sig3 ;
//    delete rv_mu_sl_ttbar_sig4 ;
//    delete rv_mu_sl_ttbar_sig5 ;
//    delete rv_mu_sl_ttbar_sb1 ;
//    delete rv_mu_sl_ttbar_sb2 ;
//    delete rv_mu_sl_ttbar_sb3 ;
//    delete rv_mu_sl_ttbar_sb4 ;
//    delete rv_mu_sl_ttbar_sb5 ;
//    delete rv_mu_sl_ttbar_msb1 ;
//    delete rv_mu_sl_ttbar_msb2 ;
//    delete rv_mu_sl_ttbar_msb3 ;
//    delete rv_mu_sl_ttbar_msb4 ;
//    delete rv_mu_sl_ttbar_msb5 ;
//    delete rv_mu_sl_ew_sig1 ;
//    delete rv_mu_sl_ew_sig2 ;
//    delete rv_mu_sl_ew_sig3 ;
//    delete rv_mu_sl_ew_sig4 ;
//    delete rv_mu_sl_ew_sig5 ;
//    delete rv_mu_sl_ew_sb1 ;
//    delete rv_mu_sl_ew_sb2 ;
//    delete rv_mu_sl_ew_sb3 ;
//    delete rv_mu_sl_ew_sb4 ;
//    delete rv_mu_sl_ew_sb5 ;
//    delete rv_mu_sl_ew_msb1 ;
//    delete rv_mu_sl_ew_msb2 ;
//    delete rv_mu_sl_ew_msb3 ;
//    delete rv_mu_sl_ew_msb4 ;
//    delete rv_mu_sl_ew_msb5 ;

//    delete rv_mu_sl_susy_sig1 ;
//    delete rv_mu_sl_susy_sig2 ;
//    delete rv_mu_sl_susy_sig3;
//    delete rv_mu_sl_susy_sig4 ;
//    delete rv_mu_sl_susy_sig5 ;
//    delete rv_mu_sl_susy_sb1 ;
//    delete rv_mu_sl_susy_sb2 ;
//    delete rv_mu_sl_susy_sb3 ;
//    delete rv_mu_sl_susy_sb4 ;
//    delete rv_mu_sl_susy_sb5 ;
//    delete rv_mu_sl_susy_msb1 ;
//    delete rv_mu_sl_susy_msb2 ;
//    delete rv_mu_sl_susy_msb3 ;
//    delete rv_mu_sl_susy_msb4 ;
//    delete rv_mu_sl_susy_msb5 ;

//    delete rv_mu_sl_ttbar_sb ;
//    delete rv_mu_sl_ttbar_sig ;
//    delete rv_mu_qcd_lsb ;

//    delete rv_f_qcd_lsb1 ;
//    delete rv_f_qcd_lsb2 ;
//    delete rv_f_qcd_lsb3 ;
//    delete rv_f_qcd_lsb4 ;
//    delete rv_f_qcd_lsb5 ;
//    delete rv_mu_qcd_sb1 ;
//    delete rv_mu_qcd_sb2 ;
//    delete rv_mu_qcd_sb3 ;
//    delete rv_mu_qcd_sb4 ;
//    delete rv_mu_qcd_sb5 ;
//    delete rv_mu_sl_ttbar1 ;
//    delete rv_mu_sl_ttbar2 ;
//    delete rv_mu_sl_ttbar3 ;
//    delete rv_mu_sl_ttbar4 ;
//    delete rv_mu_sl_ttbar5 ;
//    delete rv_mu_sl_ttbar ;
//    delete rv_f_sl_ttbar1 ;
//    delete rv_f_sl_ttbar2 ;
//    delete rv_f_sl_ttbar3 ;
//    delete rv_f_sl_ttbar4 ;
//    delete rv_f_sl_ttbar5 ;
//    delete rv_mu_ttbar_sb1 ;
//    delete rv_mu_ttbar_sb2 ;
//    delete rv_mu_ttbar_sb3 ;
//    delete rv_mu_ttbar_sb4 ;
//    delete rv_mu_ttbar_sb5 ;
//    delete rv_n_sig ;
//    delete rv_n_sb1 ;
//    delete rv_n_sb2 ;
//    delete rv_n_sb3 ;
//    delete rv_n_sb4 ;
//    delete rv_n_sb5 ;
//    delete rv_n_a ;
//    delete rv_n_d ;
//    delete rv_n_lsb1 ;
//    delete rv_n_lsb2 ;
//    delete rv_n_lsb3 ;
//    delete rv_n_lsb4 ;
//    delete rv_n_lsb5 ;
//    delete rv_n_sl_sig1 ;
//    delete rv_n_sl_sig2 ;
//    delete rv_n_sl_sig3 ;
//    delete rv_n_sl_sig4 ;
//    delete rv_n_sl_sig5 ;
//    delete rv_n_sl_sb1 ;
//    delete rv_n_sl_sb2 ;
//    delete rv_n_sl_sb3 ;
//    delete rv_n_sl_sb4 ;
//    delete rv_n_sl_sb5 ;
//    delete rv_n_sl_msb1 ;
//    delete rv_n_sl_msb2 ;
//    delete rv_n_sl_msb3 ;
//    delete rv_n_sl_msb4 ;
//    delete rv_n_sl_msb5 ;

//    delete pdf_Nsig ;
//    delete pdf_Na ;
//    delete pdf_Nd ;
//    delete pdf_Nsb1 ;
//    delete pdf_Nsb2 ;
//    delete pdf_Nsb3 ;
//    delete pdf_Nsb4 ;
//    delete pdf_Nsb5 ;
//    delete pdf_Nlsb1 ;
//    delete pdf_Nlsb2 ;
//    delete pdf_Nlsb3 ;
//    delete pdf_Nlsb4 ;
//    delete pdf_Nlsb5 ;
//    delete pdf_Nsl_sig1 ;
//    delete pdf_Nsl_sig2 ;
//    delete pdf_Nsl_sig3 ;
//    delete pdf_Nsl_sig4 ;
//    delete pdf_Nsl_sig5 ;
//    delete pdf_Nsl_sb1 ;
//    delete pdf_Nsl_sb2 ;
//    delete pdf_Nsl_sb3 ;
//    delete pdf_Nsl_sb4 ;
//    delete pdf_Nsl_sb5 ;
//    delete pdf_Nsl_msb1 ;
//    delete pdf_Nsl_msb2 ;
//    delete pdf_Nsl_msb3 ;
//    delete pdf_Nsl_msb4 ;
//    delete pdf_Nsl_msb5 ;
//    delete pdf_Nqcdmc_sig ;
//    delete pdf_Nqcdmc_sb  ;
//    delete pdf_Nqcdmc_a   ;
//    delete pdf_Nqcdmc_d   ;

//    delete likelihood ;

//    delete workspace ;



   }



  //===================================================================

    bool ra2bRoostatsClass3::initialize( const char* infile ) {


       printf( "\n\n Opening input file : %s\n\n", infile ) ;

       FILE* infp ;
       if ( (infp=fopen( infile,"r"))==NULL ) {
          printf("\n\n *** Problem opening input file: %s.\n\n", infile ) ;
          return false ;
       }

       sprintf( initializeFile, "%s", infile ) ;


       int    Nsig                  ;
       int    Nsb                   ;
       int    Nsig_sl               ;
       int    Nsb_sl                ;
       int    Nsig_ldp              ;
       int    Nsb_ldp               ;
       int    Nlsb                  ;
       int    Nlsb_ldp              ;
       int    Nlsb_0b               ;
       int    Nlsb_0b_ldp           ;
       float  Nqcdmc_sig            ;
       float  Nqcdmc_sig_err        ;
       float  Nqcdmc_sb             ;
       float  Nqcdmc_sb_err         ;
       float  Nqcdmc_sig_sl         ;
       float  Nqcdmc_sig_sl_err     ;
       float  Nqcdmc_sb_sl          ;
       float  Nqcdmc_sb_sl_err      ;
       float  Nqcdmc_sig_ldp        ;
       float  Nqcdmc_sig_ldp_err    ;
       float  Nqcdmc_sb_ldp         ;
       float  Nqcdmc_sb_ldp_err     ;
       float  Nqcdmc_lsb            ;
       float  Nqcdmc_lsb_err        ;
       float  Nqcdmc_lsb_ldp        ;
       float  Nqcdmc_lsb_ldp_err    ;
       float  Nqcdmc_lsb_0b         ;
       float  Nqcdmc_lsb_0b_err     ;
       float  Nqcdmc_lsb_0b_ldp     ;
       float  Nqcdmc_lsb_0b_ldp_err ;
       float  Nttbarmc_sig          ;
       float  Nttbarmc_sb           ;
       float  Nttbarmc_sig_sl       ;
       float  Nttbarmc_sb_sl        ;
       float  Nttbarmc_sig_ldp      ;
       float  Nttbarmc_sb_ldp       ;
       float  Nttbarmc_lsb          ;
       float  Nttbarmc_lsb_ldp      ;
       int    NWJmc_sig             ;
       int    NWJmc_sb              ;
       int    NWJmc_sig_sl          ;
       int    NWJmc_sb_sl           ;
       int    NWJmc_sig_ldp         ;
       int    NWJmc_sb_ldp          ;
       int    NWJmc_lsb             ;
       int    NWJmc_lsb_ldp         ;
       int    NZnnmc_sig            ;
       int    NZnnmc_sb             ;
       int    NZnnmc_sig_sl         ;
       int    NZnnmc_sb_sl          ;
       int    NZnnmc_sig_ldp        ;
       int    NZnnmc_sb_ldp         ;
       int    NZnnmc_lsb            ;
       int    NZnnmc_lsb_ldp        ;
       int    NEwomc_sig            ;
       int    NEwomc_sb             ;
       int    NEwomc_sig_sl         ;
       int    NEwomc_sb_sl          ;
       int    NEwomc_sig_ldp        ;
       int    NEwomc_sb_ldp         ;
       int    NEwomc_lsb            ;
       int    NEwomc_lsb_ldp        ;
       float  Nsusymc_sig           ;
       float  Nsusymc_sb            ;
       float  Nsusymc_sig_sl        ;
       float  Nsusymc_sb_sl         ;
       float  Nsusymc_sig_ldp       ;
       float  Nsusymc_sb_ldp        ;
       float  Nsusymc_lsb           ;
       float  Nsusymc_lsb_ldp       ;
       float  Nsusymc_lsb_0b        ;
       float  Nsusymc_lsb_0b_ldp    ;
       int    Nhtonlytrig_lsb_0b      ;
       int    Nhtonlytrig_lsb_0b_ldp  ;
       int    Nsb_ee                  ;
       int    Nsig_ee                 ;
       int    Nsb_mm                  ;
       int    Nsig_mm                 ;



       //--- read in description line.
       printf("\n\n") ;
       char c(0) ;
       while ( c!=10  ) { c = fgetc( infp ) ; printf("%c", c ) ; }
       printf("\n\n") ;


       char label[1000] ;

      //--- Inputs generated with gen_roostats_input4.c
      //    The order here must be consistent with the order there!

       fscanf( infp, "%s %g", label, &EffScaleFactor        ) ;   printf( "%s %g\n", label, EffScaleFactor        ) ;
       fscanf( infp, "%s %g", label, &EffScaleFactorErr     ) ;   printf( "%s %g\n", label, EffScaleFactorErr     ) ;
       fscanf( infp, "%s %d", label, &Nsig                  ) ;   printf( "%s %d\n", label, Nsig                  ) ;
       fscanf( infp, "%s %d", label, &Nsb                   ) ;   printf( "%s %d\n", label, Nsb                   ) ;
       fscanf( infp, "%s %d", label, &Nsig_sl               ) ;   printf( "%s %d\n", label, Nsig_sl               ) ;
       fscanf( infp, "%s %d", label, &Nsb_sl                ) ;   printf( "%s %d\n", label, Nsb_sl                ) ;
       fscanf( infp, "%s %d", label, &Nsig_ldp              ) ;   printf( "%s %d\n", label, Nsig_ldp              ) ;
       fscanf( infp, "%s %d", label, &Nsb_ldp               ) ;   printf( "%s %d\n", label, Nsb_ldp               ) ;
       fscanf( infp, "%s %d", label, &Nlsb                  ) ;   printf( "%s %d\n", label, Nlsb                  ) ;
       fscanf( infp, "%s %d", label, &Nlsb_ldp              ) ;   printf( "%s %d\n", label, Nlsb_ldp              ) ;
       fscanf( infp, "%s %d", label, &Nlsb_0b               ) ;   printf( "%s %d\n", label, Nlsb_0b               ) ;
       fscanf( infp, "%s %d", label, &Nlsb_0b_ldp           ) ;   printf( "%s %d\n", label, Nlsb_0b_ldp           ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig            ) ;   printf( "%s %g\n", label, Nqcdmc_sig            ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_err        ) ;   printf( "%s %g\n", label, Nqcdmc_sig_err        ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb             ) ;   printf( "%s %g\n", label, Nqcdmc_sb             ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_err         ) ;   printf( "%s %g\n", label, Nqcdmc_sb_err         ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_sl         ) ;   printf( "%s %g\n", label, Nqcdmc_sig_sl         ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_sl_err     ) ;   printf( "%s %g\n", label, Nqcdmc_sig_sl_err     ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_sl          ) ;   printf( "%s %g\n", label, Nqcdmc_sb_sl          ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_sl_err      ) ;   printf( "%s %g\n", label, Nqcdmc_sb_sl_err      ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_ldp        ) ;   printf( "%s %g\n", label, Nqcdmc_sig_ldp        ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_ldp_err    ) ;   printf( "%s %g\n", label, Nqcdmc_sig_ldp_err    ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_ldp         ) ;   printf( "%s %g\n", label, Nqcdmc_sb_ldp         ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_ldp_err     ) ;   printf( "%s %g\n", label, Nqcdmc_sb_ldp_err     ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb            ) ;   printf( "%s %g\n", label, Nqcdmc_lsb            ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_err        ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_err        ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_ldp        ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_ldp        ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_ldp_err    ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_ldp_err    ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_0b         ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_0b         ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_0b_err     ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_0b_err     ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_0b_ldp     ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_0b_ldp     ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_0b_ldp_err ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_0b_ldp_err ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sig          ) ;   printf( "%s %g\n", label, Nttbarmc_sig          ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sb           ) ;   printf( "%s %g\n", label, Nttbarmc_sb           ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sig_sl       ) ;   printf( "%s %g\n", label, Nttbarmc_sig_sl       ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sb_sl        ) ;   printf( "%s %g\n", label, Nttbarmc_sb_sl        ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sig_ldp      ) ;   printf( "%s %g\n", label, Nttbarmc_sig_ldp      ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sb_ldp       ) ;   printf( "%s %g\n", label, Nttbarmc_sb_ldp       ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_lsb          ) ;   printf( "%s %g\n", label, Nttbarmc_lsb          ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_lsb_ldp      ) ;   printf( "%s %g\n", label, Nttbarmc_lsb_ldp      ) ;
       fscanf( infp, "%s %g", label, &lsf_WJmc              ) ;   printf( "%s %g\n", label, lsf_WJmc              ) ;
       fscanf( infp, "%s %g", label, &lsf_WJmc_err          ) ;   printf( "%s %g\n", label, lsf_WJmc_err          ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sig             ) ;   printf( "%s %d\n", label, NWJmc_sig             ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sb              ) ;   printf( "%s %d\n", label, NWJmc_sb              ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sig_sl          ) ;   printf( "%s %d\n", label, NWJmc_sig_sl          ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sb_sl           ) ;   printf( "%s %d\n", label, NWJmc_sb_sl           ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sig_ldp         ) ;   printf( "%s %d\n", label, NWJmc_sig_ldp         ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sb_ldp          ) ;   printf( "%s %d\n", label, NWJmc_sb_ldp          ) ;
       fscanf( infp, "%s %d", label, &NWJmc_lsb             ) ;   printf( "%s %d\n", label, NWJmc_lsb             ) ;
       fscanf( infp, "%s %d", label, &NWJmc_lsb_ldp         ) ;   printf( "%s %d\n", label, NWJmc_lsb_ldp         ) ;
       fscanf( infp, "%s %g", label, &lsf_Znnmc             ) ;   printf( "%s %g\n", label, lsf_Znnmc             ) ;
       fscanf( infp, "%s %g", label, &lsf_Znnmc_err         ) ;   printf( "%s %g\n", label, lsf_Znnmc_err         ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sig            ) ;   printf( "%s %d\n", label, NZnnmc_sig            ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sb             ) ;   printf( "%s %d\n", label, NZnnmc_sb             ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sig_sl         ) ;   printf( "%s %d\n", label, NZnnmc_sig_sl         ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sb_sl          ) ;   printf( "%s %d\n", label, NZnnmc_sb_sl          ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sig_ldp        ) ;   printf( "%s %d\n", label, NZnnmc_sig_ldp        ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sb_ldp         ) ;   printf( "%s %d\n", label, NZnnmc_sb_ldp         ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_lsb            ) ;   printf( "%s %d\n", label, NZnnmc_lsb            ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_lsb_ldp        ) ;   printf( "%s %d\n", label, NZnnmc_lsb_ldp        ) ;
       fscanf( infp, "%s %g", label, &lsf_Ewomc             ) ;   printf( "%s %g\n", label, lsf_Ewomc             ) ;
       fscanf( infp, "%s %g", label, &lsf_Ewomc_err         ) ;   printf( "%s %g\n", label, lsf_Ewomc_err         ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sig            ) ;   printf( "%s %d\n", label, NEwomc_sig            ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sb             ) ;   printf( "%s %d\n", label, NEwomc_sb             ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sig_sl         ) ;   printf( "%s %d\n", label, NEwomc_sig_sl         ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sb_sl          ) ;   printf( "%s %d\n", label, NEwomc_sb_sl          ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sig_ldp        ) ;   printf( "%s %d\n", label, NEwomc_sig_ldp        ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sb_ldp         ) ;   printf( "%s %d\n", label, NEwomc_sb_ldp         ) ;
       fscanf( infp, "%s %d", label, &NEwomc_lsb            ) ;   printf( "%s %d\n", label, NEwomc_lsb            ) ;
       fscanf( infp, "%s %d", label, &NEwomc_lsb_ldp        ) ;   printf( "%s %d\n", label, NEwomc_lsb_ldp        ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sig           ) ;   printf( "%s %g\n", label, Nsusymc_sig           ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sb            ) ;   printf( "%s %g\n", label, Nsusymc_sb            ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sig_sl        ) ;   printf( "%s %g\n", label, Nsusymc_sig_sl        ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sb_sl         ) ;   printf( "%s %g\n", label, Nsusymc_sb_sl         ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sig_ldp       ) ;   printf( "%s %g\n", label, Nsusymc_sig_ldp       ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sb_ldp        ) ;   printf( "%s %g\n", label, Nsusymc_sb_ldp        ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_lsb           ) ;   printf( "%s %g\n", label, Nsusymc_lsb           ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_lsb_ldp       ) ;   printf( "%s %g\n", label, Nsusymc_lsb_ldp       ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_lsb_0b        ) ;   printf( "%s %g\n", label, Nsusymc_lsb_0b        ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_lsb_0b_ldp    ) ;   printf( "%s %g\n", label, Nsusymc_lsb_0b_ldp    ) ;
       fscanf( infp, "%s %d", label, &Nhtonlytrig_lsb_0b        ) ;   printf( "%s %d\n", label, Nhtonlytrig_lsb_0b        ) ;
       fscanf( infp, "%s %d", label, &Nhtonlytrig_lsb_0b_ldp    ) ;   printf( "%s %d\n", label, Nhtonlytrig_lsb_0b_ldp    ) ;
       fscanf( infp, "%s %d", label, &Nsb_ee                    ) ;   printf( "%s %d\n", label, Nsb_ee                    ) ;
       fscanf( infp, "%s %d", label, &Nsig_ee                   ) ;   printf( "%s %d\n", label, Nsig_ee                   ) ;
       fscanf( infp, "%s %d", label, &Nsb_mm                    ) ;   printf( "%s %d\n", label, Nsb_mm                    ) ;
       fscanf( infp, "%s %d", label, &Nsig_mm                   ) ;   printf( "%s %d\n", label, Nsig_mm                   ) ;
       fscanf( infp, "%s %g", label, &acc_ee_mean               ) ;   printf( "%s %g\n", label, acc_ee_mean               ) ;
       fscanf( infp, "%s %g", label, &acc_ee_err                ) ;   printf( "%s %g\n", label, acc_ee_err                ) ;
       fscanf( infp, "%s %g", label, &acc_mm_mean               ) ;   printf( "%s %g\n", label, acc_mm_mean               ) ;
       fscanf( infp, "%s %g", label, &acc_mm_err                ) ;   printf( "%s %g\n", label, acc_mm_err                ) ;
       fscanf( infp, "%s %g", label, &eff_ee_mean               ) ;   printf( "%s %g\n", label, eff_ee_mean               ) ;
       fscanf( infp, "%s %g", label, &eff_ee_err                ) ;   printf( "%s %g\n", label, eff_ee_err                ) ;
       fscanf( infp, "%s %g", label, &eff_mm_mean               ) ;   printf( "%s %g\n", label, eff_mm_mean               ) ;
       fscanf( infp, "%s %g", label, &eff_mm_err                ) ;   printf( "%s %g\n", label, eff_mm_err                ) ;
       fscanf( infp, "%s %g", label, &Ztoll_lumi                ) ;   printf( "%s %g\n", label, Ztoll_lumi                ) ;
       fscanf( infp, "%s %g", label, &Ztoll_tight_sf            ) ;   printf( "%s %g\n", label, Ztoll_tight_sf            ) ;
       fscanf( infp, "%s %g", label, &Ztoll_tight_sf_err        ) ;   printf( "%s %g\n", label, Ztoll_tight_sf_err        ) ;
       fscanf( infp, "%s %g", label, &DataLumi                  ) ;   printf( "%s %g\n", label, DataLumi                  ) ;

       printf("\n Done reading in %s\n\n", infile ) ;
       fclose( infp ) ;


       //--- Print out a nice summary of the inputs.

       float Nsm_sig         = Nttbarmc_sig          +  lsf_WJmc*NWJmc_sig          +  Nqcdmc_sig          +  lsf_Znnmc*NZnnmc_sig          +  lsf_Ewomc*NEwomc_sig         ;
       float Nsm_sb          = Nttbarmc_sb           +  lsf_WJmc*NWJmc_sb           +  Nqcdmc_sb           +  lsf_Znnmc*NZnnmc_sb           +  lsf_Ewomc*NEwomc_sb          ;
       float Nsm_sig_sl      = Nttbarmc_sig_sl       +  lsf_WJmc*NWJmc_sig_sl       +  Nqcdmc_sig_sl       +  lsf_Znnmc*NZnnmc_sig_sl       +  lsf_Ewomc*NEwomc_sig_sl      ;
       float Nsm_sb_sl       = Nttbarmc_sb_sl        +  lsf_WJmc*NWJmc_sb_sl        +  Nqcdmc_sb_sl        +  lsf_Znnmc*NZnnmc_sb_sl        +  lsf_Ewomc*NEwomc_sb_sl       ;
       float Nsm_sig_ldp     = Nttbarmc_sig_ldp      +  lsf_WJmc*NWJmc_sig_ldp      +  Nqcdmc_sig_ldp      +  lsf_Znnmc*NZnnmc_sig_ldp      +  lsf_Ewomc*NEwomc_sig_ldp     ;
       float Nsm_sb_ldp      = Nttbarmc_sb_ldp       +  lsf_WJmc*NWJmc_sb_ldp       +  Nqcdmc_sb_ldp       +  lsf_Znnmc*NZnnmc_sb_ldp       +  lsf_Ewomc*NEwomc_sb_ldp      ;
       float Nsm_lsb         = Nttbarmc_lsb          +  lsf_WJmc*NWJmc_lsb          +  Nqcdmc_lsb          +  lsf_Znnmc*NZnnmc_lsb          +  lsf_Ewomc*NEwomc_lsb         ;
       float Nsm_lsb_ldp     = Nttbarmc_lsb_ldp      +  lsf_WJmc*NWJmc_lsb_ldp      +  Nqcdmc_lsb_ldp      +  lsf_Znnmc*NZnnmc_lsb_ldp      +  lsf_Ewomc*NEwomc_lsb_ldp     ;
       float Nsm_lsb_0b      =                                                         Nqcdmc_lsb_0b                                                                        ;
       float Nsm_lsb_0b_ldp  =                                                         Nqcdmc_lsb_0b_ldp                                                                    ;



       //--- QCD min Delta phi N  pass / fail ratios.

       float Rqcd_sig        =  0. ;
       if ( Nqcdmc_sig_ldp > 0. ) { Rqcd_sig        =  Nqcdmc_sig        / Nqcdmc_sig_ldp ;  }
       float Rqcd_sig_err    =  0. ;
       if ( Nqcdmc_sig > 0. && Nqcdmc_sig_ldp > 0. ) { Rqcd_sig_err  =  Rqcd_sig * sqrt( pow(Nqcdmc_sig_err/Nqcdmc_sig,2) + pow(Nqcdmc_sig_ldp_err/Nqcdmc_sig_ldp,2) ) ; }


       float Rqcd_sb        =  0. ;
       if ( Nqcdmc_sb_ldp > 0. ) { Rqcd_sb        =  Nqcdmc_sb        / Nqcdmc_sb_ldp ;  }
       float Rqcd_sb_err    =  0. ;
       if ( Nqcdmc_sb > 0. && Nqcdmc_sb_ldp > 0. ) { Rqcd_sb_err  =  Rqcd_sb * sqrt( pow(Nqcdmc_sb_err/Nqcdmc_sb,2) + pow(Nqcdmc_sb_ldp_err/Nqcdmc_sb_ldp,2) ) ; }


       float Rqcd_lsb        =  0. ;
       if ( Nqcdmc_lsb_ldp > 0. ) { Rqcd_lsb        =  Nqcdmc_lsb        / Nqcdmc_lsb_ldp ;  }
       float Rqcd_lsb_err    =  0. ;
       if ( Nqcdmc_lsb > 0. && Nqcdmc_lsb_ldp > 0. ) { Rqcd_lsb_err  =  Rqcd_lsb * sqrt( pow(Nqcdmc_lsb_err/Nqcdmc_lsb,2) + pow(Nqcdmc_lsb_ldp_err/Nqcdmc_lsb_ldp,2) ) ; }


       float Nlsb_corrected     = Nlsb     - Nttbarmc_lsb     - lsf_WJmc*NWJmc_lsb     - lsf_Znnmc*NZnnmc_lsb     - lsf_Ewomc*NEwomc_lsb ;
       float Nlsb_ldp_corrected = Nlsb_ldp - Nttbarmc_lsb_ldp - lsf_WJmc*NWJmc_lsb_ldp - lsf_Znnmc*NZnnmc_lsb_ldp - lsf_Ewomc*NEwomc_lsb_ldp ;
       float Rdata_lsb = 0. ;
       if ( Nlsb_ldp_corrected > 0 ) { Rdata_lsb = (Nlsb_corrected) / (Nlsb_ldp_corrected) ; }
       float Rdata_lsb_err = 0. ;
       if ( Nlsb_corrected > 0 && Nlsb_ldp_corrected > 0 ) { Rdata_lsb_err = Rdata_lsb * sqrt( 1.0/(Nlsb_corrected) + 1.0/(Nlsb_ldp_corrected) ) ; }


       float Rqcd_lsb_0b        =  0. ;
       if ( Nqcdmc_lsb_0b_ldp > 0. ) { Rqcd_lsb_0b        =  Nqcdmc_lsb_0b        / Nqcdmc_lsb_0b_ldp ;  }
       float Rqcd_lsb_0b_err    =  0. ;
       if ( Nqcdmc_lsb_0b > 0. && Nqcdmc_lsb_0b_ldp > 0. ) { Rqcd_lsb_0b_err  =  Rqcd_lsb_0b * sqrt( pow(Nqcdmc_lsb_0b_err/Nqcdmc_lsb_0b,2) + pow(Nqcdmc_lsb_0b_ldp_err/Nqcdmc_lsb_0b_ldp,2) ) ; }


       float Rdata_lsb_0b = 0. ;
       if ( Nlsb_0b_ldp > 0 ) { Rdata_lsb_0b = (1.0*Nlsb_0b) / (1.0*Nlsb_0b_ldp) ; }
       float Rdata_lsb_0b_err = 0. ;
       if ( Nlsb_0b > 0 && Nlsb_0b_ldp > 0 ) { Rdata_lsb_0b_err = Rdata_lsb_0b * sqrt( 1.0/(1.0*Nlsb_0b) + 1.0/(1.0*Nlsb_0b_ldp) ) ; }


       //--- ttbar + Wjets  MET  SIG / SB ratios

       float Rttwj = 0. ;
       if ( (Nttbarmc_sb+lsf_WJmc*NWJmc_sb) > 0. ) { Rttwj    =   (Nttbarmc_sig+lsf_WJmc*NWJmc_sig) / (Nttbarmc_sb+lsf_WJmc*NWJmc_sb) ; }
       float Rttwj_err2 = pow( lsf_WJmc/ (Nttbarmc_sb+lsf_WJmc*NWJmc_sb),2)*NWJmc_sig
                        + pow( (Nttbarmc_sig+lsf_WJmc*NWJmc_sig) * lsf_WJmc / pow((Nttbarmc_sb+lsf_WJmc*NWJmc_sb),2), 2)*NWJmc_sb ;
       float Rttwj_err = sqrt(Rttwj_err2) ;

       float Rttwj_sl = 0. ;
       if ( (Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl) > 0. ) { Rttwj_sl    =   (Nttbarmc_sig_sl+lsf_WJmc*NWJmc_sig_sl) / (Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl) ; }
       float Rttwj_sl_err2 = pow( lsf_WJmc / (Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl), 2) * NWJmc_sig_sl
                           + pow( lsf_WJmc * (Nttbarmc_sig_sl+lsf_WJmc*NWJmc_sig_sl) / pow((Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl),2), 2)*NWJmc_sb_sl ;
       float Rttwj_sl_err = sqrt(Rttwj_sl_err2) ;

       float Rdata_sl = 0. ;
       if ( Nsb_sl > 0 ) { Rdata_sl = (1.0*Nsig_sl)/(1.0*Nsb_sl) ; }
       float Rdata_sl_err = 0. ;
       if ( Nsig_sl>0 && Nsb_sl>0 ) { Rdata_sl_err = Rdata_sl * sqrt( 1.0/(1.0*Nsig_sl) + 1.0/(1.0*Nsb_sl) ) ; }


       //--- Simple MC closure tests.

       float comp_mc_qcd_sb = Nqcdmc_sb_ldp * ( Nqcdmc_lsb_0b / Nqcdmc_lsb_0b_ldp ) ;
       float comp_mc_qcd_sb_err = Nqcdmc_sb_ldp_err * ( Nqcdmc_lsb_0b / Nqcdmc_lsb_0b_ldp ) ;

       float comp_mc_qcd_sig = Nqcdmc_sig_ldp * ( Nqcdmc_lsb_0b / Nqcdmc_lsb_0b_ldp ) ;
       float comp_mc_qcd_sig_err = Nqcdmc_sig_ldp_err * ( Nqcdmc_lsb_0b / Nqcdmc_lsb_0b_ldp ) ;

       float comp_mc_ttwj_sig = (Nttbarmc_sb + lsf_WJmc*NWJmc_sb) * ( (Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl) / (Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl) ) ;

       //--- below ignores ttbar errors, uses sqrt(N) on raw WJ counts.
       float comp_mc_ttwj_sig_err2 = pow( lsf_WJmc*( (Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl) / (Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl) ),2)*NWJmc_sb
                                   + pow( (Nttbarmc_sb + lsf_WJmc*NWJmc_sb) *(lsf_WJmc/ (Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl) ), 2)*NWJmc_sig_sl
                                   + pow( (Nttbarmc_sb + lsf_WJmc*NWJmc_sb) *  (Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl) / pow((Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl) ,2)*lsf_WJmc, 2 )*NWJmc_sb_sl ;
       float comp_mc_ttwj_sig_err = sqrt(comp_mc_ttwj_sig_err2) ;

       //--- Simple data calculations.


       float Nsb_ldp_corrected  = Nsb_ldp  - (Nttbarmc_sb_ldp  + lsf_WJmc*NWJmc_sb_ldp  + lsf_Znnmc*NZnnmc_sb_ldp  + lsf_Ewomc*NEwomc_sb_ldp  ) ;
       float comp_data_qcd_sb = Nsb_ldp_corrected * ( (1.0*Nlsb_0b)/(1.0*Nlsb_0b_ldp) ) ;
       float comp_data_qcd_sb_err = sqrt( Nsb_ldp + Nttbarmc_sb_ldp  + lsf_WJmc*NWJmc_sb_ldp  + lsf_Znnmc*NZnnmc_sb_ldp  + lsf_Ewomc*NEwomc_sb_ldp ) * ( (1.0*Nlsb_0b)/(1.0*Nlsb_0b_ldp) ) ;

       float Nsig_ldp_corrected = Nsig_ldp - (Nttbarmc_sig_ldp + lsf_WJmc*NWJmc_sig_ldp + lsf_Znnmc*NZnnmc_sig_ldp + lsf_Ewomc*NEwomc_sig_ldp ) ;
       float comp_data_qcd_sig = Nsig_ldp_corrected * ( (1.0*Nlsb_0b)/(1.0*Nlsb_0b_ldp) ) ;
       float comp_data_qcd_sig_err = sqrt( Nsig_ldp + Nttbarmc_sig_ldp  + lsf_WJmc*NWJmc_sig_ldp  + lsf_Znnmc*NZnnmc_sig_ldp  + lsf_Ewomc*NEwomc_sig_ldp ) * ( (1.0*Nlsb_0b)/(1.0*Nlsb_0b_ldp) ) ;

       float comp_data_ttwj_sig = (Nsb - (comp_data_qcd_sb + lsf_Znnmc*NZnnmc_sb + lsf_Ewomc*NEwomc_sb)) * ( (1.0*Nsig_sl) / (1.0*Nsb_sl) ) ;
       float comp_data_ttwj_sig_err2 = pow( ( (1.0*Nsig_sl) / (1.0*Nsb_sl) ), 2)*Nsb
                                     + pow( ( (1.0*Nsig_sl) / (1.0*Nsb_sl) ), 2)*pow(comp_data_qcd_sb_err,2)
                                     + pow( (Nsb - (comp_data_qcd_sb + lsf_Znnmc*NZnnmc_sb + lsf_Ewomc*NEwomc_sb)) / (1.0*Nsb_sl), 2 )*Nsig_sl
                                     + pow( (Nsb - (comp_data_qcd_sb + lsf_Znnmc*NZnnmc_sb + lsf_Ewomc*NEwomc_sb)) * (1.0*Nsig_sl) / pow(1.0*Nsb_sl,2), 2 ) * Nsb_sl ;
       float comp_data_ttwj_sig_err = sqrt(comp_data_ttwj_sig_err2) ;


       printf("\n\n\n") ;

       printf("------------+----------+----------+-----------------------+----------+----------+------------+------------+---------------\n") ;
       printf("   Sample   |  ttbar   |  W+jets  |          QCD          |  Z to nn |   other  |   All SM   |    Data    |     SUSY      \n") ;
       printf("------------+----------+----------+-----------------------+----------+----------+------------+------------+---------------\n") ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sig", Nttbarmc_sig, (lsf_WJmc*NWJmc_sig), Nqcdmc_sig, Nqcdmc_sig_err, lsf_Znnmc*NZnnmc_sig, lsf_Ewomc*NEwomc_sig, Nsm_sig, Nsig, Nsusymc_sig ) ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sb", Nttbarmc_sb, (lsf_WJmc*NWJmc_sb), Nqcdmc_sb, Nqcdmc_sb_err, lsf_Znnmc*NZnnmc_sb, lsf_Ewomc*NEwomc_sb, Nsm_sb, Nsb, Nsusymc_sb ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sig_sl", Nttbarmc_sig_sl, (lsf_WJmc*NWJmc_sig_sl), Nqcdmc_sig_sl, Nqcdmc_sig_sl_err, lsf_Znnmc*NZnnmc_sig_sl, lsf_Ewomc*NEwomc_sig_sl, Nsm_sig_sl, Nsig_sl, Nsusymc_sig_sl ) ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sb_sl", Nttbarmc_sb_sl, (lsf_WJmc*NWJmc_sb_sl), Nqcdmc_sb_sl, Nqcdmc_sb_sl_err, lsf_Znnmc*NZnnmc_sb_sl, lsf_Ewomc*NEwomc_sb_sl, Nsm_sb_sl, Nsb_sl, Nsusymc_sb_sl ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sig_ldp", Nttbarmc_sig_ldp, (lsf_WJmc*NWJmc_sig_ldp), Nqcdmc_sig_ldp, Nqcdmc_sig_ldp_err, lsf_Znnmc*NZnnmc_sig_ldp, lsf_Ewomc*NEwomc_sig_ldp, Nsm_sig_ldp, Nsig_ldp, Nsusymc_sig_ldp ) ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sb_ldp", Nttbarmc_sb_ldp, (lsf_WJmc*NWJmc_sb_ldp), Nqcdmc_sb_ldp, Nqcdmc_sb_ldp_err, lsf_Znnmc*NZnnmc_sb_ldp, lsf_Ewomc*NEwomc_sb_ldp, Nsm_sb_ldp, Nsb_ldp, Nsusymc_sb_ldp ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "lsb", Nttbarmc_lsb, (lsf_WJmc*NWJmc_lsb), Nqcdmc_lsb, Nqcdmc_lsb_err, lsf_Znnmc*NZnnmc_lsb, lsf_Ewomc*NEwomc_lsb, Nsm_lsb, Nlsb, Nsusymc_lsb ) ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "lsb_ldp", Nttbarmc_lsb_ldp, (lsf_WJmc*NWJmc_lsb_ldp), Nqcdmc_lsb_ldp, Nqcdmc_lsb_ldp_err, lsf_Znnmc*NZnnmc_lsb_ldp, lsf_Ewomc*NEwomc_lsb_ldp, Nsm_lsb_ldp, Nlsb_ldp, Nsusymc_lsb_ldp ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | -------- | -------- | %10.1f +/- %6.1f | -------- | -------- | %10.1f | %10d | %8.1f\n",
            "lsb_0b",  Nqcdmc_lsb_0b, Nqcdmc_lsb_0b_err, Nsm_lsb_0b, Nlsb_0b, Nsusymc_lsb_0b ) ;
       printf(" %10s | -------- | -------- | %10.1f +/- %6.1f | -------- | -------- | %10.1f | %10d | %8.1f\n",
            "lsb_0b_ldp",  Nqcdmc_lsb_0b_ldp, Nqcdmc_lsb_0b_ldp_err, Nsm_lsb_0b_ldp, Nlsb_0b_ldp, Nsusymc_lsb_0b_ldp ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf("------------+----------+----------+-----------------------+----------+----------+------------+------------+---------------\n") ;


       printf("\n\n\n") ;

       printf(" R QCD  :  sig    /    sig_ldp  : ( %10.1f +/- %6.1f ) / ( %10.1f +/- %6.1f )    =    %5.3f +/- %5.3f\n",
            Nqcdmc_sig, Nqcdmc_sig_err, Nqcdmc_sig_ldp, Nqcdmc_sig_ldp_err, Rqcd_sig, Rqcd_sig_err ) ;

       printf(" R QCD  :  sb     /     sb_ldp  : ( %10.1f +/- %6.1f ) / ( %10.1f +/- %6.1f )    =    %5.3f +/- %5.3f\n",
            Nqcdmc_sb, Nqcdmc_sb_err, Nqcdmc_sb_ldp, Nqcdmc_sb_ldp_err, Rqcd_sb, Rqcd_sb_err ) ;

       printf(" R QCD  :  lsb    /    lsb_ldp  : ( %10.1f +/- %6.1f ) / ( %10.1f +/- %6.1f )    =    %5.3f +/- %5.3f\n",
            Nqcdmc_lsb, Nqcdmc_lsb_err, Nqcdmc_lsb_ldp, Nqcdmc_lsb_ldp_err, Rqcd_lsb, Rqcd_lsb_err ) ;

       printf(" R data :  lsb    /    lsb_ldp  : ( %10.1f            ) / ( %10.1f            )    =    %5.3f +/- %5.3f\n",
            Nlsb_corrected, Nlsb_ldp_corrected, Rdata_lsb, Rdata_lsb_err ) ;

       printf(" R QCD  :  lsb_0b / lsb_0b_ldp  : ( %10.1f +/- %6.1f ) / ( %10.1f +/- %6.1f )    =    %5.3f +/- %5.3f\n",
            Nqcdmc_lsb_0b, Nqcdmc_lsb_0b_err, Nqcdmc_lsb_0b_ldp, Nqcdmc_lsb_0b_ldp_err, Rqcd_lsb_0b, Rqcd_lsb_0b_err ) ;

       printf(" R data :  lsb_0b / lsb_0b_ldp  : ( %8d              ) / ( %8d              )    =    %5.3f +/- %5.3f\n",
            Nlsb_0b, Nlsb_0b_ldp, Rdata_lsb_0b, Rdata_lsb_0b_err ) ;


       printf("\n\n\n") ;


       printf(" R ttwj :  sig    /    sb    :  ( %5.1f +/- %4.1f ) / ( %5.1f +/- %4.1f )   =   %5.3f +/- %5.3f\n",
                 (Nttbarmc_sig+lsf_WJmc*NWJmc_sig), lsf_WJmc*sqrt(NWJmc_sig),
                 (Nttbarmc_sb+lsf_WJmc*NWJmc_sb), lsf_WJmc*sqrt(NWJmc_sb),
                 Rttwj, Rttwj_err ) ;

       printf(" R ttwj :  sig_sl /    sb_sl :  ( %5.1f +/- %4.1f ) / ( %5.1f +/- %4.1f )   =   %5.3f +/- %5.3f\n",
                 (Nttbarmc_sig_sl+lsf_WJmc*NWJmc_sig_sl), lsf_WJmc*sqrt(NWJmc_sig_sl),
                 (Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl), lsf_WJmc*sqrt(NWJmc_sb_sl),
                 Rttwj_sl, Rttwj_sl_err ) ;

       printf(" R data :  sig_sl /    sb_sl :    %3d              /   %3d                =   %5.3f +/- %5.3f\n",
                 Nsig_sl, Nsb_sl, Rdata_sl, Rdata_sl_err ) ;



       printf("\n\n\n") ;


       printf(" ----- Simple MC closure tests\n\n") ;

       printf("  QCD :  sb =  sb_ldp * (lsb_0b/lsb_0b_ldp) : ( %6.1f +/- %4.1f ) * ( %8.1f / %10.1f ) = %8.1f +/- %6.1f\n",
             Nqcdmc_sb_ldp, Nqcdmc_sb_ldp_err, Nqcdmc_lsb_0b, Nqcdmc_lsb_0b_ldp, comp_mc_qcd_sb, comp_mc_qcd_sb_err ) ;
       printf("                                                                               MC truth value : %8.1f +/- %6.1f\n",
                   Nqcdmc_sb, Nqcdmc_sb_err ) ;

       printf("\n") ;
       printf("  QCD : sig = sig_ldp * (lsb_0b/lsb_0b_ldp) : ( %6.1f +/- %4.1f ) * ( %8.1f / %10.1f ) = %8.1f +/- %6.1f\n",
             Nqcdmc_sig_ldp, Nqcdmc_sig_ldp_err, Nqcdmc_lsb_0b, Nqcdmc_lsb_0b_ldp, comp_mc_qcd_sig, comp_mc_qcd_sig_err ) ;
       printf("                                                                               MC truth value : %8.1f +/- %6.1f\n",
                   Nqcdmc_sig, Nqcdmc_sig_err ) ;


       printf("\n") ;
       printf("  ttwj : sig = ttwj_sb * ( ttwj_sig_sl / ttwj_sb_sl ) :  %6.1f * ( %6.1f / %6.1f ) = %6.1f +/- %4.1f\n",
              (Nttbarmc_sb + lsf_WJmc*NWJmc_sb), (Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl), (Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl),
              comp_mc_ttwj_sig, comp_mc_ttwj_sig_err ) ;
       printf("                                                                       MC truth value : %6.1f +/- %4.1f\n",
                (Nttbarmc_sig + lsf_WJmc*NWJmc_sig), lsf_WJmc*sqrt(NWJmc_sig) ) ;


       printf("\n\n\n") ;

       printf(" ----- Simple Data calculations\n\n") ;


       printf(" QCD :  sb  = (  sb_ldp - (ttwj+znn+other)_ldp_mc ) * (lsb_0b/lsb_0b_ldp) : ( %4d - %5.1f ) * ( %5d / %7d ) = %6.1f +/- %4.1f\n",
           Nsb_ldp, (Nttbarmc_sb_ldp  + lsf_WJmc*NWJmc_sb_ldp  + lsf_Znnmc*NZnnmc_sb_ldp  + lsf_Ewomc*NEwomc_sb_ldp  ),
           Nlsb_0b, Nlsb_0b_ldp, comp_data_qcd_sb, comp_data_qcd_sb_err ) ;
       printf("                                                                                                  MC truth value : %8.1f +/- %4.1f\n",
                   Nqcdmc_sb, Nqcdmc_sb_err ) ;

       printf("\n") ;
       printf(" QCD :  sig = ( sig_ldp - (ttwj+znn+other)_ldp_mc ) * (lsb_0b/lsb_0b_ldp) : ( %4d - %5.1f ) * ( %5d / %7d ) = %6.1f +/- %4.1f\n",
           Nsig_ldp, (Nttbarmc_sig_ldp  + lsf_WJmc*NWJmc_sig_ldp  + lsf_Znnmc*NZnnmc_sig_ldp  + lsf_Ewomc*NEwomc_sig_ldp  ),
           Nlsb_0b, Nlsb_0b_ldp, comp_data_qcd_sig, comp_data_qcd_sig_err ) ;
       printf("                                                                                                  MC truth value : %8.1f +/- %4.1f\n",
                   Nqcdmc_sig, Nqcdmc_sig_err ) ;


       printf("\n") ;
       printf(" ttwj : sig = ( sb - (qcd_sb + (znn+other)_sb_mc) ) * (sig_sl/sb_sl) : ( %5d - ( %5.1f + %5.1f))*(%3d/%3d) = %5.1f +/- %4.1f\n",
              Nsb, comp_data_qcd_sb, (lsf_Znnmc*NZnnmc_sb + lsf_Ewomc*NEwomc_sb), Nsig_sl, Nsb_sl, comp_data_ttwj_sig, comp_data_ttwj_sig_err ) ;
       printf("                                                                                              MC truth value : %5.1f +/- %4.1f\n",
                 (Nttbarmc_sig + lsf_WJmc*NWJmc_sig), lsf_WJmc*sqrt(NWJmc_sig) ) ;


       printf("\n\n\n") ;





     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


       printf(" --- Defining observables.\n" ) ;


      rv_Nsig        = new RooRealVar( "Nsig"        , "Nsig"        , 0.0, 400. ) ;
      rv_Nsb         = new RooRealVar( "Nsb"         , "Nsb"         , 0.0, 1000000. ) ;

      rv_Nsig_sl     = new RooRealVar( "Nsig_sl"     , "Nsig_sl"     , 0.0, 1000000. ) ;
      rv_Nsb_sl      = new RooRealVar( "Nsb_sl"      , "Nsb_sl"      , 0.0, 1000000. ) ;

      rv_Nsig_ldp    = new RooRealVar( "Nsig_ldp"    , "Nsig_ldp"    , 0.0, 1000000. ) ;
      rv_Nsb_ldp     = new RooRealVar( "Nsb_ldp"     , "Nsb_ldp"     , 0.0, 1000000. ) ;

      rv_Nlsb_0b     = new RooRealVar( "Nlsb_0b"     , "Nlsb_0b"     , 0.0, 1000000. ) ;
      rv_Nlsb_0b_ldp = new RooRealVar( "Nlsb_0b_ldp" , "Nlsb_0b_ldp" , 0.0, 1000000. ) ;

      rv_Nsb_ee      = new RooRealVar( "Nsb_ee"      ,"Nsb_ee"       , 0., 100. ) ;
      rv_Nsig_ee     = new RooRealVar( "Nsig_ee"     ,"Nsig_ee"      , 0., 100. ) ;

      rv_Nsb_mm      = new RooRealVar( "Nsb_mm"      ,"Nsb_mm"       , 0., 100. ) ;
      rv_Nsig_mm     = new RooRealVar( "Nsig_mm"     ,"Nsig_mm"      , 0., 100. ) ;





      if ( Nsig < 0 ) {
         printf("\n\n *** Negative value for Nsig in input file.  Will set Nsig to MC expectation, which is %d.\n\n",
             TMath::Nint( Nsm_sig ) ) ;
         Nsig = TMath::Nint( Nsm_sig ) ;
      }

      rv_Nsig        -> setVal( Nsig ) ;
      rv_Nsb         -> setVal( Nsb ) ;

      rv_Nsig_sl     -> setVal( Nsig_sl ) ;
      rv_Nsb_sl      -> setVal( Nsb_sl ) ;

      rv_Nsig_ldp    -> setVal( Nsig_ldp ) ;
      rv_Nsb_ldp     -> setVal( Nsb_ldp ) ;

      rv_Nlsb_0b     -> setVal( Nhtonlytrig_lsb_0b ) ;
      rv_Nlsb_0b_ldp -> setVal( Nhtonlytrig_lsb_0b_ldp ) ;

      rv_Nsb_ee      -> setVal( Nsb_ee ) ;
      rv_Nsig_ee     -> setVal( Nsig_ee ) ;

      rv_Nsb_mm      -> setVal( Nsb_mm ) ;
      rv_Nsig_mm     -> setVal( Nsig_mm ) ;








    //++++++++ Parameters of the likelihood +++++++++++++++++++++++++++++++++++++++++

      printf(" --- Defining parameters.\n" ) ;



    //____ Counts in SIG ______________________

      if ( useSigTtwjVar ) {
         rrv_mu_ttwj_sig = new RooRealVar( "mu_ttwj_sig"   , "mu_ttwj_sig"   , 0.0, 100. ) ;
         rv_mu_ttwj_sig = rrv_mu_ttwj_sig ;
         rrv_mu_ttwj_sig   -> setVal( Nttbarmc_sig + lsf_WJmc*NWJmc_sig ) ;  //-- this is a starting value only.
      }
      if ( !useLdpVars ) {
         rrv_mu_qcd_sig  = new RooRealVar( "mu_qcd_sig"    , "mu_qcd_sig"    , 0.0, 25. ) ;
         rv_mu_qcd_sig = rrv_mu_qcd_sig ;
         rrv_mu_qcd_sig  -> setVal( Nqcdmc_sig ) ; //-- this is a starting value only.
      }


      //-- Note: Ewo is rfv

      rv_mu_znn_sig      = new RooRealVar( "mu_znn_sig"    , "mu_znn_sig"    , 0.0, 80. ) ;

      float maxSusySig = 2.0*Nsig ;
      rv_mu_susy_sig     = new RooRealVar( "mu_susy_sig"   , "mu_susy_sig"   , 0.0, maxSusySig ) ;


      rv_mu_znn_sig   -> setVal( lsf_Znnmc*NZnnmc_sig ) ;  //-- this is a starting value only.
      rv_mu_susy_sig    -> setVal( 0. ) ;  //-- this is a starting value only.





    //____ Counts in SB  ______________________


      if ( !useSigTtwjVar ) {
         rrv_mu_ttwj_sb  = new RooRealVar( "mu_ttwj_sb"    , "mu_ttwj_sb"    , 0.0, 200. ) ;
         rv_mu_ttwj_sb = rrv_mu_ttwj_sb ;
         rrv_mu_ttwj_sb   -> setVal( Nttbarmc_sb + lsf_WJmc*NWJmc_sb ) ;  //-- this is a starting value only.
      }
      if ( !useLdpVars ) {
         rrv_mu_qcd_sb  = new RooRealVar( "mu_qcd_sb"    , "mu_qcd_sb"    , 0.0, 100. ) ;
         rv_mu_qcd_sb = rrv_mu_qcd_sb ;
         rrv_mu_qcd_sb  -> setVal( Nqcdmc_sb ) ; //-- this is a starting value only.
      }

      //-- Note: QCD is rfv
      //-- Note: Ewo is rfv
      //-- Note: SUSY is rfv

      rv_mu_znn_sb       = new RooRealVar( "mu_znn_sb"     , "mu_znn_sb"     , 0.0, 150. ) ;

      rv_mu_znn_sb   -> setVal( lsf_Znnmc*NZnnmc_sb ) ;  //-- this is a starting value only.






    //____ Counts in SIG, SL  ______________________

      rv_mu_ttwj_sig_sl  = new RooRealVar( "mu_ttwj_sig_sl"    , "mu_ttwj_sig_sl"    , 0.0, 500. ) ;

      //-- Note: QCD, Ewo, and Znn are assumed to be negligible and are not explicitly included.
      //-- Note: SUSY is rfv

      rv_mu_ttwj_sig_sl  -> setVal( Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl ) ;  //-- this is a starting value only.







    //____ Counts in SB, SL  ______________________

      rv_mu_ttwj_sb_sl  = new RooRealVar( "mu_ttwj_sb_sl"    , "mu_ttwj_sb_sl"    , 0.0, 500. ) ;

      //-- Note: QCD, Ewo, and Znn are assumed to be negligible and are not explicitly included.
      //-- Note: SUSY is rfv

      rv_mu_ttwj_sb_sl  -> setVal( Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl ) ;  //-- this is a starting value only.







    //____ Counts in SIG, LDP  ______________________

      if ( useLdpVars ) {
         rrv_mu_qcd_sig_ldp  = new RooRealVar( "mu_qcd_sig_ldp"    , "mu_qcd_sig_ldp"    , 0.0, 500. ) ;
         rv_mu_qcd_sig_ldp = rrv_mu_qcd_sig_ldp ;
         rrv_mu_qcd_sig_ldp  -> setVal( Nqcdmc_sig_ldp ) ; //-- this is a starting value only.
      }

      //-- Note: Ewo is assumed to be negligible and is not explicitly included.
      //-- Note: Znn is rfv (MC)
      //-- Note: ttwj is rfv (MC)
      //-- Note: SUSY is rfv








    //____ Counts in SB, LDP  ______________________

      if ( useLdpVars ) {
         rrv_mu_qcd_sb_ldp  = new RooRealVar( "mu_qcd_sb_ldp"    , "mu_qcd_sb_ldp"    , 0.0, 500. ) ;
         rv_mu_qcd_sb_ldp = rrv_mu_qcd_sb_ldp ;
         rrv_mu_qcd_sb_ldp  -> setVal( Nqcdmc_sb_ldp ) ; //-- this is a starting value only.
      }


      //-- Note: Ewo is assumed to be negligible and is not explicitly included.
      //-- Note: Znn is rfv (MC)
      //-- Note: ttwj is rfv (MC)
      //-- Note: SUSY is rfv








    //____ Counts in LSB, 0b  ______________________

      rv_mu_qcd_lsb_0b  = new RooRealVar( "mu_qcd_lsb_0b"    ,  "mu_qcd_lsb_0b" ,  0.    ,  10000. ) ;

      //-- Note: The 0btag LSB is assumed to be 100% QCD.

      rv_mu_qcd_lsb_0b  -> setVal( Nhtonlytrig_lsb_0b ) ;  //-- this is a starting value only.






    //____ Counts in LSB, 0b, LDP  ______________________

      rv_mu_qcd_lsb_0b_ldp  = new RooRealVar( "mu_qcd_lsb_0b_ldp"    ,  "mu_qcd_lsb_0b_ldp",   0. ,  10000. ) ;

      //-- Note: The 0btag LSB, LDP is assumed to be 100% QCD.

      rv_mu_qcd_lsb_0b_ldp  -> setVal( Nhtonlytrig_lsb_0b_ldp ) ;  //-- this is a starting value only.






    //____ Counts in SB, ee  ______________________


      //-- Note: The Z to ee sample is assumed to be 100% Z to ee.
      //-- Note: zee is rfv






    //____ Counts in SIG, ee  ______________________


      //-- Note: The Z to ee sample is assumed to be 100% Z to ee.
      //-- Note: zee is rfv








    //____ Counts in SB, mm  ______________________


      //-- Note: The Z to mm sample is assumed to be 100% Z to mm.
      //-- Note: zmm is rfv






    //____ Counts in SIG, mm  ______________________


      //-- Note: The Z to mm sample is assumed to be 100% Z to mm.
      //-- Note: zmm is rfv






    //____ MC inputs _______________________________

      printf(" --- Defining MC parameters.\n" ) ;

     //-- SUSY

      rv_mu_susymc_sig      = new RooRealVar( "mu_susymc_sig"     , "mu_susymc_sig"     , 0.0, 100000. ) ;
      rv_mu_susymc_sb       = new RooRealVar( "mu_susymc_sb"      , "mu_susymc_sb"      , 0.0, 100000. ) ;
      rv_mu_susymc_sig_sl   = new RooRealVar( "mu_susymc_sig_sl"  , "mu_susymc_sig_sl"  , 0.0, 100000. ) ;
      rv_mu_susymc_sb_sl    = new RooRealVar( "mu_susymc_sb_sl"   , "mu_susymc_sb_sl"   , 0.0, 100000. ) ;
      rv_mu_susymc_sig_ldp  = new RooRealVar( "mu_susymc_sig_ldp" , "mu_susymc_sig_ldp" , 0.0, 100000. ) ;
      rv_mu_susymc_sb_ldp   = new RooRealVar( "mu_susymc_sb_ldp"  , "mu_susymc_sb_ldp"  , 0.0, 100000. ) ;

      rv_mu_susymc_sig     -> setVal( 0.1 ) ;
      rv_mu_susymc_sb      -> setVal( 0. ) ;
      rv_mu_susymc_sig_sl  -> setVal( 0. ) ;
      rv_mu_susymc_sb_sl   -> setVal( 0. ) ;
      rv_mu_susymc_sig_ldp -> setVal( 0. ) ;
      rv_mu_susymc_sb_ldp  -> setVal( 0. ) ;

      rv_mu_susymc_sig     -> setConstant(kTRUE) ;
      rv_mu_susymc_sb      -> setConstant(kTRUE) ;
      rv_mu_susymc_sig_sl  -> setConstant(kTRUE) ;
      rv_mu_susymc_sb_sl   -> setConstant(kTRUE) ;
      rv_mu_susymc_sig_ldp -> setConstant(kTRUE) ;
      rv_mu_susymc_sb_ldp  -> setConstant(kTRUE) ;



     //-- SIG, LDP

      rv_mu_ttbarmc_sig_ldp   = new RooRealVar( "mu_ttbarmc_sig_ldp" ,"mu_ttbarmc_sig_ldp" , 0., 1000. ) ;
      rv_mu_WJmc_sig_ldp      = new RooRealVar( "mu_WJmc_sig_ldp"    ,"mu_WJmc_sig_ldp"    , 0., 1000. ) ;
      rv_mu_Znnmc_sig_ldp     = new RooRealVar( "mu_Znnmc_sig_ldp"   ,"mu_Znnmc_sig_ldp"   , 0., 1000. ) ;
      rv_mu_Ewomc_sig_ldp     = new RooRealVar( "mu_Ewomc_sig_ldp"   ,"mu_Ewomc_sig_ldp"   , 0., 1000. ) ;

      rv_mu_ttbarmc_sig_ldp  -> setVal( Nttbarmc_sig_ldp ) ;
      rv_mu_WJmc_sig_ldp     -> setVal( NWJmc_sig_ldp ) ;
      rv_mu_Znnmc_sig_ldp    -> setVal( NZnnmc_sig_ldp ) ;
      rv_mu_Ewomc_sig_ldp    -> setVal( NEwomc_sig_ldp ) ;

      rv_mu_ttbarmc_sig_ldp  -> setConstant( kTRUE ) ;
      rv_mu_WJmc_sig_ldp     -> setConstant( kTRUE ) ;
      rv_mu_Znnmc_sig_ldp    -> setConstant( kTRUE ) ;
      rv_mu_Ewomc_sig_ldp    -> setConstant( kTRUE ) ;


     //-- SB, LDP

      rv_mu_ttbarmc_sb_ldp   = new RooRealVar( "mu_ttbarmc_sb_ldp" ,"mu_ttbarmc_sb_ldp" , 0., 1000. ) ;
      rv_mu_WJmc_sb_ldp      = new RooRealVar( "mu_WJmc_sb_ldp"    ,"mu_WJmc_sb_ldp"    , 0., 1000. ) ;
      rv_mu_Znnmc_sb_ldp     = new RooRealVar( "mu_Znnmc_sb_ldp"   ,"mu_Znnmc_sb_ldp"   , 0., 1000. ) ;
      rv_mu_Ewomc_sb_ldp     = new RooRealVar( "mu_Ewomc_sb_ldp"   ,"mu_Ewomc_sb_ldp"   , 0., 1000. ) ;

      rv_mu_ttbarmc_sb_ldp  -> setVal( Nttbarmc_sb_ldp ) ;
      rv_mu_WJmc_sb_ldp     -> setVal( NWJmc_sb_ldp ) ;
      rv_mu_Znnmc_sb_ldp    -> setVal( NZnnmc_sb_ldp ) ;
      rv_mu_Ewomc_sb_ldp    -> setVal( NEwomc_sb_ldp ) ;

      rv_mu_ttbarmc_sb_ldp  -> setConstant( kTRUE ) ;
      rv_mu_WJmc_sb_ldp     -> setConstant( kTRUE ) ;
      rv_mu_Znnmc_sb_ldp    -> setConstant( kTRUE ) ;
      rv_mu_Ewomc_sb_ldp    -> setConstant( kTRUE ) ;


     //-- SIG

      rv_mu_Ewomc_sig     = new RooRealVar( "mu_Ewomc_sig"   ,"mu_Ewomc_sig"   , 0., 1000. ) ;
      rv_mu_Ewomc_sig    -> setVal( NEwomc_sig ) ;
      rv_mu_Ewomc_sig    -> setConstant( kTRUE ) ;

     //-- SB

      rv_mu_Ewomc_sb     = new RooRealVar( "mu_Ewomc_sb"   ,"mu_Ewomc_sb"   , 0., 1000. ) ;
      rv_mu_Ewomc_sb    -> setVal( NEwomc_sb ) ;
      rv_mu_Ewomc_sb    -> setConstant( kTRUE ) ;





    //+++++++ Gaussian constraints ++++++++++++++++++++++++++++++++

      printf(" --- Defining Gaussian constraint and constant parameters.\n" ) ;

    //_______ Efficiency scale factor.  Applied to SUSY and all MC inputs _______________

      double pmin, pmax ;

      pmin = (EffScaleFactor-4.*EffScaleFactorErr) ;
      pmax = (EffScaleFactor+4.*EffScaleFactorErr) ;
      if ( pmin < 0 ) pmin = 0.1 ;
      rv_eff_sf  = new RooRealVar( "eff_sf"      , "eff_sf"      , pmin, pmax ) ;
      rv_eff_sf  -> setVal( EffScaleFactor ) ;


      pmin = (lsf_Znnmc-4.*lsf_Znnmc_err) ;
      pmax = (lsf_Znnmc+4.*lsf_Znnmc_err) ;
      if ( pmin < 0 ) pmin = 0.1 ;
      rv_lsf_Znnmc  = new RooRealVar( "lsf_Znnmc", "lsf_Znnmc", pmin, pmax ) ;
      rv_lsf_Znnmc  -> setVal( lsf_Znnmc ) ;


      pmin = (lsf_WJmc-4.*lsf_WJmc_err) ;
      pmax = (lsf_WJmc+4.*lsf_WJmc_err) ;
      if ( pmin < 0 ) pmin = 0.1 ;
      rv_lsf_WJmc  = new RooRealVar( "lsf_WJmc", "lsf_WJmc", pmin, pmax ) ;
      rv_lsf_WJmc  -> setVal( lsf_WJmc ) ;


      pmin = (lsf_Ewomc-4.*lsf_Ewomc_err) ;
      pmax = (lsf_Ewomc+4.*lsf_Ewomc_err) ;
      if ( pmin < 0 ) pmin = 0.1 ;
      rv_lsf_Ewomc  = new RooRealVar( "lsf_Ewomc", "lsf_Ewomc", pmin, pmax ) ;
      rv_lsf_Ewomc  -> setVal( lsf_Ewomc ) ;



     //-- adding this here by hand.
      rv_sf_ttbarmc   = new RooRealVar( "sf_ttbarmc", "sf_ttbarmc", 0.1, 2.2 ) ;
      sf_ttbarmc = 1.0 ;
      sf_ttbarmc_err = 0.2 ;
      rv_sf_ttbarmc   -> setVal( sf_ttbarmc ) ;



    //-- Z to nunu stuff

      pmin = (acc_ee_mean-4.*acc_ee_err) ;
      pmax = (acc_ee_mean+4.*acc_ee_err) ;
      if ( pmin < 0 ) pmin = 0.1 ;
      rv_acc_ee  = new RooRealVar( "acc_ee", "acc_ee", pmin, pmax ) ;
      rv_acc_ee  -> setVal( acc_ee_mean ) ;

      pmin = (acc_mm_mean-4.*acc_mm_err) ;
      pmax = (acc_mm_mean+4.*acc_mm_err) ;
      if ( pmin < 0 ) pmin = 0.1 ;
      rv_acc_mm  = new RooRealVar( "acc_mm", "acc_mm", pmin, pmax ) ;
      rv_acc_mm  -> setVal( acc_mm_mean ) ;


      pmin = (eff_ee_mean-4.*eff_ee_err) ;
      pmax = (eff_ee_mean+4.*eff_ee_err) ;
      if ( pmin < 0 ) pmin = 0.1 ;
      rv_eff_ee  = new RooRealVar( "eff_ee", "eff_ee", pmin, pmax ) ;
      rv_eff_ee  -> setVal( eff_ee_mean ) ;

      pmin = (eff_mm_mean-4.*eff_mm_err) ;
      pmax = (eff_mm_mean+4.*eff_mm_err) ;
      if ( pmin < 0 ) pmin = 0.1 ;
      rv_eff_mm  = new RooRealVar( "eff_mm", "eff_mm", pmin, pmax ) ;
      rv_eff_mm  -> setVal( eff_mm_mean ) ;


      rv_znnoverll_bfratio = new RooRealVar( "znnoverll_bfratio", "znnoverll_bfratio", 0.1, 10. ) ;
      rv_znnoverll_bfratio -> setVal( 5.95 ) ;
      rv_znnoverll_bfratio -> setConstant( kTRUE ) ;

      rv_dataoverll_lumiratio = new RooRealVar( "dataoverll_lumiratio", "dataoverll_lumiratio", 0.1, 10.0 ) ;
      rv_dataoverll_lumiratio  -> setVal( DataLumi / Ztoll_lumi ) ;
      rv_dataoverll_lumiratio  -> setConstant( kTRUE ) ;






     //+++++++++++++++++ Relationships between parameters ++++++++++++++++++++++++++++++++++++++++++++

       printf(" --- Defining relationships between parameters.\n" ) ;




    //-- ttwj

      if ( useSigTtwjVar ) {
         rfv_mu_ttwj_sb = new RooFormulaVar( "mu_ttwj_sb",
                                                       "mu_ttwj_sig*(mu_ttwj_sb_sl/mu_ttwj_sig_sl)",
                                                       RooArgSet( *rv_mu_ttwj_sig, *rv_mu_ttwj_sb_sl, *rv_mu_ttwj_sig_sl ) ) ;
         rv_mu_ttwj_sb = rfv_mu_ttwj_sb ;
      } else {
         rfv_mu_ttwj_sig = new RooFormulaVar( "mu_ttwj_sig",
                                                       "mu_ttwj_sb*(mu_ttwj_sig_sl/mu_ttwj_sb_sl)",
                                                       RooArgSet( *rv_mu_ttwj_sb, *rv_mu_ttwj_sig_sl, *rv_mu_ttwj_sb_sl ) ) ;
         rv_mu_ttwj_sig = rfv_mu_ttwj_sig ;
      }

      rv_mu_ttwj_sig_ldp = new RooFormulaVar( "mu_ttwj_sig_ldp",
                              "sf_ttbarmc*mu_ttbarmc_sig_ldp + lsf_WJmc*mu_WJmc_sig_ldp",
                              RooArgSet( *rv_sf_ttbarmc, *rv_mu_ttbarmc_sig_ldp,
                                         *rv_lsf_WJmc, *rv_mu_WJmc_sig_ldp ) ) ;


      rv_mu_ttwj_sb_ldp = new RooFormulaVar( "mu_ttwj_sb_ldp",
                              "sf_ttbarmc*mu_ttbarmc_sb_ldp + lsf_WJmc*mu_WJmc_sb_ldp",
                              RooArgSet( *rv_sf_ttbarmc, *rv_mu_ttbarmc_sb_ldp,
                                         *rv_lsf_WJmc, *rv_mu_WJmc_sb_ldp ) ) ;






    //-- QCD

      if ( useLdpVars ) {

         rfv_mu_qcd_sig = new RooFormulaVar( "mu_qcd_sig",
                                     "mu_qcd_sig_ldp * ( mu_qcd_lsb_0b / mu_qcd_lsb_0b_ldp )",
                                     RooArgSet( *rv_mu_qcd_sig_ldp, *rv_mu_qcd_lsb_0b, *rv_mu_qcd_lsb_0b_ldp ) ) ;
         rv_mu_qcd_sig = rfv_mu_qcd_sig ;

         rfv_mu_qcd_sb = new RooFormulaVar( "mu_qcd_sb",
                                     "mu_qcd_sb_ldp * ( mu_qcd_lsb_0b / mu_qcd_lsb_0b_ldp )",
                                     RooArgSet( *rv_mu_qcd_sb_ldp, *rv_mu_qcd_lsb_0b, *rv_mu_qcd_lsb_0b_ldp ) ) ;
         rv_mu_qcd_sb = rfv_mu_qcd_sb ;

      } else {

         rfv_mu_qcd_sig_ldp = new RooFormulaVar( "mu_qcd_sig_ldp",
                                     "mu_qcd_sig * ( mu_qcd_lsb_0b_ldp / mu_qcd_lsb_0b )",
                                     RooArgSet( *rv_mu_qcd_sig, *rv_mu_qcd_lsb_0b_ldp, *rv_mu_qcd_lsb_0b ) ) ;
         rv_mu_qcd_sig_ldp = rfv_mu_qcd_sig_ldp ;

         rfv_mu_qcd_sb_ldp = new RooFormulaVar( "mu_qcd_sb_ldp",
                                     "mu_qcd_sb * ( mu_qcd_lsb_0b_ldp / mu_qcd_lsb_0b )",
                                     RooArgSet( *rv_mu_qcd_sb, *rv_mu_qcd_lsb_0b_ldp, *rv_mu_qcd_lsb_0b ) ) ;
         rv_mu_qcd_sb_ldp = rfv_mu_qcd_sb_ldp ;

      }




    //-- SUSY

      rv_mu_susy_sb = new RooFormulaVar( "mu_susy_sb",
                                        "mu_susymc_sb * (mu_susy_sig/mu_susymc_sig)",
                                        RooArgSet( *rv_mu_susymc_sb, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sig_sl = new RooFormulaVar( "mu_susy_sig_sl",
                                        "mu_susymc_sig_sl * (mu_susy_sig/mu_susymc_sig)",
                                        RooArgSet( *rv_mu_susymc_sig_sl, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sb_sl = new RooFormulaVar( "mu_susy_sb_sl",
                                        "mu_susymc_sb_sl * (mu_susy_sig/mu_susymc_sig)",
                                        RooArgSet( *rv_mu_susymc_sb_sl, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sig_ldp = new RooFormulaVar( "mu_susy_sig_ldp",
                                        "mu_susymc_sig_ldp * (mu_susy_sig/mu_susymc_sig)",
                                        RooArgSet( *rv_mu_susymc_sig_ldp, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sb_ldp = new RooFormulaVar( "mu_susy_sb_ldp",
                                        "mu_susymc_sb_ldp * (mu_susy_sig/mu_susymc_sig)",
                                        RooArgSet( *rv_mu_susymc_sb_ldp, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;





    //-- Z to nu nu

      rv_mu_zee_sb_ee = new RooFormulaVar( "mu_zee_sb_ee",
                                      "mu_znn_sb * ( acc_ee * eff_ee ) / ( znnoverll_bfratio * dataoverll_lumiratio )",
                                      RooArgSet( *rv_mu_znn_sb, *rv_acc_ee, *rv_eff_ee, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

      rv_mu_zee_sig_ee = new RooFormulaVar( "mu_zee_sig_ee",
                                      "mu_znn_sig * ( acc_ee * eff_ee ) / ( znnoverll_bfratio * dataoverll_lumiratio )",
                                      RooArgSet( *rv_mu_znn_sig, *rv_acc_ee, *rv_eff_ee, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

      rv_mu_zmm_sb_mm = new RooFormulaVar( "mu_zmm_sb_mm",
                                      "mu_znn_sb * ( acc_mm * eff_mm ) / ( znnoverll_bfratio * dataoverll_lumiratio )",
                                      RooArgSet( *rv_mu_znn_sb, *rv_acc_mm, *rv_eff_mm, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

      rv_mu_zmm_sig_mm = new RooFormulaVar( "mu_zmm_sig_mm",
                                      "mu_znn_sig * ( acc_mm * eff_mm ) / ( znnoverll_bfratio * dataoverll_lumiratio )",
                                      RooArgSet( *rv_mu_znn_sig, *rv_acc_mm, *rv_eff_mm, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

      rv_mu_znn_sig_ldp = new RooFormulaVar( "mu_znn_sig_ldp",
                              "lsf_Znnmc*mu_Znnmc_sig_ldp",
                              RooArgSet( *rv_lsf_Znnmc, *rv_mu_Znnmc_sig_ldp ) ) ;

      rv_mu_znn_sb_ldp = new RooFormulaVar( "mu_znn_sb_ldp",
                              "lsf_Znnmc*mu_Znnmc_sb_ldp",
                              RooArgSet( *rv_lsf_Znnmc, *rv_mu_Znnmc_sb_ldp ) ) ;




    //-- EWO

      rv_mu_ewo_sig     = new RooFormulaVar( "mu_ewo_sig"     , "lsf_Ewomc*mu_Ewomc_sig"     , RooArgSet( *rv_lsf_Ewomc, *rv_mu_Ewomc_sig     ) ) ;
      rv_mu_ewo_sb      = new RooFormulaVar( "mu_ewo_sb"      , "lsf_Ewomc*mu_Ewomc_sb"      , RooArgSet( *rv_lsf_Ewomc, *rv_mu_Ewomc_sb      ) ) ;
      rv_mu_ewo_sig_ldp = new RooFormulaVar( "mu_ewo_sig_ldp" , "lsf_Ewomc*mu_Ewomc_sig_ldp" , RooArgSet( *rv_lsf_Ewomc, *rv_mu_Ewomc_sig_ldp ) ) ;
      rv_mu_ewo_sb_ldp  = new RooFormulaVar( "mu_ewo_sb_ldp"  , "lsf_Ewomc*mu_Ewomc_sb_ldp"  , RooArgSet( *rv_lsf_Ewomc, *rv_mu_Ewomc_sb_ldp  ) ) ;





    //+++++++++++++ Expected counts for observables in terms of parameters ++++++++++++++++++

       printf(" --- Defining expected counts in terms of parameters.\n" ) ;


      rv_n_sig         = new RooFormulaVar( "n_sig",
                                     "mu_ttwj_sig + mu_qcd_sig + mu_znn_sig + eff_sf*( mu_ewo_sig + mu_susy_sig)",
                                     RooArgSet( *rv_mu_ttwj_sig, *rv_mu_qcd_sig, *rv_mu_znn_sig, *rv_eff_sf, *rv_mu_ewo_sig, *rv_mu_susy_sig ) ) ;

      rv_n_sb          = new RooFormulaVar( "n_sb",
                                     "mu_ttwj_sb  + mu_qcd_sb  + mu_znn_sb  + eff_sf*( mu_ewo_sb  + mu_susy_sb )",
                                     RooArgSet( *rv_mu_ttwj_sb , *rv_mu_qcd_sb , *rv_mu_znn_sb , *rv_eff_sf, *rv_mu_ewo_sb , *rv_mu_susy_sb  ) ) ;

      rv_n_sig_ldp     = new RooFormulaVar( "n_sig_ldp",
                                     "mu_qcd_sig_ldp + eff_sf*( mu_ttwj_sig_ldp + mu_znn_sig_ldp + mu_ewo_sig_ldp + mu_susy_sig_ldp)",
                                     RooArgSet( *rv_mu_qcd_sig_ldp, *rv_eff_sf, *rv_mu_ttwj_sig_ldp, *rv_mu_znn_sig_ldp, *rv_mu_ewo_sig_ldp, *rv_mu_susy_sig_ldp ) ) ;

      rv_n_sb_ldp      = new RooFormulaVar( "n_sb_ldp",
                                     "mu_qcd_sb_ldp + eff_sf*( mu_ttwj_sb_ldp + mu_znn_sb_ldp + mu_ewo_sb_ldp + mu_susy_sb_ldp)",
                                     RooArgSet( *rv_mu_qcd_sb_ldp, *rv_eff_sf, *rv_mu_ttwj_sb_ldp, *rv_mu_znn_sb_ldp, *rv_mu_ewo_sb_ldp, *rv_mu_susy_sb_ldp ) ) ;

      rv_n_sig_sl      = new RooFormulaVar( "n_sig_sl",
                                     "mu_ttwj_sig_sl + eff_sf*mu_susy_sig_sl",
                                     RooArgSet( *rv_mu_ttwj_sig_sl, *rv_eff_sf, *rv_mu_susy_sig_sl ) ) ;

      rv_n_sb_sl       = new RooFormulaVar( "n_sb_sl",
                                     "mu_ttwj_sb_sl + eff_sf*mu_susy_sb_sl",
                                     RooArgSet( *rv_mu_ttwj_sb_sl, *rv_eff_sf, *rv_mu_susy_sb_sl ) ) ;

      rv_n_sig_ee      = new RooFormulaVar( "n_sig_ee",
                                     "mu_zee_sig_ee",
                                     RooArgSet( *rv_mu_zee_sig_ee ) ) ;

      rv_n_sb_ee       = new RooFormulaVar( "n_sb_ee",
                                     "mu_zee_sb_ee",
                                     RooArgSet( *rv_mu_zee_sb_ee ) ) ;

      rv_n_sig_mm      = new RooFormulaVar( "n_sig_mm",
                                     "mu_zmm_sig_mm",
                                     RooArgSet( *rv_mu_zmm_sig_mm ) ) ;

      rv_n_sb_mm       = new RooFormulaVar( "n_sb_mm",
                                     "mu_zmm_sb_mm",
                                     RooArgSet( *rv_mu_zmm_sb_mm ) ) ;

      rv_n_lsb_0b      = new RooFormulaVar( "n_lsb_0b",
                                     "mu_qcd_lsb_0b",
                                     RooArgSet( *rv_mu_qcd_lsb_0b ) ) ;

      rv_n_lsb_0b_ldp  = new RooFormulaVar( "n_lsb_0b_ldp",
                                     "mu_qcd_lsb_0b_ldp",
                                     RooArgSet( *rv_mu_qcd_lsb_0b_ldp ) ) ;


   //++++++++++++ PDFs for the likelihood +++++++++++++++++++++++++++++++++++++++++++++

      printf(" --- Defining PDFs of the likelihood.\n" ) ;

      pdf_Nsig        = new RooPoisson( "pdf_Nsig"        , "Nsig Poisson PDF"        , *rv_Nsig        , *rv_n_sig ) ;
      pdf_Nsb         = new RooPoisson( "pdf_Nsb"         , "Nsb Poisson PDF"         , *rv_Nsb         , *rv_n_sb ) ;
      pdf_Nsig_ldp    = new RooPoisson( "pdf_Nsig_ldp"    , "Nsig_ldp Poisson PDF"    , *rv_Nsig_ldp    , *rv_n_sig_ldp ) ;
      pdf_Nsb_ldp     = new RooPoisson( "pdf_Nsb_ldp"     , "Nsb_ldp Poisson PDF"     , *rv_Nsb_ldp     , *rv_n_sb_ldp ) ;
      pdf_Nsig_sl     = new RooPoisson( "pdf_Nsig_sl"     , "Nsig_sl Poisson PDF"     , *rv_Nsig_sl     , *rv_n_sig_sl ) ;
      pdf_Nsb_sl      = new RooPoisson( "pdf_Nsb_sl"      , "Nsb_sl Poisson PDF"      , *rv_Nsb_sl      , *rv_n_sb_sl ) ;
      pdf_Nsig_ee     = new RooPoisson( "pdf_Nsig_ee"     , "Nsig_ee Poisson PDF"     , *rv_Nsig_ee     , *rv_n_sig_ee ) ;
      pdf_Nsb_ee      = new RooPoisson( "pdf_Nsb_ee"      , "Nsb_ee Poisson PDF"      , *rv_Nsb_ee      , *rv_n_sb_ee ) ;
      pdf_Nsig_mm     = new RooPoisson( "pdf_Nsig_mm"     , "Nsig_mm Poisson PDF"     , *rv_Nsig_mm     , *rv_n_sig_mm ) ;
      pdf_Nsb_mm      = new RooPoisson( "pdf_Nsb_mm"      , "Nsb_mm Poisson PDF"      , *rv_Nsb_mm      , *rv_n_sb_mm ) ;
      pdf_Nlsb_0b     = new RooPoisson( "pdf_Nlsb_0b"     , "Nlsb_0b Poisson PDF"     , *rv_Nlsb_0b     , *rv_n_lsb_0b ) ;
      pdf_Nlsb_0b_ldp = new RooPoisson( "pdf_Nlsb_0b_ldp" , "Nlsb_0b_ldp Poisson PDF" , *rv_Nlsb_0b_ldp , *rv_n_lsb_0b_ldp ) ;


      pdf_lsf_WJmc   = new RooGaussian( "pdf_lsf_Wjmc" , "Gaussian pdf for lsf, WJmc",
                                          *rv_lsf_WJmc , RooConst( lsf_WJmc ) , RooConst( lsf_WJmc_err ) ) ;

      pdf_lsf_Znnmc  = new RooGaussian( "pdf_lsf_Znnmc", "Gaussian pdf for lsf, Znnmc",
                                          *rv_lsf_Znnmc, RooConst( lsf_Znnmc ), RooConst( lsf_Znnmc_err ) ) ;

      pdf_lsf_Ewomc  = new RooGaussian( "pdf_lsf_Ewomc", "Gaussian pdf for lsf, Ewomc",
                                          *rv_lsf_Ewomc, RooConst( lsf_Ewomc ), RooConst( lsf_Ewomc_err ) ) ;

      pdf_sf_ttbarmc  = new RooGaussian( "pdf_sf_ttbarmc", "Gaussian pdf for lsf, Ewomc",
                                          *rv_sf_ttbarmc, RooConst( sf_ttbarmc ), RooConst( sf_ttbarmc_err ) ) ;


      pdf_acc_ee   = new RooGaussian( "pdf_acc_ee" , "Gaussian pdf for Z to ee acceptance",
                                          *rv_acc_ee , RooConst( acc_ee_mean ) , RooConst( acc_ee_err ) ) ;

      pdf_acc_mm   = new RooGaussian( "pdf_acc_mm" , "Gaussian pdf for Z to mm acceptance",
                                          *rv_acc_mm , RooConst( acc_mm_mean ) , RooConst( acc_mm_err ) ) ;

      pdf_eff_ee   = new RooGaussian( "pdf_eff_ee" , "Gaussian pdf for Z to ee effeptance",
                                          *rv_eff_ee , RooConst( eff_ee_mean ) , RooConst( eff_ee_err ) ) ;

      pdf_eff_mm   = new RooGaussian( "pdf_eff_mm" , "Gaussian pdf for Z to mm effeptance",
                                          *rv_eff_mm , RooConst( eff_mm_mean ) , RooConst( eff_mm_err ) ) ;



      pdf_Eff_sf     = new RooGaussian( "pdf_Eff_sf", "Gaussian pdf for Efficiency scale factor",
                                          *rv_eff_sf, RooConst( EffScaleFactor ) , RooConst( EffScaleFactorErr ) ) ;





      {
         RooArgSet pdflist ;
         pdflist.add( *pdf_Nsig        ) ;
         pdflist.add( *pdf_Nsb         ) ;
         pdflist.add( *pdf_Nsig_ldp    ) ;
         pdflist.add( *pdf_Nsb_ldp     ) ;
         pdflist.add( *pdf_Nsig_sl     ) ;
         pdflist.add( *pdf_Nsb_sl      ) ;
         pdflist.add( *pdf_Nsig_ee     ) ;
         pdflist.add( *pdf_Nsb_ee      ) ;
         pdflist.add( *pdf_Nsig_mm     ) ;
         pdflist.add( *pdf_Nsb_mm      ) ;
         pdflist.add( *pdf_Nlsb_0b     ) ;
         pdflist.add( *pdf_Nlsb_0b_ldp ) ;
         pdflist.add( *pdf_lsf_WJmc    ) ;
         pdflist.add( *pdf_lsf_Znnmc   ) ;
         pdflist.add( *pdf_lsf_Ewomc   ) ;
         pdflist.add( *pdf_sf_ttbarmc  ) ;
         pdflist.add( *pdf_acc_ee      ) ;
         pdflist.add( *pdf_acc_mm      ) ;
         pdflist.add( *pdf_eff_ee      ) ;
         pdflist.add( *pdf_eff_mm      ) ;
         pdflist.add( *pdf_Eff_sf      ) ;
         likelihood = new RooProdPdf( "likelihood", "ra2b likelihood", pdflist ) ;
      }


     //---- Define the list of observables.

       observedParametersList.add( *rv_Nsig        ) ;
       observedParametersList.add( *rv_Nsb         ) ;
       observedParametersList.add( *rv_Nsig_sl     ) ;
       observedParametersList.add( *rv_Nsb_sl      ) ;
       observedParametersList.add( *rv_Nsig_ldp    ) ;
       observedParametersList.add( *rv_Nsb_ldp     ) ;
       observedParametersList.add( *rv_Nlsb_0b     ) ;
       observedParametersList.add( *rv_Nlsb_0b_ldp ) ;
       observedParametersList.add( *rv_Nsb_ee      ) ;
       observedParametersList.add( *rv_Nsig_ee     ) ;
       observedParametersList.add( *rv_Nsb_mm      ) ;
       observedParametersList.add( *rv_Nsig_mm     ) ;



       dsObserved = new RooDataSet("ra2b_observed_rds", "RA2b observed data values",
                                      observedParametersList ) ;
       dsObserved->add( observedParametersList ) ;


       workspace = new RooWorkspace("ra2bv4ws") ;
       workspace->import( *likelihood ) ;
       workspace->import( *dsObserved ) ;
       printf("\n\n ========== Likelihood configuration:\n\n") ;
       workspace->Print() ;

       initialized = true ;

       return true ;


    } // initialize.

  //===================================================================

    bool ra2bRoostatsClass3::reinitialize( ) {


       printf( "\n\n Opening input file : %s\n\n", initializeFile ) ;

       FILE* infp ;
       if ( (infp=fopen( initializeFile,"r"))==NULL ) {
          printf("\n\n *** Problem opening input file: %s.\n\n", initializeFile ) ;
          return false ;
       }

       int    Nsig                  ;
       int    Nsb                   ;
       int    Nsig_sl               ;
       int    Nsb_sl                ;
       int    Nsig_ldp              ;
       int    Nsb_ldp               ;
       int    Nlsb                  ;
       int    Nlsb_ldp              ;
       int    Nlsb_0b               ;
       int    Nlsb_0b_ldp           ;
       float  Nqcdmc_sig            ;
       float  Nqcdmc_sig_err        ;
       float  Nqcdmc_sb             ;
       float  Nqcdmc_sb_err         ;
       float  Nqcdmc_sig_sl         ;
       float  Nqcdmc_sig_sl_err     ;
       float  Nqcdmc_sb_sl          ;
       float  Nqcdmc_sb_sl_err      ;
       float  Nqcdmc_sig_ldp        ;
       float  Nqcdmc_sig_ldp_err    ;
       float  Nqcdmc_sb_ldp         ;
       float  Nqcdmc_sb_ldp_err     ;
       float  Nqcdmc_lsb            ;
       float  Nqcdmc_lsb_err        ;
       float  Nqcdmc_lsb_ldp        ;
       float  Nqcdmc_lsb_ldp_err    ;
       float  Nqcdmc_lsb_0b         ;
       float  Nqcdmc_lsb_0b_err     ;
       float  Nqcdmc_lsb_0b_ldp     ;
       float  Nqcdmc_lsb_0b_ldp_err ;
       float  Nttbarmc_sig          ;
       float  Nttbarmc_sb           ;
       float  Nttbarmc_sig_sl       ;
       float  Nttbarmc_sb_sl        ;
       float  Nttbarmc_sig_ldp      ;
       float  Nttbarmc_sb_ldp       ;
       float  Nttbarmc_lsb          ;
       float  Nttbarmc_lsb_ldp      ;
       int    NWJmc_sig             ;
       int    NWJmc_sb              ;
       int    NWJmc_sig_sl          ;
       int    NWJmc_sb_sl           ;
       int    NWJmc_sig_ldp         ;
       int    NWJmc_sb_ldp          ;
       int    NWJmc_lsb             ;
       int    NWJmc_lsb_ldp         ;
       int    NZnnmc_sig            ;
       int    NZnnmc_sb             ;
       int    NZnnmc_sig_sl         ;
       int    NZnnmc_sb_sl          ;
       int    NZnnmc_sig_ldp        ;
       int    NZnnmc_sb_ldp         ;
       int    NZnnmc_lsb            ;
       int    NZnnmc_lsb_ldp        ;
       int    NEwomc_sig            ;
       int    NEwomc_sb             ;
       int    NEwomc_sig_sl         ;
       int    NEwomc_sb_sl          ;
       int    NEwomc_sig_ldp        ;
       int    NEwomc_sb_ldp         ;
       int    NEwomc_lsb            ;
       int    NEwomc_lsb_ldp        ;
       float  Nsusymc_sig           ;
       float  Nsusymc_sb            ;
       float  Nsusymc_sig_sl        ;
       float  Nsusymc_sb_sl         ;
       float  Nsusymc_sig_ldp       ;
       float  Nsusymc_sb_ldp        ;
       float  Nsusymc_lsb           ;
       float  Nsusymc_lsb_ldp       ;
       float  Nsusymc_lsb_0b        ;
       float  Nsusymc_lsb_0b_ldp    ;
       int    Nhtonlytrig_lsb_0b      ;
       int    Nhtonlytrig_lsb_0b_ldp  ;
       int    Nsb_ee                  ;
       int    Nsig_ee                 ;
       int    Nsb_mm                  ;
       int    Nsig_mm                 ;



       //--- read in description line.
       printf("\n\n") ;
       char c(0) ;
       while ( c!=10  ) { c = fgetc( infp ) ; printf("%c", c ) ; }
       printf("\n\n") ;


       char label[1000] ;

      //--- Inputs generated with gen_roostats_input4.c
      //    The order here must be consistent with the order there!

       fscanf( infp, "%s %g", label, &EffScaleFactor        ) ;   printf( "%s %g\n", label, EffScaleFactor        ) ;
       fscanf( infp, "%s %g", label, &EffScaleFactorErr     ) ;   printf( "%s %g\n", label, EffScaleFactorErr     ) ;
       fscanf( infp, "%s %d", label, &Nsig                  ) ;   printf( "%s %d\n", label, Nsig                  ) ;
       fscanf( infp, "%s %d", label, &Nsb                   ) ;   printf( "%s %d\n", label, Nsb                   ) ;
       fscanf( infp, "%s %d", label, &Nsig_sl               ) ;   printf( "%s %d\n", label, Nsig_sl               ) ;
       fscanf( infp, "%s %d", label, &Nsb_sl                ) ;   printf( "%s %d\n", label, Nsb_sl                ) ;
       fscanf( infp, "%s %d", label, &Nsig_ldp              ) ;   printf( "%s %d\n", label, Nsig_ldp              ) ;
       fscanf( infp, "%s %d", label, &Nsb_ldp               ) ;   printf( "%s %d\n", label, Nsb_ldp               ) ;
       fscanf( infp, "%s %d", label, &Nlsb                  ) ;   printf( "%s %d\n", label, Nlsb                  ) ;
       fscanf( infp, "%s %d", label, &Nlsb_ldp              ) ;   printf( "%s %d\n", label, Nlsb_ldp              ) ;
       fscanf( infp, "%s %d", label, &Nlsb_0b               ) ;   printf( "%s %d\n", label, Nlsb_0b               ) ;
       fscanf( infp, "%s %d", label, &Nlsb_0b_ldp           ) ;   printf( "%s %d\n", label, Nlsb_0b_ldp           ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig            ) ;   printf( "%s %g\n", label, Nqcdmc_sig            ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_err        ) ;   printf( "%s %g\n", label, Nqcdmc_sig_err        ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb             ) ;   printf( "%s %g\n", label, Nqcdmc_sb             ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_err         ) ;   printf( "%s %g\n", label, Nqcdmc_sb_err         ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_sl         ) ;   printf( "%s %g\n", label, Nqcdmc_sig_sl         ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_sl_err     ) ;   printf( "%s %g\n", label, Nqcdmc_sig_sl_err     ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_sl          ) ;   printf( "%s %g\n", label, Nqcdmc_sb_sl          ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_sl_err      ) ;   printf( "%s %g\n", label, Nqcdmc_sb_sl_err      ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_ldp        ) ;   printf( "%s %g\n", label, Nqcdmc_sig_ldp        ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sig_ldp_err    ) ;   printf( "%s %g\n", label, Nqcdmc_sig_ldp_err    ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_ldp         ) ;   printf( "%s %g\n", label, Nqcdmc_sb_ldp         ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_sb_ldp_err     ) ;   printf( "%s %g\n", label, Nqcdmc_sb_ldp_err     ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb            ) ;   printf( "%s %g\n", label, Nqcdmc_lsb            ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_err        ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_err        ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_ldp        ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_ldp        ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_ldp_err    ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_ldp_err    ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_0b         ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_0b         ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_0b_err     ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_0b_err     ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_0b_ldp     ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_0b_ldp     ) ;
       fscanf( infp, "%s %g", label, &Nqcdmc_lsb_0b_ldp_err ) ;   printf( "%s %g\n", label, Nqcdmc_lsb_0b_ldp_err ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sig          ) ;   printf( "%s %g\n", label, Nttbarmc_sig          ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sb           ) ;   printf( "%s %g\n", label, Nttbarmc_sb           ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sig_sl       ) ;   printf( "%s %g\n", label, Nttbarmc_sig_sl       ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sb_sl        ) ;   printf( "%s %g\n", label, Nttbarmc_sb_sl        ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sig_ldp      ) ;   printf( "%s %g\n", label, Nttbarmc_sig_ldp      ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sb_ldp       ) ;   printf( "%s %g\n", label, Nttbarmc_sb_ldp       ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_lsb          ) ;   printf( "%s %g\n", label, Nttbarmc_lsb          ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_lsb_ldp      ) ;   printf( "%s %g\n", label, Nttbarmc_lsb_ldp      ) ;
       fscanf( infp, "%s %g", label, &lsf_WJmc              ) ;   printf( "%s %g\n", label, lsf_WJmc              ) ;
       fscanf( infp, "%s %g", label, &lsf_WJmc_err          ) ;   printf( "%s %g\n", label, lsf_WJmc_err          ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sig             ) ;   printf( "%s %d\n", label, NWJmc_sig             ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sb              ) ;   printf( "%s %d\n", label, NWJmc_sb              ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sig_sl          ) ;   printf( "%s %d\n", label, NWJmc_sig_sl          ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sb_sl           ) ;   printf( "%s %d\n", label, NWJmc_sb_sl           ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sig_ldp         ) ;   printf( "%s %d\n", label, NWJmc_sig_ldp         ) ;
       fscanf( infp, "%s %d", label, &NWJmc_sb_ldp          ) ;   printf( "%s %d\n", label, NWJmc_sb_ldp          ) ;
       fscanf( infp, "%s %d", label, &NWJmc_lsb             ) ;   printf( "%s %d\n", label, NWJmc_lsb             ) ;
       fscanf( infp, "%s %d", label, &NWJmc_lsb_ldp         ) ;   printf( "%s %d\n", label, NWJmc_lsb_ldp         ) ;
       fscanf( infp, "%s %g", label, &lsf_Znnmc             ) ;   printf( "%s %g\n", label, lsf_Znnmc             ) ;
       fscanf( infp, "%s %g", label, &lsf_Znnmc_err         ) ;   printf( "%s %g\n", label, lsf_Znnmc_err         ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sig            ) ;   printf( "%s %d\n", label, NZnnmc_sig            ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sb             ) ;   printf( "%s %d\n", label, NZnnmc_sb             ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sig_sl         ) ;   printf( "%s %d\n", label, NZnnmc_sig_sl         ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sb_sl          ) ;   printf( "%s %d\n", label, NZnnmc_sb_sl          ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sig_ldp        ) ;   printf( "%s %d\n", label, NZnnmc_sig_ldp        ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_sb_ldp         ) ;   printf( "%s %d\n", label, NZnnmc_sb_ldp         ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_lsb            ) ;   printf( "%s %d\n", label, NZnnmc_lsb            ) ;
       fscanf( infp, "%s %d", label, &NZnnmc_lsb_ldp        ) ;   printf( "%s %d\n", label, NZnnmc_lsb_ldp        ) ;
       fscanf( infp, "%s %g", label, &lsf_Ewomc             ) ;   printf( "%s %g\n", label, lsf_Ewomc             ) ;
       fscanf( infp, "%s %g", label, &lsf_Ewomc_err         ) ;   printf( "%s %g\n", label, lsf_Ewomc_err         ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sig            ) ;   printf( "%s %d\n", label, NEwomc_sig            ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sb             ) ;   printf( "%s %d\n", label, NEwomc_sb             ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sig_sl         ) ;   printf( "%s %d\n", label, NEwomc_sig_sl         ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sb_sl          ) ;   printf( "%s %d\n", label, NEwomc_sb_sl          ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sig_ldp        ) ;   printf( "%s %d\n", label, NEwomc_sig_ldp        ) ;
       fscanf( infp, "%s %d", label, &NEwomc_sb_ldp         ) ;   printf( "%s %d\n", label, NEwomc_sb_ldp         ) ;
       fscanf( infp, "%s %d", label, &NEwomc_lsb            ) ;   printf( "%s %d\n", label, NEwomc_lsb            ) ;
       fscanf( infp, "%s %d", label, &NEwomc_lsb_ldp        ) ;   printf( "%s %d\n", label, NEwomc_lsb_ldp        ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sig           ) ;   printf( "%s %g\n", label, Nsusymc_sig           ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sb            ) ;   printf( "%s %g\n", label, Nsusymc_sb            ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sig_sl        ) ;   printf( "%s %g\n", label, Nsusymc_sig_sl        ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sb_sl         ) ;   printf( "%s %g\n", label, Nsusymc_sb_sl         ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sig_ldp       ) ;   printf( "%s %g\n", label, Nsusymc_sig_ldp       ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_sb_ldp        ) ;   printf( "%s %g\n", label, Nsusymc_sb_ldp        ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_lsb           ) ;   printf( "%s %g\n", label, Nsusymc_lsb           ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_lsb_ldp       ) ;   printf( "%s %g\n", label, Nsusymc_lsb_ldp       ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_lsb_0b        ) ;   printf( "%s %g\n", label, Nsusymc_lsb_0b        ) ;
       fscanf( infp, "%s %g", label, &Nsusymc_lsb_0b_ldp    ) ;   printf( "%s %g\n", label, Nsusymc_lsb_0b_ldp    ) ;
       fscanf( infp, "%s %d", label, &Nhtonlytrig_lsb_0b        ) ;   printf( "%s %d\n", label, Nhtonlytrig_lsb_0b        ) ;
       fscanf( infp, "%s %d", label, &Nhtonlytrig_lsb_0b_ldp    ) ;   printf( "%s %d\n", label, Nhtonlytrig_lsb_0b_ldp    ) ;
       fscanf( infp, "%s %d", label, &Nsb_ee                    ) ;   printf( "%s %d\n", label, Nsb_ee                    ) ;
       fscanf( infp, "%s %d", label, &Nsig_ee                   ) ;   printf( "%s %d\n", label, Nsig_ee                   ) ;
       fscanf( infp, "%s %d", label, &Nsb_mm                    ) ;   printf( "%s %d\n", label, Nsb_mm                    ) ;
       fscanf( infp, "%s %d", label, &Nsig_mm                   ) ;   printf( "%s %d\n", label, Nsig_mm                   ) ;
       fscanf( infp, "%s %g", label, &acc_ee_mean               ) ;   printf( "%s %g\n", label, acc_ee_mean               ) ;
       fscanf( infp, "%s %g", label, &acc_ee_err                ) ;   printf( "%s %g\n", label, acc_ee_err                ) ;
       fscanf( infp, "%s %g", label, &acc_mm_mean               ) ;   printf( "%s %g\n", label, acc_mm_mean               ) ;
       fscanf( infp, "%s %g", label, &acc_mm_err                ) ;   printf( "%s %g\n", label, acc_mm_err                ) ;
       fscanf( infp, "%s %g", label, &eff_ee_mean               ) ;   printf( "%s %g\n", label, eff_ee_mean               ) ;
       fscanf( infp, "%s %g", label, &eff_ee_err                ) ;   printf( "%s %g\n", label, eff_ee_err                ) ;
       fscanf( infp, "%s %g", label, &eff_mm_mean               ) ;   printf( "%s %g\n", label, eff_mm_mean               ) ;
       fscanf( infp, "%s %g", label, &eff_mm_err                ) ;   printf( "%s %g\n", label, eff_mm_err                ) ;
       fscanf( infp, "%s %g", label, &Ztoll_lumi                ) ;   printf( "%s %g\n", label, Ztoll_lumi                ) ;
       fscanf( infp, "%s %g", label, &Ztoll_tight_sf            ) ;   printf( "%s %g\n", label, Ztoll_tight_sf            ) ;
       fscanf( infp, "%s %g", label, &Ztoll_tight_sf_err        ) ;   printf( "%s %g\n", label, Ztoll_tight_sf_err        ) ;
       fscanf( infp, "%s %g", label, &DataLumi                  ) ;   printf( "%s %g\n", label, DataLumi                  ) ;

       printf("\n Done reading in %s\n\n", initializeFile ) ;
       fclose( infp ) ;


       //--- Print out a nice summary of the inputs.

       float Nsm_sig         = Nttbarmc_sig          +  lsf_WJmc*NWJmc_sig          +  Nqcdmc_sig          +  lsf_Znnmc*NZnnmc_sig          +  lsf_Ewomc*NEwomc_sig         ;
       float Nsm_sb          = Nttbarmc_sb           +  lsf_WJmc*NWJmc_sb           +  Nqcdmc_sb           +  lsf_Znnmc*NZnnmc_sb           +  lsf_Ewomc*NEwomc_sb          ;
       float Nsm_sig_sl      = Nttbarmc_sig_sl       +  lsf_WJmc*NWJmc_sig_sl       +  Nqcdmc_sig_sl       +  lsf_Znnmc*NZnnmc_sig_sl       +  lsf_Ewomc*NEwomc_sig_sl      ;
       float Nsm_sb_sl       = Nttbarmc_sb_sl        +  lsf_WJmc*NWJmc_sb_sl        +  Nqcdmc_sb_sl        +  lsf_Znnmc*NZnnmc_sb_sl        +  lsf_Ewomc*NEwomc_sb_sl       ;
       float Nsm_sig_ldp     = Nttbarmc_sig_ldp      +  lsf_WJmc*NWJmc_sig_ldp      +  Nqcdmc_sig_ldp      +  lsf_Znnmc*NZnnmc_sig_ldp      +  lsf_Ewomc*NEwomc_sig_ldp     ;
       float Nsm_sb_ldp      = Nttbarmc_sb_ldp       +  lsf_WJmc*NWJmc_sb_ldp       +  Nqcdmc_sb_ldp       +  lsf_Znnmc*NZnnmc_sb_ldp       +  lsf_Ewomc*NEwomc_sb_ldp      ;
       float Nsm_lsb         = Nttbarmc_lsb          +  lsf_WJmc*NWJmc_lsb          +  Nqcdmc_lsb          +  lsf_Znnmc*NZnnmc_lsb          +  lsf_Ewomc*NEwomc_lsb         ;
       float Nsm_lsb_ldp     = Nttbarmc_lsb_ldp      +  lsf_WJmc*NWJmc_lsb_ldp      +  Nqcdmc_lsb_ldp      +  lsf_Znnmc*NZnnmc_lsb_ldp      +  lsf_Ewomc*NEwomc_lsb_ldp     ;
       float Nsm_lsb_0b      =                                                         Nqcdmc_lsb_0b                                                                        ;
       float Nsm_lsb_0b_ldp  =                                                         Nqcdmc_lsb_0b_ldp                                                                    ;



       //--- QCD min Delta phi N  pass / fail ratios.

       float Rqcd_sig        =  0. ;
       if ( Nqcdmc_sig_ldp > 0. ) { Rqcd_sig        =  Nqcdmc_sig        / Nqcdmc_sig_ldp ;  }
       float Rqcd_sig_err    =  0. ;
       if ( Nqcdmc_sig > 0. && Nqcdmc_sig_ldp > 0. ) { Rqcd_sig_err  =  Rqcd_sig * sqrt( pow(Nqcdmc_sig_err/Nqcdmc_sig,2) + pow(Nqcdmc_sig_ldp_err/Nqcdmc_sig_ldp,2) ) ; }


       float Rqcd_sb        =  0. ;
       if ( Nqcdmc_sb_ldp > 0. ) { Rqcd_sb        =  Nqcdmc_sb        / Nqcdmc_sb_ldp ;  }
       float Rqcd_sb_err    =  0. ;
       if ( Nqcdmc_sb > 0. && Nqcdmc_sb_ldp > 0. ) { Rqcd_sb_err  =  Rqcd_sb * sqrt( pow(Nqcdmc_sb_err/Nqcdmc_sb,2) + pow(Nqcdmc_sb_ldp_err/Nqcdmc_sb_ldp,2) ) ; }


       float Rqcd_lsb        =  0. ;
       if ( Nqcdmc_lsb_ldp > 0. ) { Rqcd_lsb        =  Nqcdmc_lsb        / Nqcdmc_lsb_ldp ;  }
       float Rqcd_lsb_err    =  0. ;
       if ( Nqcdmc_lsb > 0. && Nqcdmc_lsb_ldp > 0. ) { Rqcd_lsb_err  =  Rqcd_lsb * sqrt( pow(Nqcdmc_lsb_err/Nqcdmc_lsb,2) + pow(Nqcdmc_lsb_ldp_err/Nqcdmc_lsb_ldp,2) ) ; }


       float Nlsb_corrected     = Nlsb     - Nttbarmc_lsb     - lsf_WJmc*NWJmc_lsb     - lsf_Znnmc*NZnnmc_lsb     - lsf_Ewomc*NEwomc_lsb ;
       float Nlsb_ldp_corrected = Nlsb_ldp - Nttbarmc_lsb_ldp - lsf_WJmc*NWJmc_lsb_ldp - lsf_Znnmc*NZnnmc_lsb_ldp - lsf_Ewomc*NEwomc_lsb_ldp ;
       float Rdata_lsb = 0. ;
       if ( Nlsb_ldp_corrected > 0 ) { Rdata_lsb = (Nlsb_corrected) / (Nlsb_ldp_corrected) ; }
       float Rdata_lsb_err = 0. ;
       if ( Nlsb_corrected > 0 && Nlsb_ldp_corrected > 0 ) { Rdata_lsb_err = Rdata_lsb * sqrt( 1.0/(Nlsb_corrected) + 1.0/(Nlsb_ldp_corrected) ) ; }


       float Rqcd_lsb_0b        =  0. ;
       if ( Nqcdmc_lsb_0b_ldp > 0. ) { Rqcd_lsb_0b        =  Nqcdmc_lsb_0b        / Nqcdmc_lsb_0b_ldp ;  }
       float Rqcd_lsb_0b_err    =  0. ;
       if ( Nqcdmc_lsb_0b > 0. && Nqcdmc_lsb_0b_ldp > 0. ) { Rqcd_lsb_0b_err  =  Rqcd_lsb_0b * sqrt( pow(Nqcdmc_lsb_0b_err/Nqcdmc_lsb_0b,2) + pow(Nqcdmc_lsb_0b_ldp_err/Nqcdmc_lsb_0b_ldp,2) ) ; }


       float Rdata_lsb_0b = 0. ;
       if ( Nlsb_0b_ldp > 0 ) { Rdata_lsb_0b = (1.0*Nlsb_0b) / (1.0*Nlsb_0b_ldp) ; }
       float Rdata_lsb_0b_err = 0. ;
       if ( Nlsb_0b > 0 && Nlsb_0b_ldp > 0 ) { Rdata_lsb_0b_err = Rdata_lsb_0b * sqrt( 1.0/(1.0*Nlsb_0b) + 1.0/(1.0*Nlsb_0b_ldp) ) ; }


       //--- ttbar + Wjets  MET  SIG / SB ratios

       float Rttwj = 0. ;
       if ( (Nttbarmc_sb+lsf_WJmc*NWJmc_sb) > 0. ) { Rttwj    =   (Nttbarmc_sig+lsf_WJmc*NWJmc_sig) / (Nttbarmc_sb+lsf_WJmc*NWJmc_sb) ; }
       float Rttwj_err2 = pow( lsf_WJmc/ (Nttbarmc_sb+lsf_WJmc*NWJmc_sb),2)*NWJmc_sig
                        + pow( (Nttbarmc_sig+lsf_WJmc*NWJmc_sig) * lsf_WJmc / pow((Nttbarmc_sb+lsf_WJmc*NWJmc_sb),2), 2)*NWJmc_sb ;
       float Rttwj_err = sqrt(Rttwj_err2) ;

       float Rttwj_sl = 0. ;
       if ( (Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl) > 0. ) { Rttwj_sl    =   (Nttbarmc_sig_sl+lsf_WJmc*NWJmc_sig_sl) / (Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl) ; }
       float Rttwj_sl_err2 = pow( lsf_WJmc / (Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl), 2) * NWJmc_sig_sl
                           + pow( lsf_WJmc * (Nttbarmc_sig_sl+lsf_WJmc*NWJmc_sig_sl) / pow((Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl),2), 2)*NWJmc_sb_sl ;
       float Rttwj_sl_err = sqrt(Rttwj_sl_err2) ;

       float Rdata_sl = 0. ;
       if ( Nsb_sl > 0 ) { Rdata_sl = (1.0*Nsig_sl)/(1.0*Nsb_sl) ; }
       float Rdata_sl_err = 0. ;
       if ( Nsig_sl>0 && Nsb_sl>0 ) { Rdata_sl_err = Rdata_sl * sqrt( 1.0/(1.0*Nsig_sl) + 1.0/(1.0*Nsb_sl) ) ; }


       //--- Simple MC closure tests.

       float comp_mc_qcd_sb = Nqcdmc_sb_ldp * ( Nqcdmc_lsb_0b / Nqcdmc_lsb_0b_ldp ) ;
       float comp_mc_qcd_sb_err = Nqcdmc_sb_ldp_err * ( Nqcdmc_lsb_0b / Nqcdmc_lsb_0b_ldp ) ;

       float comp_mc_qcd_sig = Nqcdmc_sig_ldp * ( Nqcdmc_lsb_0b / Nqcdmc_lsb_0b_ldp ) ;
       float comp_mc_qcd_sig_err = Nqcdmc_sig_ldp_err * ( Nqcdmc_lsb_0b / Nqcdmc_lsb_0b_ldp ) ;

       float comp_mc_ttwj_sig = (Nttbarmc_sb + lsf_WJmc*NWJmc_sb) * ( (Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl) / (Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl) ) ;

       //--- below ignores ttbar errors, uses sqrt(N) on raw WJ counts.
       float comp_mc_ttwj_sig_err2 = pow( lsf_WJmc*( (Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl) / (Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl) ),2)*NWJmc_sb
                                   + pow( (Nttbarmc_sb + lsf_WJmc*NWJmc_sb) *(lsf_WJmc/ (Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl) ), 2)*NWJmc_sig_sl
                                   + pow( (Nttbarmc_sb + lsf_WJmc*NWJmc_sb) *  (Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl) / pow((Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl) ,2)*lsf_WJmc, 2 )*NWJmc_sb_sl ;
       float comp_mc_ttwj_sig_err = sqrt(comp_mc_ttwj_sig_err2) ;

       //--- Simple data calculations.


       float Nsb_ldp_corrected  = Nsb_ldp  - (Nttbarmc_sb_ldp  + lsf_WJmc*NWJmc_sb_ldp  + lsf_Znnmc*NZnnmc_sb_ldp  + lsf_Ewomc*NEwomc_sb_ldp  ) ;
       float comp_data_qcd_sb = Nsb_ldp_corrected * ( (1.0*Nlsb_0b)/(1.0*Nlsb_0b_ldp) ) ;
       float comp_data_qcd_sb_err = sqrt( Nsb_ldp + Nttbarmc_sb_ldp  + lsf_WJmc*NWJmc_sb_ldp  + lsf_Znnmc*NZnnmc_sb_ldp  + lsf_Ewomc*NEwomc_sb_ldp ) * ( (1.0*Nlsb_0b)/(1.0*Nlsb_0b_ldp) ) ;

       float Nsig_ldp_corrected = Nsig_ldp - (Nttbarmc_sig_ldp + lsf_WJmc*NWJmc_sig_ldp + lsf_Znnmc*NZnnmc_sig_ldp + lsf_Ewomc*NEwomc_sig_ldp ) ;
       float comp_data_qcd_sig = Nsig_ldp_corrected * ( (1.0*Nlsb_0b)/(1.0*Nlsb_0b_ldp) ) ;
       float comp_data_qcd_sig_err = sqrt( Nsig_ldp + Nttbarmc_sig_ldp  + lsf_WJmc*NWJmc_sig_ldp  + lsf_Znnmc*NZnnmc_sig_ldp  + lsf_Ewomc*NEwomc_sig_ldp ) * ( (1.0*Nlsb_0b)/(1.0*Nlsb_0b_ldp) ) ;

       float comp_data_ttwj_sig = (Nsb - (comp_data_qcd_sb + lsf_Znnmc*NZnnmc_sb + lsf_Ewomc*NEwomc_sb)) * ( (1.0*Nsig_sl) / (1.0*Nsb_sl) ) ;
       float comp_data_ttwj_sig_err2 = pow( ( (1.0*Nsig_sl) / (1.0*Nsb_sl) ), 2)*Nsb
                                     + pow( ( (1.0*Nsig_sl) / (1.0*Nsb_sl) ), 2)*pow(comp_data_qcd_sb_err,2)
                                     + pow( (Nsb - (comp_data_qcd_sb + lsf_Znnmc*NZnnmc_sb + lsf_Ewomc*NEwomc_sb)) / (1.0*Nsb_sl), 2 )*Nsig_sl
                                     + pow( (Nsb - (comp_data_qcd_sb + lsf_Znnmc*NZnnmc_sb + lsf_Ewomc*NEwomc_sb)) * (1.0*Nsig_sl) / pow(1.0*Nsb_sl,2), 2 ) * Nsb_sl ;
       float comp_data_ttwj_sig_err = sqrt(comp_data_ttwj_sig_err2) ;


       printf("\n\n\n") ;

       printf("------------+----------+----------+-----------------------+----------+----------+------------+------------+---------------\n") ;
       printf("   Sample   |  ttbar   |  W+jets  |          QCD          |  Z to nn |   other  |   All SM   |    Data    |     SUSY      \n") ;
       printf("------------+----------+----------+-----------------------+----------+----------+------------+------------+---------------\n") ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sig", Nttbarmc_sig, (lsf_WJmc*NWJmc_sig), Nqcdmc_sig, Nqcdmc_sig_err, lsf_Znnmc*NZnnmc_sig, lsf_Ewomc*NEwomc_sig, Nsm_sig, Nsig, Nsusymc_sig ) ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sb", Nttbarmc_sb, (lsf_WJmc*NWJmc_sb), Nqcdmc_sb, Nqcdmc_sb_err, lsf_Znnmc*NZnnmc_sb, lsf_Ewomc*NEwomc_sb, Nsm_sb, Nsb, Nsusymc_sb ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sig_sl", Nttbarmc_sig_sl, (lsf_WJmc*NWJmc_sig_sl), Nqcdmc_sig_sl, Nqcdmc_sig_sl_err, lsf_Znnmc*NZnnmc_sig_sl, lsf_Ewomc*NEwomc_sig_sl, Nsm_sig_sl, Nsig_sl, Nsusymc_sig_sl ) ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sb_sl", Nttbarmc_sb_sl, (lsf_WJmc*NWJmc_sb_sl), Nqcdmc_sb_sl, Nqcdmc_sb_sl_err, lsf_Znnmc*NZnnmc_sb_sl, lsf_Ewomc*NEwomc_sb_sl, Nsm_sb_sl, Nsb_sl, Nsusymc_sb_sl ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sig_ldp", Nttbarmc_sig_ldp, (lsf_WJmc*NWJmc_sig_ldp), Nqcdmc_sig_ldp, Nqcdmc_sig_ldp_err, lsf_Znnmc*NZnnmc_sig_ldp, lsf_Ewomc*NEwomc_sig_ldp, Nsm_sig_ldp, Nsig_ldp, Nsusymc_sig_ldp ) ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "sb_ldp", Nttbarmc_sb_ldp, (lsf_WJmc*NWJmc_sb_ldp), Nqcdmc_sb_ldp, Nqcdmc_sb_ldp_err, lsf_Znnmc*NZnnmc_sb_ldp, lsf_Ewomc*NEwomc_sb_ldp, Nsm_sb_ldp, Nsb_ldp, Nsusymc_sb_ldp ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "lsb", Nttbarmc_lsb, (lsf_WJmc*NWJmc_lsb), Nqcdmc_lsb, Nqcdmc_lsb_err, lsf_Znnmc*NZnnmc_lsb, lsf_Ewomc*NEwomc_lsb, Nsm_lsb, Nlsb, Nsusymc_lsb ) ;
       printf(" %10s | %8.1f | %8.1f | %10.1f +/- %6.1f | %8.1f | %8.1f | %10.1f | %10d | %8.1f\n",
            "lsb_ldp", Nttbarmc_lsb_ldp, (lsf_WJmc*NWJmc_lsb_ldp), Nqcdmc_lsb_ldp, Nqcdmc_lsb_ldp_err, lsf_Znnmc*NZnnmc_lsb_ldp, lsf_Ewomc*NEwomc_lsb_ldp, Nsm_lsb_ldp, Nlsb_ldp, Nsusymc_lsb_ldp ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf(" %10s | -------- | -------- | %10.1f +/- %6.1f | -------- | -------- | %10.1f | %10d | %8.1f\n",
            "lsb_0b",  Nqcdmc_lsb_0b, Nqcdmc_lsb_0b_err, Nsm_lsb_0b, Nlsb_0b, Nsusymc_lsb_0b ) ;
       printf(" %10s | -------- | -------- | %10.1f +/- %6.1f | -------- | -------- | %10.1f | %10d | %8.1f\n",
            "lsb_0b_ldp",  Nqcdmc_lsb_0b_ldp, Nqcdmc_lsb_0b_ldp_err, Nsm_lsb_0b_ldp, Nlsb_0b_ldp, Nsusymc_lsb_0b_ldp ) ;
       printf("            |          |          |                       |          |          |            |            |               \n") ;
       printf("------------+----------+----------+-----------------------+----------+----------+------------+------------+---------------\n") ;


       printf("\n\n\n") ;

       printf(" R QCD  :  sig    /    sig_ldp  : ( %10.1f +/- %6.1f ) / ( %10.1f +/- %6.1f )    =    %5.3f +/- %5.3f\n",
            Nqcdmc_sig, Nqcdmc_sig_err, Nqcdmc_sig_ldp, Nqcdmc_sig_ldp_err, Rqcd_sig, Rqcd_sig_err ) ;

       printf(" R QCD  :  sb     /     sb_ldp  : ( %10.1f +/- %6.1f ) / ( %10.1f +/- %6.1f )    =    %5.3f +/- %5.3f\n",
            Nqcdmc_sb, Nqcdmc_sb_err, Nqcdmc_sb_ldp, Nqcdmc_sb_ldp_err, Rqcd_sb, Rqcd_sb_err ) ;

       printf(" R QCD  :  lsb    /    lsb_ldp  : ( %10.1f +/- %6.1f ) / ( %10.1f +/- %6.1f )    =    %5.3f +/- %5.3f\n",
            Nqcdmc_lsb, Nqcdmc_lsb_err, Nqcdmc_lsb_ldp, Nqcdmc_lsb_ldp_err, Rqcd_lsb, Rqcd_lsb_err ) ;

       printf(" R data :  lsb    /    lsb_ldp  : ( %10.1f            ) / ( %10.1f            )    =    %5.3f +/- %5.3f\n",
            Nlsb_corrected, Nlsb_ldp_corrected, Rdata_lsb, Rdata_lsb_err ) ;

       printf(" R QCD  :  lsb_0b / lsb_0b_ldp  : ( %10.1f +/- %6.1f ) / ( %10.1f +/- %6.1f )    =    %5.3f +/- %5.3f\n",
            Nqcdmc_lsb_0b, Nqcdmc_lsb_0b_err, Nqcdmc_lsb_0b_ldp, Nqcdmc_lsb_0b_ldp_err, Rqcd_lsb_0b, Rqcd_lsb_0b_err ) ;

       printf(" R data :  lsb_0b / lsb_0b_ldp  : ( %8d              ) / ( %8d              )    =    %5.3f +/- %5.3f\n",
            Nlsb_0b, Nlsb_0b_ldp, Rdata_lsb_0b, Rdata_lsb_0b_err ) ;


       printf("\n\n\n") ;


       printf(" R ttwj :  sig    /    sb    :  ( %5.1f +/- %4.1f ) / ( %5.1f +/- %4.1f )   =   %5.3f +/- %5.3f\n",
                 (Nttbarmc_sig+lsf_WJmc*NWJmc_sig), lsf_WJmc*sqrt(NWJmc_sig),
                 (Nttbarmc_sb+lsf_WJmc*NWJmc_sb), lsf_WJmc*sqrt(NWJmc_sb),
                 Rttwj, Rttwj_err ) ;

       printf(" R ttwj :  sig_sl /    sb_sl :  ( %5.1f +/- %4.1f ) / ( %5.1f +/- %4.1f )   =   %5.3f +/- %5.3f\n",
                 (Nttbarmc_sig_sl+lsf_WJmc*NWJmc_sig_sl), lsf_WJmc*sqrt(NWJmc_sig_sl),
                 (Nttbarmc_sb_sl+lsf_WJmc*NWJmc_sb_sl), lsf_WJmc*sqrt(NWJmc_sb_sl),
                 Rttwj_sl, Rttwj_sl_err ) ;

       printf(" R data :  sig_sl /    sb_sl :    %3d              /   %3d                =   %5.3f +/- %5.3f\n",
                 Nsig_sl, Nsb_sl, Rdata_sl, Rdata_sl_err ) ;



       printf("\n\n\n") ;


       printf(" ----- Simple MC closure tests\n\n") ;

       printf("  QCD :  sb =  sb_ldp * (lsb_0b/lsb_0b_ldp) : ( %6.1f +/- %4.1f ) * ( %8.1f / %10.1f ) = %8.1f +/- %6.1f\n",
             Nqcdmc_sb_ldp, Nqcdmc_sb_ldp_err, Nqcdmc_lsb_0b, Nqcdmc_lsb_0b_ldp, comp_mc_qcd_sb, comp_mc_qcd_sb_err ) ;
       printf("                                                                               MC truth value : %8.1f +/- %6.1f\n",
                   Nqcdmc_sb, Nqcdmc_sb_err ) ;

       printf("\n") ;
       printf("  QCD : sig = sig_ldp * (lsb_0b/lsb_0b_ldp) : ( %6.1f +/- %4.1f ) * ( %8.1f / %10.1f ) = %8.1f +/- %6.1f\n",
             Nqcdmc_sig_ldp, Nqcdmc_sig_ldp_err, Nqcdmc_lsb_0b, Nqcdmc_lsb_0b_ldp, comp_mc_qcd_sig, comp_mc_qcd_sig_err ) ;
       printf("                                                                               MC truth value : %8.1f +/- %6.1f\n",
                   Nqcdmc_sig, Nqcdmc_sig_err ) ;


       printf("\n") ;
       printf("  ttwj : sig = ttwj_sb * ( ttwj_sig_sl / ttwj_sb_sl ) :  %6.1f * ( %6.1f / %6.1f ) = %6.1f +/- %4.1f\n",
              (Nttbarmc_sb + lsf_WJmc*NWJmc_sb), (Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl), (Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl),
              comp_mc_ttwj_sig, comp_mc_ttwj_sig_err ) ;
       printf("                                                                       MC truth value : %6.1f +/- %4.1f\n",
                (Nttbarmc_sig + lsf_WJmc*NWJmc_sig), lsf_WJmc*sqrt(NWJmc_sig) ) ;


       printf("\n\n\n") ;

       printf(" ----- Simple Data calculations\n\n") ;


       printf(" QCD :  sb  = (  sb_ldp - (ttwj+znn+other)_ldp_mc ) * (lsb_0b/lsb_0b_ldp) : ( %4d - %5.1f ) * ( %5d / %7d ) = %6.1f +/- %4.1f\n",
           Nsb_ldp, (Nttbarmc_sb_ldp  + lsf_WJmc*NWJmc_sb_ldp  + lsf_Znnmc*NZnnmc_sb_ldp  + lsf_Ewomc*NEwomc_sb_ldp  ),
           Nlsb_0b, Nlsb_0b_ldp, comp_data_qcd_sb, comp_data_qcd_sb_err ) ;
       printf("                                                                                                  MC truth value : %8.1f +/- %4.1f\n",
                   Nqcdmc_sb, Nqcdmc_sb_err ) ;

       printf("\n") ;
       printf(" QCD :  sig = ( sig_ldp - (ttwj+znn+other)_ldp_mc ) * (lsb_0b/lsb_0b_ldp) : ( %4d - %5.1f ) * ( %5d / %7d ) = %6.1f +/- %4.1f\n",
           Nsig_ldp, (Nttbarmc_sig_ldp  + lsf_WJmc*NWJmc_sig_ldp  + lsf_Znnmc*NZnnmc_sig_ldp  + lsf_Ewomc*NEwomc_sig_ldp  ),
           Nlsb_0b, Nlsb_0b_ldp, comp_data_qcd_sig, comp_data_qcd_sig_err ) ;
       printf("                                                                                                  MC truth value : %8.1f +/- %4.1f\n",
                   Nqcdmc_sig, Nqcdmc_sig_err ) ;


       printf("\n") ;
       printf(" ttwj : sig = ( sb - (qcd_sb + (znn+other)_sb_mc) ) * (sig_sl/sb_sl) : ( %5d - ( %5.1f + %5.1f))*(%3d/%3d) = %5.1f +/- %4.1f\n",
              Nsb, comp_data_qcd_sb, (lsf_Znnmc*NZnnmc_sb + lsf_Ewomc*NEwomc_sb), Nsig_sl, Nsb_sl, comp_data_ttwj_sig, comp_data_ttwj_sig_err ) ;
       printf("                                                                                              MC truth value : %5.1f +/- %4.1f\n",
                 (Nttbarmc_sig + lsf_WJmc*NWJmc_sig), lsf_WJmc*sqrt(NWJmc_sig) ) ;


       printf("\n\n\n") ;





     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




      if ( Nsig < 0 ) {
         printf("\n\n *** Negative value for Nsig in input file.  Will set Nsig to MC expectation, which is %d.\n\n",
             TMath::Nint( Nsm_sig ) ) ;
         Nsig = TMath::Nint( Nsm_sig ) ;
      }

      rv_Nsig        -> setVal( Nsig ) ;
      rv_Nsb         -> setVal( Nsb ) ;

      rv_Nsig_sl     -> setVal( Nsig_sl ) ;
      rv_Nsb_sl      -> setVal( Nsb_sl ) ;

      rv_Nsig_ldp    -> setVal( Nsig_ldp ) ;
      rv_Nsb_ldp     -> setVal( Nsb_ldp ) ;

      rv_Nlsb_0b     -> setVal( Nhtonlytrig_lsb_0b ) ;
      rv_Nlsb_0b_ldp -> setVal( Nhtonlytrig_lsb_0b_ldp ) ;

      rv_Nsb_ee      -> setVal( Nsb_ee ) ;
      rv_Nsig_ee     -> setVal( Nsig_ee ) ;

      rv_Nsb_mm      -> setVal( Nsb_mm ) ;
      rv_Nsig_mm     -> setVal( Nsig_mm ) ;







      if ( useSigTtwjVar ) {
         rrv_mu_ttwj_sig   -> setVal( Nttbarmc_sig + lsf_WJmc*NWJmc_sig ) ;  //-- this is a starting value only.
      }
      if ( !useLdpVars ) {
         rrv_mu_qcd_sig  -> setVal( Nqcdmc_sig ) ; //-- this is a starting value only.
      }



      rv_mu_znn_sig   -> setVal( lsf_Znnmc*NZnnmc_sig ) ;  //-- this is a starting value only.
      rv_mu_susy_sig    -> setVal( 0. ) ;  //-- this is a starting value only.


      if ( !useSigTtwjVar ) {
         rrv_mu_ttwj_sb   -> setVal( Nttbarmc_sb + lsf_WJmc*NWJmc_sb ) ;  //-- this is a starting value only.
      }
      if ( !useLdpVars ) {
         rrv_mu_qcd_sb  -> setVal( Nqcdmc_sb ) ; //-- this is a starting value only.
      }

      rv_mu_znn_sb   -> setVal( lsf_Znnmc*NZnnmc_sb ) ;  //-- this is a starting value only.
      rv_mu_ttwj_sig_sl  -> setVal( Nttbarmc_sig_sl + lsf_WJmc*NWJmc_sig_sl ) ;  //-- this is a starting value only.
      rv_mu_ttwj_sb_sl  -> setVal( Nttbarmc_sb_sl + lsf_WJmc*NWJmc_sb_sl ) ;  //-- this is a starting value only.


      if ( useLdpVars ) {
         rrv_mu_qcd_sig_ldp  -> setVal( Nqcdmc_sig_ldp ) ; //-- this is a starting value only.
      }

      if ( useLdpVars ) {
         rrv_mu_qcd_sb_ldp  -> setVal( Nqcdmc_sb_ldp ) ; //-- this is a starting value only.
      }

      rv_mu_qcd_lsb_0b  -> setVal( Nhtonlytrig_lsb_0b ) ;  //-- this is a starting value only.

      rv_mu_qcd_lsb_0b_ldp  -> setVal( Nhtonlytrig_lsb_0b_ldp ) ;  //-- this is a starting value only.

      rv_mu_ttbarmc_sig_ldp  -> setVal( Nttbarmc_sig_ldp ) ;
      rv_mu_WJmc_sig_ldp     -> setVal( NWJmc_sig_ldp ) ;
      rv_mu_Znnmc_sig_ldp    -> setVal( NZnnmc_sig_ldp ) ;
      rv_mu_Ewomc_sig_ldp    -> setVal( NEwomc_sig_ldp ) ;

      rv_mu_ttbarmc_sig_ldp  -> setConstant( kTRUE ) ;
      rv_mu_WJmc_sig_ldp     -> setConstant( kTRUE ) ;
      rv_mu_Znnmc_sig_ldp    -> setConstant( kTRUE ) ;
      rv_mu_Ewomc_sig_ldp    -> setConstant( kTRUE ) ;


      rv_mu_ttbarmc_sb_ldp  -> setVal( Nttbarmc_sb_ldp ) ;
      rv_mu_WJmc_sb_ldp     -> setVal( NWJmc_sb_ldp ) ;
      rv_mu_Znnmc_sb_ldp    -> setVal( NZnnmc_sb_ldp ) ;
      rv_mu_Ewomc_sb_ldp    -> setVal( NEwomc_sb_ldp ) ;

      rv_mu_ttbarmc_sb_ldp  -> setConstant( kTRUE ) ;
      rv_mu_WJmc_sb_ldp     -> setConstant( kTRUE ) ;
      rv_mu_Znnmc_sb_ldp    -> setConstant( kTRUE ) ;
      rv_mu_Ewomc_sb_ldp    -> setConstant( kTRUE ) ;


      rv_mu_Ewomc_sig    -> setVal( NEwomc_sig ) ;
      rv_mu_Ewomc_sig    -> setConstant( kTRUE ) ;

      rv_mu_Ewomc_sb    -> setVal( NEwomc_sb ) ;
      rv_mu_Ewomc_sb    -> setConstant( kTRUE ) ;



      rv_eff_sf  -> setVal( EffScaleFactor ) ;
      rv_lsf_Znnmc  -> setVal( lsf_Znnmc ) ;
      rv_lsf_WJmc  -> setVal( lsf_WJmc ) ;
      rv_lsf_Ewomc  -> setVal( lsf_Ewomc ) ;
      sf_ttbarmc = 1.0 ;
      sf_ttbarmc_err = 0.2 ;
      rv_sf_ttbarmc   -> setVal( sf_ttbarmc ) ;

      rv_acc_ee  -> setVal( acc_ee_mean ) ;
      rv_acc_mm  -> setVal( acc_mm_mean ) ;
      rv_eff_ee  -> setVal( eff_ee_mean ) ;
      rv_eff_mm  -> setVal( eff_mm_mean ) ;
      rv_znnoverll_bfratio -> setVal( 5.95 ) ;
      rv_znnoverll_bfratio -> setConstant( kTRUE ) ;
      rv_dataoverll_lumiratio  -> setVal( DataLumi / Ztoll_lumi ) ;
      rv_dataoverll_lumiratio  -> setConstant( kTRUE ) ;



       initialized = true ;

       return true ;


    } // reinitialize.

////===================================================================

//  bool ra2bRoostatsClass3::readToyDataset( const char* inputRootFile, int dsIndex ) {

//        if ( ! initialized ) {
//           printf("\n\n *** Call initialize first.\n\n") ;
//           return false ;
//        }

//       TFile intoyfile( inputRootFile, "READ" ) ;
//       gDirectory->ls() ;
//       TTree* toytree = (TTree*) gDirectory->FindObjectAny("likelihoodData") ;
//       TTree* toytruetree = (TTree*) gDirectory->FindObjectAny("toytruetree") ;

//       if ( toytree == 0 ) {
//          printf(" \n\n *** Can't find TTree likelihoodData in file %s\n", inputRootFile ) ;
//       } else {
//          toytree->Print("toponly") ;
//       }

//       if ( toytruetree == 0 ) {
//          printf(" \n\n *** Can't find TTree toytruetree in file %s\n", inputRootFile ) ;
//       } else {
//          toytruetree->Print("toponly") ;
//       }


//       Double_t toyNsig;
//       Double_t toyNa;
//       Double_t toyNd;
//       Double_t toyNsb1;
//       Double_t toyNsb2;
//       Double_t toyNsb3;
//       Double_t toyNsb4;
//       Double_t toyNsb5;
//       Double_t toyNlsb1;
//       Double_t toyNlsb2;
//       Double_t toyNlsb3;
//       Double_t toyNlsb4;
//       Double_t toyNlsb5;
//       Double_t toyNslsig1;
//       Double_t toyNslsig2;
//       Double_t toyNslsig3;
//       Double_t toyNslsig4;
//       Double_t toyNslsig5;
//       Double_t toyNslsb1;
//       Double_t toyNslsb2;
//       Double_t toyNslsb3;
//       Double_t toyNslsb4;
//       Double_t toyNslsb5;
//       Double_t toyNslmsb1;
//       Double_t toyNslmsb2;
//       Double_t toyNslmsb3;
//       Double_t toyNslmsb4;
//       Double_t toyNslmsb5;

//       Double_t toyNqcdmca ;
//       Double_t toyNqcdmcd ;
//       Double_t toyNqcdmcsig ;
//       Double_t toyNqcdmcsb ;

//       // List of branches
//       TBranch *b_toyNsig;   //!
//       TBranch *b_toyNa;   //!
//       TBranch *b_toyNd;   //!
//       TBranch *b_toyNsb1;   //!
//       TBranch *b_toyNsb2;   //!
//       TBranch *b_toyNsb3;   //!
//       TBranch *b_toyNsb4;   //!
//       TBranch *b_toyNsb5;   //!
//       TBranch *b_toyNlsb1;   //!
//       TBranch *b_toyNlsb2;   //!
//       TBranch *b_toyNlsb3;   //!
//       TBranch *b_toyNlsb4;   //!
//       TBranch *b_toyNlsb5;   //!
//       TBranch *b_toyNslsig1;   //!
//       TBranch *b_toyNslsig2;   //!
//       TBranch *b_toyNslsig3;   //!
//       TBranch *b_toyNslsig4;   //!
//       TBranch *b_toyNslsig5;   //!
//       TBranch *b_toyNslsb1;   //!
//       TBranch *b_toyNslsb2;   //!
//       TBranch *b_toyNslsb3;   //!
//       TBranch *b_toyNslsb4;   //!
//       TBranch *b_toyNslsb5;   //!
//       TBranch *b_toyNslmsb1;   //!
//       TBranch *b_toyNslmsb2;   //!
//       TBranch *b_toyNslmsb3;   //!
//       TBranch *b_toyNslmsb4;   //!
//       TBranch *b_toyNslmsb5;   //!

//       TBranch *b_toyNqcdmca ;
//       TBranch *b_toyNqcdmcd ;
//       TBranch *b_toyNqcdmcsig ;
//       TBranch *b_toyNqcdmcsb ;

//       TBranch *b_toy_mu0_ttbar_sig ;
//       TBranch *b_toy_mu0_qcd_sig ;
//       TBranch *b_toy_mu0_ttbar_sb ;
//       TBranch *b_toy_mu0_qcd_sb ;
//       TBranch *b_toy_mu0_susy_sig ;
//       TBranch *b_toy_mu0_allbg_sig ;

//       printf("\n\n Setting branch addresses...\n") ;
//       cout << " Toy tree pointer: " << toytree << endl ;
//       toytree->SetBranchAddress("Nsig", &toyNsig, &b_toyNsig ) ;
//       toytree->SetBranchAddress("Na", &toyNa, &b_toyNa);
//       toytree->SetBranchAddress("Nd", &toyNd, &b_toyNd);
//       toytree->SetBranchAddress("Nsb1", &toyNsb1, &b_toyNsb1);
//       toytree->SetBranchAddress("Nsb2", &toyNsb2, &b_toyNsb2);
//       toytree->SetBranchAddress("Nsb3", &toyNsb3, &b_toyNsb3);
//       toytree->SetBranchAddress("Nsb4", &toyNsb4, &b_toyNsb4);
//       toytree->SetBranchAddress("Nsb5", &toyNsb5, &b_toyNsb5);
//       toytree->SetBranchAddress("Nlsb1", &toyNlsb1, &b_toyNlsb1);
//       toytree->SetBranchAddress("Nlsb2", &toyNlsb2, &b_toyNlsb2);
//       toytree->SetBranchAddress("Nlsb3", &toyNlsb3, &b_toyNlsb3);
//       toytree->SetBranchAddress("Nlsb4", &toyNlsb4, &b_toyNlsb4);
//       toytree->SetBranchAddress("Nlsb5", &toyNlsb5, &b_toyNlsb5);
//       toytree->SetBranchAddress("Nslsig1", &toyNslsig1, &b_toyNslsig1);
//       toytree->SetBranchAddress("Nslsig2", &toyNslsig2, &b_toyNslsig2);
//       toytree->SetBranchAddress("Nslsig3", &toyNslsig3, &b_toyNslsig3);
//       toytree->SetBranchAddress("Nslsig4", &toyNslsig4, &b_toyNslsig4);
//       toytree->SetBranchAddress("Nslsig5", &toyNslsig5, &b_toyNslsig5);
//       toytree->SetBranchAddress("Nslsb1", &toyNslsb1, &b_toyNslsb1);
//       toytree->SetBranchAddress("Nslsb2", &toyNslsb2, &b_toyNslsb2);
//       toytree->SetBranchAddress("Nslsb3", &toyNslsb3, &b_toyNslsb3);
//       toytree->SetBranchAddress("Nslsb4", &toyNslsb4, &b_toyNslsb4);
//       toytree->SetBranchAddress("Nslsb5", &toyNslsb5, &b_toyNslsb5);
//       toytree->SetBranchAddress("Nslmsb1", &toyNslmsb1, &b_toyNslmsb1);
//       toytree->SetBranchAddress("Nslmsb2", &toyNslmsb2, &b_toyNslmsb2);
//       toytree->SetBranchAddress("Nslmsb3", &toyNslmsb3, &b_toyNslmsb3);
//       toytree->SetBranchAddress("Nslmsb4", &toyNslmsb4, &b_toyNslmsb4);
//       toytree->SetBranchAddress("Nslmsb5", &toyNslmsb5, &b_toyNslmsb5);

//       toytree->SetBranchAddress("Nqcdmca"  , &toyNqcdmca  , &b_toyNqcdmca);
//       toytree->SetBranchAddress("Nqcdmcd"  , &toyNqcdmcd  , &b_toyNqcdmcd);
//       toytree->SetBranchAddress("Nqcdmcsig", &toyNqcdmcsig, &b_toyNqcdmcsig);
//       toytree->SetBranchAddress("Nqcdmcsb" , &toyNqcdmcsb , &b_toyNqcdmcsb);

//       toytruetree->SetBranchAddress("mu0_ttbar_sig" , &toy_mu0_ttbar_sig , &b_toy_mu0_ttbar_sig);
//       toytruetree->SetBranchAddress("mu0_qcd_sig"   , &toy_mu0_qcd_sig   , &b_toy_mu0_qcd_sig);
//       toytruetree->SetBranchAddress("mu0_ttbar_sb"  , &toy_mu0_ttbar_sb  , &b_toy_mu0_ttbar_sb);
//       toytruetree->SetBranchAddress("mu0_qcd_sb"    , &toy_mu0_qcd_sb    , &b_toy_mu0_qcd_sb);
//       toytruetree->SetBranchAddress("mu0_susy_sig"  , &toy_mu0_susy_sig  , &b_toy_mu0_susy_sig);
//       toytruetree->SetBranchAddress("mu0_allbg_sig" , &toy_mu0_allbg_sig , &b_toy_mu0_allbg_sig);

//       printf("\n Done.\n\n") ;


//       toytruetree->GetEntry(0) ;

//       toytree->GetEntry( dsIndex ) ;

//       rv_Nsig -> setVal( toyNsig ) ;
//       rv_Na -> setVal( toyNa ) ;
//       rv_Nd -> setVal( toyNd ) ;
//       rv_Nsb1 -> setVal( toyNsb1 ) ;
//       rv_Nsb2 -> setVal( toyNsb2 ) ;
//       rv_Nsb3 -> setVal( toyNsb3 ) ;
//       rv_Nsb4 -> setVal( toyNsb4 ) ;
//       rv_Nsb5 -> setVal( toyNsb5 ) ;
//       rv_Nlsb1 -> setVal( toyNlsb1 ) ;
//       rv_Nlsb2 -> setVal( toyNlsb2 ) ;
//       rv_Nlsb3 -> setVal( toyNlsb3 ) ;
//       rv_Nlsb4 -> setVal( toyNlsb4 ) ;
//       rv_Nlsb5 -> setVal( toyNlsb5 ) ;
//       rv_Nslsig1 -> setVal( toyNslsig1 ) ;
//       rv_Nslsig2 -> setVal( toyNslsig2 ) ;
//       rv_Nslsig3 -> setVal( toyNslsig3 ) ;
//       rv_Nslsig4 -> setVal( toyNslsig4 ) ;
//       rv_Nslsig5 -> setVal( toyNslsig5 ) ;
//       rv_Nslsb1 -> setVal( toyNslsb1 ) ;
//       rv_Nslsb2 -> setVal( toyNslsb2 ) ;
//       rv_Nslsb3 -> setVal( toyNslsb3 ) ;
//       rv_Nslsb4 -> setVal( toyNslsb4 ) ;
//       rv_Nslsb5 -> setVal( toyNslsb5 ) ;
//       rv_Nslmsb1 -> setVal( toyNslmsb1 ) ;
//       rv_Nslmsb2 -> setVal( toyNslmsb2 ) ;
//       rv_Nslmsb3 -> setVal( toyNslmsb3 ) ;
//       rv_Nslmsb4 -> setVal( toyNslmsb4 ) ;
//       rv_Nslmsb5 -> setVal( toyNslmsb5 ) ;

//       rv_Nqcdmca   -> setVal( toyNqcdmca ) ;
//       rv_Nqcdmcd   -> setVal( toyNqcdmcd ) ;
//       rv_Nqcdmcsig -> setVal( toyNqcdmcsig ) ;
//       rv_Nqcdmcsb  -> setVal( toyNqcdmcsb ) ;


//       if ( dsObserved != 0x0 ) delete dsObserved ;

//       dsObserved = new RooDataSet("ra2b_observed_rds", "RA2b observed data values",
//                                    observedParametersList ) ;
//       dsObserved->add( observedParametersList ) ;
//       printf("\n\n") ;
//       dsObserved->printMultiline(cout, 1, kTRUE, "") ;
//       printf("\n\n") ;


//       return true ;

//  } // readToyDataset

////===================================================================



//  bool ra2bRoostatsClass3::readTextDataset( const char* infile ) {

//        if ( ! initialized ) {
//           printf("\n\n *** Call initialize first.\n\n") ;
//           return false ;
//        }


//     printf( "\n\n Opening input file : %s\n\n", infile ) ;

//     FILE* infp ;
//     if ( (infp=fopen( infile,"r"))==NULL ) {
//        printf("\n\n *** Problem opening input file: %s.\n\n", infile ) ;
//        return false ;
//     }

//     int N3jmBins(0) ; //-- number of 3-jet mass bins.

//     int Nsig(0), Na(0), Nd(0) ; //-- data counts in signal region, A, and D.
//     int Nsb1(0), Nsb2(0), Nsb3(0), Nsb4(0), Nsb5(0) ; //-- data counts in 5 3-jet mass bins of SB.
//     int Nlsb1(0), Nlsb2(0), Nlsb3(0), Nlsb4(0), Nlsb5(0) ; //-- data counts in 5 3-jet mass bins of LSB.
//     int Nslsig1(0), Nslsig2(0), Nslsig3(0), Nslsig4(0), Nslsig5(0) ; //-- data counts in 5 3-jet mass bins of SL, SIG.
//     int Nslsb1(0), Nslsb2(0), Nslsb3(0), Nslsb4(0), Nslsb5(0) ; ; //-- data counts in 5 3-jet mass bins of SL, SB.
//     int Nslmsb1(0), Nslmsb2(0), Nslmsb3(0), Nslmsb4(0), Nslmsb5(0) ; //-- data counts in 5 3-jet mass bins of SL, MSB.


//     //--- read in description line.
//     printf("\n\n") ;
//     char c(0) ;
//     while ( c!=10  ) { c = fgetc( infp ) ; printf("%c", c ) ; }
//     printf("\n\n") ;


//     char label[1000] ;

//    //--- Inputs generated with gen_roostats_input.c
//    //    The order here must be consistent with the order there!

//     fscanf( infp, "%s %d", label, &N3jmBins ) ;             printf( "%s %d\n", label, N3jmBins ) ;                
//     fscanf( infp, "%s %d", label, &Nsig ) ;                 printf( "%s %d\n", label, Nsig ) ;         
//     fscanf( infp, "%s %d", label, &Na ) ;                   printf( "%s %d\n", label, Na ) ;           
//     fscanf( infp, "%s %d", label, &Nd ) ;                   printf( "%s %d\n", label, Nd ) ;           
//     fscanf( infp, "%s %d", label, &Nsb1 ) ;                 printf( "%s %d\n", label, Nsb1 ) ;         
//     fscanf( infp, "%s %d", label, &Nsb2 ) ;                 printf( "%s %d\n", label, Nsb2 ) ;         
//     fscanf( infp, "%s %d", label, &Nsb3 ) ;                 printf( "%s %d\n", label, Nsb3 ) ;         
//     fscanf( infp, "%s %d", label, &Nsb4 ) ;                 printf( "%s %d\n", label, Nsb4 ) ;         
//     fscanf( infp, "%s %d", label, &Nsb5 ) ;                 printf( "%s %d\n", label, Nsb5 ) ;         
//     fscanf( infp, "%s %d", label, &Nlsb1 ) ;                printf( "%s %d\n", label, Nlsb1 ) ;        
//     fscanf( infp, "%s %d", label, &Nlsb2 ) ;                printf( "%s %d\n", label, Nlsb2 ) ;        
//     fscanf( infp, "%s %d", label, &Nlsb3 ) ;                printf( "%s %d\n", label, Nlsb3 ) ;        
//     fscanf( infp, "%s %d", label, &Nlsb4 ) ;                printf( "%s %d\n", label, Nlsb4 ) ;        
//     fscanf( infp, "%s %d", label, &Nlsb5 ) ;                printf( "%s %d\n", label, Nlsb5 ) ;        
//     fscanf( infp, "%s %d", label, &Nslsig1 ) ;              printf( "%s %d\n", label, Nslsig1 ) ;      
//     fscanf( infp, "%s %d", label, &Nslsig2 ) ;              printf( "%s %d\n", label, Nslsig2 ) ;      
//     fscanf( infp, "%s %d", label, &Nslsig3 ) ;              printf( "%s %d\n", label, Nslsig3 ) ;      
//     fscanf( infp, "%s %d", label, &Nslsig4 ) ;              printf( "%s %d\n", label, Nslsig4 ) ;      
//     fscanf( infp, "%s %d", label, &Nslsig5 ) ;              printf( "%s %d\n", label, Nslsig5 ) ;      
//     fscanf( infp, "%s %d", label, &Nslsb1 ) ;               printf( "%s %d\n", label, Nslsb1 ) ;       
//     fscanf( infp, "%s %d", label, &Nslsb2 ) ;               printf( "%s %d\n", label, Nslsb2 ) ;       
//     fscanf( infp, "%s %d", label, &Nslsb3 ) ;               printf( "%s %d\n", label, Nslsb3 ) ;       
//     fscanf( infp, "%s %d", label, &Nslsb4 ) ;               printf( "%s %d\n", label, Nslsb4 ) ;       
//     fscanf( infp, "%s %d", label, &Nslsb5 ) ;               printf( "%s %d\n", label, Nslsb5 ) ;       
//     fscanf( infp, "%s %d", label, &Nslmsb1 ) ;              printf( "%s %d\n", label, Nslmsb1 ) ;      
//     fscanf( infp, "%s %d", label, &Nslmsb2 ) ;              printf( "%s %d\n", label, Nslmsb2 ) ;      
//     fscanf( infp, "%s %d", label, &Nslmsb3 ) ;              printf( "%s %d\n", label, Nslmsb3 ) ;      
//     fscanf( infp, "%s %d", label, &Nslmsb4 ) ;              printf( "%s %d\n", label, Nslmsb4 ) ;      
//     fscanf( infp, "%s %d", label, &Nslmsb5 ) ;              printf( "%s %d\n", label, Nslmsb5 ) ;      

//     rv_Nsig -> setVal( Nsig ) ;
//     rv_Na -> setVal( Na ) ;
//     rv_Nd -> setVal( Nd ) ;
//     rv_Nsb1 -> setVal( Nsb1 ) ;
//     rv_Nsb2 -> setVal( Nsb2 ) ;
//     rv_Nsb3 -> setVal( Nsb3 ) ;
//     rv_Nsb4 -> setVal( Nsb4 ) ;
//     rv_Nsb5 -> setVal( Nsb5 ) ;
//     rv_Nlsb1 -> setVal( Nlsb1 ) ;
//     rv_Nlsb2 -> setVal( Nlsb2 ) ;
//     rv_Nlsb3 -> setVal( Nlsb3 ) ;
//     rv_Nlsb4 -> setVal( Nlsb4 ) ;
//     rv_Nlsb5 -> setVal( Nlsb5 ) ;
//     rv_Nslsig1 -> setVal( Nslsig1 ) ;
//     rv_Nslsig2 -> setVal( Nslsig2 ) ;
//     rv_Nslsig3 -> setVal( Nslsig3 ) ;
//     rv_Nslsig4 -> setVal( Nslsig4 ) ;
//     rv_Nslsig5 -> setVal( Nslsig5 ) ;
//     rv_Nslsb1 -> setVal( Nslsb1 ) ;
//     rv_Nslsb2 -> setVal( Nslsb2 ) ;
//     rv_Nslsb3 -> setVal( Nslsb3 ) ;
//     rv_Nslsb4 -> setVal( Nslsb4 ) ;
//     rv_Nslsb5 -> setVal( Nslsb5 ) ;
//     rv_Nslmsb1 -> setVal( Nslmsb1 ) ;
//     rv_Nslmsb2 -> setVal( Nslmsb2 ) ;
//     rv_Nslmsb3 -> setVal( Nslmsb3 ) ;
//     rv_Nslmsb4 -> setVal( Nslmsb4 ) ;
//     rv_Nslmsb5 -> setVal( Nslmsb5 ) ;


//     if ( dsObserved != 0x0 ) delete dsObserved ;

//     dsObserved = new RooDataSet("ra2b_observed_rds", "RA2b observed data values",
//                                  observedParametersList ) ;
//     dsObserved->add( observedParametersList ) ;
//     printf("\n\n") ;
//     dsObserved->printMultiline(cout, 1, kTRUE, "") ;
//     printf("\n\n") ;


//     return true ;



//  } // readTextDataset


////===================================================================


//  bool ra2bRoostatsClass3::doToyStudy( const char* inputRootFile, const char* outputRootFile, int dsFirst, int nToys ) {


//       double toyNsig ;
//       double toy_mu_ttbar_sig ;
//       double toy_mu_qcd_sig ;
//       double toy_mu_ttbar_sb ;
//       double toy_mu_qcd_sb ;

//       double toy_mu_ttbar_sig_err ;
//       double toy_mu_qcd_sig_err ;
//       double toy_mu_ttbar_sb_err ;
//       double toy_mu_qcd_sb_err ;

//       double toy_rho_ttbar_qcd_sig ;
//       double toy_rho_ttbar_qcd_sb ;

//       double toy_mu_susy_sig ;
//       double toy_mu_susy_sig_err ;
//       double toy_mu_susy_sig_ul ;

//       double toy_mu_allbg_sig ;
//       double toy_mu_allbg_sig_err ;


//       TTree* toyfittree = new TTree("toyfittree","ra2b toy fit tree") ;

//       toyfittree->Branch("Nsig"              , &toyNsig, "toyNsig/D" ) ;
//       toyfittree->Branch("mu_ttbar_sig"      , &toy_mu_ttbar_sig, "mu_ttbar_sig/D" ) ;
//       toyfittree->Branch("mu_qcd_sig"        , &toy_mu_qcd_sig, "mu_qcd_sig/D" ) ;
//       toyfittree->Branch("mu_ttbar_sb"       , &toy_mu_ttbar_sb, "mu_ttbar_sb/D" ) ;
//       toyfittree->Branch("mu_qcd_sb"         , &toy_mu_qcd_sb, "mu_qcd_sb/D" ) ;
//       toyfittree->Branch("mu_ttbar_sig_err"  , &toy_mu_ttbar_sig_err, "mu_ttbar_sig_err/D" ) ;
//       toyfittree->Branch("mu_qcd_sig_err"    , &toy_mu_qcd_sig_err, "mu_qcd_sig_err/D" ) ;
//       toyfittree->Branch("mu_ttbar_sb_err"   , &toy_mu_ttbar_sb_err, "mu_ttbar_sb_err/D" ) ;
//       toyfittree->Branch("mu_qcd_sb_err"     , &toy_mu_qcd_sb_err, "mu_qcd_sb_err/D" ) ;

//       toyfittree->Branch("rho_ttbar_qcd_sig" , &toy_rho_ttbar_qcd_sig     , "rho_ttbar_qcd_sig/D" ) ;
//       toyfittree->Branch("rho_ttbar_qcd_sb"  , &toy_rho_ttbar_qcd_sb      , "rho_ttbar_qcd_sb/D" ) ;

//       toyfittree->Branch("mu_susy_sig"       , &toy_mu_susy_sig       , "mu_susy_sig/D" ) ;
//       toyfittree->Branch("mu_susy_sig_err"   , &toy_mu_susy_sig_err   , "mu_susy_sig_err/D" ) ;
//       toyfittree->Branch("mu_susy_sig_ul"    , &toy_mu_susy_sig_ul    , "mu_susy_sig_ul/D" ) ;
//       toyfittree->Branch("mu_allbg_sig"      , &toy_mu_allbg_sig      , "mu_allbg_sig/D" ) ;
//       toyfittree->Branch("mu_allbg_sig_err"  , &toy_mu_allbg_sig_err  , "mu_allbg_sig_err/D" ) ;

//       toyfittree->Branch("mu0_ttbar_sig"      , &toy_mu0_ttbar_sig, "mu0_ttbar_sig/D" ) ;
//       toyfittree->Branch("mu0_qcd_sig"        , &toy_mu0_qcd_sig, "mu0_qcd_sig/D" ) ;
//       toyfittree->Branch("mu0_ttbar_sb"       , &toy_mu0_ttbar_sb, "mu0_ttbar_sb/D" ) ;
//       toyfittree->Branch("mu0_qcd_sb"         , &toy_mu0_qcd_sb, "mu0_qcd_sb/D" ) ;
//       toyfittree->Branch("mu0_susy_sig"       , &toy_mu0_susy_sig       , "mu0_susy_sig/D" ) ;
//       toyfittree->Branch("mu0_allbg_sig"      , &toy_mu0_allbg_sig      , "mu0_allbg_sig/D" ) ;


//       for ( int dsi=dsFirst; dsi<(dsFirst+nToys); dsi++ ) {

//          printf("\n\n\n ============ Begin fit of dataset %d ====================== \n\n", dsi ) ;

//          reinitialize() ;

//          readToyDataset( inputRootFile, dsi ) ;

//          doFit() ;

//          toyNsig = rv_Nsig->getVal() ;

//          if ( useSigTtwjVar ) {
//             toy_mu_ttbar_sig = ((RooRealVar*)rv_mu_ttbar_sig)->getVal() ;
//             toy_mu_qcd_sig = ((RooRealVar*)rv_mu_qcd_sig)->getVal() ;
//             toy_mu_ttbar_sb = ((RooFormulaVar*)rv_mu_ttbar_sb)->getVal() ;
//             toy_mu_qcd_sb = ((RooFormulaVar*)rv_mu_qcd_sb)->getVal() ;
//             toy_mu_ttbar_sig_err = ((RooRealVar*)rv_mu_ttbar_sig)->getError() ;
//             toy_mu_qcd_sig_err = ((RooRealVar*)rv_mu_qcd_sig)->getError() ;
//             toy_mu_ttbar_sb_err = 0. ;
//             toy_mu_qcd_sb_err = 0. ;
//          } else {
//             toy_mu_ttbar_sig = ((RooFormulaVar*)rv_mu_ttbar_sig)->getVal() ;
//             toy_mu_qcd_sig = ((RooFormulaVar*)rv_mu_qcd_sig)->getVal() ;
//             toy_mu_ttbar_sb = ((RooRealVar*)rv_mu_ttbar_sb)->getVal() ;
//             toy_mu_qcd_sb = ((RooRealVar*)rv_mu_qcd_sb)->getVal() ;
//             toy_mu_ttbar_sb_err = ((RooRealVar*)rv_mu_ttbar_sb)->getError() ;
//             toy_mu_qcd_sb_err = ((RooRealVar*)rv_mu_qcd_sb)->getError() ;
//             toy_mu_ttbar_sig_err = 0. ;
//             toy_mu_qcd_sig_err = 0. ;
//          }

//          toy_mu_susy_sig = rv_mu_susy_sig->getVal() ;
//          toy_mu_susy_sig_err = rv_mu_susy_sig->getError() ;

//          toy_rho_ttbar_qcd_sig = fitResult->correlation( "mu_ttbar_sig", "mu_qcd_sig" ) ;
//          toy_rho_ttbar_qcd_sb  = fitResult->correlation( "mu_ttbar_sb" , "mu_qcd_sb"  ) ;

//     //--- Profile likelihood for signal susy yield.

//          ProfileLikelihoodCalculator plc_susy_sig( *dsObserved, *likelihood, RooArgSet( *rv_mu_susy_sig ) ) ;
//          plc_susy_sig.SetTestSize(0.05) ;
//          ConfInterval* plinterval_susy_sig = plc_susy_sig.GetInterval() ;
//          float susy_sig_ul = ((LikelihoodInterval*) plinterval_susy_sig)->UpperLimit(*((RooRealVar*)rv_mu_susy_sig)) ;
//          float susy_sig_ll = ((LikelihoodInterval*) plinterval_susy_sig)->LowerLimit(*((RooRealVar*)rv_mu_susy_sig)) ;
//          printf("\n\n") ;
//          printf("    susy, SIG 95%% CL interval  [%5.1f, %5.1f]\n\n", susy_sig_ll, susy_sig_ul) ;
//          printf("\n\n") ;
//          toy_mu_susy_sig_ul = susy_sig_ul ;
//          delete plinterval_susy_sig ;


//          toy_mu_allbg_sig = toy_mu_ttbar_sig + toy_mu_qcd_sig + rv_mu_ew_sig->getVal() ;
//          { double err2 = pow(toy_mu_ttbar_sig_err,2) + pow(toy_mu_qcd_sig_err,2)
//                       + 2.*toy_rho_ttbar_qcd_sig*toy_mu_ttbar_sig_err*toy_mu_qcd_sig_err ;
//             toy_mu_allbg_sig_err =  0. ;
//             if ( err2 > 0. ) toy_mu_allbg_sig_err = sqrt( err2 ) ;
//          }

//          toyfittree->Fill() ;

//          printf("\n\n\n ============ End fit of dataset %d ====================== \n\n", dsi ) ;

//       } // dsi.

//       TFile outputfile(outputRootFile,"recreate") ;
//       toyfittree->Write() ;
//       outputfile.Close() ;

//       return true ;


//  } // doToyStudy












  //===================================================================

    bool ra2bRoostatsClass3::susyScanNoContam( const char* inputScanFile ) {


       //-- First, (re)do the fit and susy signal profile scan.

       reinitialize() ;
       doFit() ;
       float susySigLow, susySigHigh ;
       profileSusySig( susySigLow, susySigHigh, false ) ;

       printf("  Upper limit on SUSY SIG yield : %6.1f\n\n", susySigHigh ) ;




       printf("\n\n Opening SUSY scan input file : %s\n", inputScanFile ) ;

       FILE* infp ;
       if ( (infp=fopen( inputScanFile,"r"))==NULL ) {
          printf("\n\n *** Problem opening input file: %s.\n\n", inputScanFile ) ;
          return false ;
       }

       int nM0bins ;
       float minM0 ;
       float maxM0 ;
       float deltaM0 ;
       int nM12bins ;
       float minM12 ;
       float maxM12 ;
       float deltaM12 ;
       int nScanPoints ;

       char label[1000] ;

       fscanf( infp, "%s %d %f %f %f", label, &nM0bins, &minM0, &maxM0, &deltaM0 ) ;
       fscanf( infp, "%s %d %f %f %f", label, &nM12bins, &minM12, &maxM12, &deltaM12 ) ;
       fscanf( infp, "%s %d", label, &nScanPoints ) ;

       printf( "\n\n" ) ;
       printf( "  M0   :  Npoints = %4d,  min=%4.0f, max=%4.0f\n", nM0bins, minM0, maxM0 ) ;
       printf( "  M1/2 :  Npoints = %4d,  min=%4.0f, max=%4.0f\n", nM12bins, minM12, maxM12 ) ;
       printf( "\n\n" ) ;

       TH2F* hsusyscanExcluded = new TH2F("hsusyscanExcluded", "SUSY m1/2 vs m0 parameter scan",
            nM0bins, minM0-deltaM0/2., maxM0+deltaM0/2.,
            nM12bins, minM12-deltaM12/2., maxM12+deltaM12/2. ) ;


       //--- read in the column headers line.
       char c(0) ;
       c = fgetc( infp ) ;
       c = 0 ;
       while ( c!=10  ) { c = fgetc( infp ) ; }

       //--- Loop over the scan points.
       for ( int pi = 0 ; pi < nScanPoints ; pi++ ) {


          float pointM0 ;
          float pointM12 ;
          float pointXsec ;
          int    n_sig ;
          int    n_sb ;
          int    n_sig_sl ;
          int    n_sb_sl ;
          int    n_sig_ldp ;
          int    n_sb_ldp ;

          fscanf( infp, "%f %f %f   %d %d %d   %d  %d  %d",
            &pointM0, &pointM12, &pointXsec,
            &n_sig, &n_sb, &n_sig_sl,
            &n_sb_sl, &n_sig_ldp, &n_sb_ldp ) ;

          int nGenPerPoint(10000) ;
          float nselWeighted = ( n_sig * pointXsec * DataLumi ) / ( 1.0*nGenPerPoint ) ;


          int m0bin  = hsusyscanExcluded->GetXaxis()->FindBin( pointM0 ) ;
          int m12bin = hsusyscanExcluded->GetYaxis()->FindBin( pointM12 ) ;

          printf(" m0 = %4.0f (%3d),  m1/2 = %4.0f (%3d),  Npred = %7.1f", pointM0, m0bin, pointM12, m12bin, nselWeighted ) ;

          //--- Owen : Do a sanity check.
          //           If Nobs is 2 or less, give the signal UL assuming zero background.
          //           From PDG
          //           Nobs  UL(95%)
          //           0     3.00
          //           1     4.74
          //           2     6.30

          if ( nselWeighted > susySigHigh ) {
             printf(" Excluded\n") ;
             hsusyscanExcluded->SetBinContent( m0bin, m12bin, 1. ) ;
          } else {
             printf("\n") ;
          }


       } // pi .

       fclose( infp ) ;


       TStringLong infilestr( inputScanFile ) ;
       TStringLong pngoutputfilestr = infilestr ;
       pngoutputfilestr.ReplaceAll("input","output") ;
       pngoutputfilestr.ReplaceAll(".txt","-scanplot-nocontam.png") ;
       printf("\n\n png output file : %s\n\n", pngoutputfilestr.Data() ) ;

       TStringLong rootoutputfilestr = pngoutputfilestr ;
       rootoutputfilestr.ReplaceAll("png","root") ;
       printf("\n\n root output file : %s\n\n", rootoutputfilestr.Data() ) ;


       gStyle->SetPadGridX(1) ;
       gStyle->SetPadGridY(1) ;
       TCanvas* csusy = new TCanvas("csusy","SUSY m1/2 vs m0 scan") ;
       hsusyscanExcluded->Draw("col") ;
       csusy->SaveAs( pngoutputfilestr.Data() ) ;
       TFile* f = new TFile( rootoutputfilestr.Data() ,"recreate") ;
       hsusyscanExcluded->Write() ;
       f->Write() ;
       f->Close() ;

       return true ;

    } // susyScanNoContam.


  //===================================================================

    bool ra2bRoostatsClass3::susyScanWithContam( const char* inputScanFile ) {


       printf("\n\n Opening SUSY scan input file : %s\n", inputScanFile ) ;

       FILE* infp ;
       if ( (infp=fopen( inputScanFile,"r"))==NULL ) {
          printf("\n\n *** Problem opening input file: %s.\n\n", inputScanFile ) ;
          return false ;
       }

       int nM0bins ;
       float minM0 ;
       float maxM0 ;
       float deltaM0 ;
       int nM12bins ;
       float minM12 ;
       float maxM12 ;
       float deltaM12 ;
       int nScanPoints ;

       char label[1000] ;

       fscanf( infp, "%s %d %f %f %f", label, &nM0bins, &minM0, &maxM0, &deltaM0 ) ;
       fscanf( infp, "%s %d %f %f %f", label, &nM12bins, &minM12, &maxM12, &deltaM12 ) ;
       fscanf( infp, "%s %d", label, &nScanPoints ) ;

       printf( "\n\n" ) ;
       printf( "  M0   :  Npoints = %4d,  min=%4.0f, max=%4.0f\n", nM0bins, minM0, maxM0 ) ;
       printf( "  M1/2 :  Npoints = %4d,  min=%4.0f, max=%4.0f\n", nM12bins, minM12, maxM12 ) ;
       printf( "\n\n" ) ;

       TH2F* hsusyscanExcluded = new TH2F("hsusyscanExcluded", "SUSY m1/2 vs m0 parameter scan",
            nM0bins, minM0-deltaM0/2., maxM0+deltaM0/2.,
            nM12bins, minM12-deltaM12/2., maxM12+deltaM12/2. ) ;

       TH2F* hsusyscanNsigul = new TH2F("hsusyscanNsigul", "SUSY m1/2 vs m0 parameter scan, upper limit on Nsig",
            nM0bins, minM0-deltaM0/2., maxM0+deltaM0/2.,
            nM12bins, minM12-deltaM12/2., maxM12+deltaM12/2. ) ;

       TH2F* hsusyscanNsigpred = new TH2F("hsusyscanNsigpred", "SUSY m1/2 vs m0 parameter scan, predicted Nsig",
            nM0bins, minM0-deltaM0/2., maxM0+deltaM0/2.,
            nM12bins, minM12-deltaM12/2., maxM12+deltaM12/2. ) ;


       //--- read in the column headers line.
       char c(0) ;
       c = fgetc( infp ) ;
       c = 0 ;
       while ( c!=10  ) { c = fgetc( infp ) ; }

       //--- Loop over the scan points.
       for ( int pi = 0 ; pi < nScanPoints ; pi++ ) {

          float pointM0 ;
          float pointM12 ;
          float pointXsec ;
          int    n_sig ;
          int    n_sb ;
          int    n_sig_sl ;
          int    n_sb_sl ;
          int    n_sig_ldp ;
          int    n_sb_ldp ;

          fscanf( infp, "%f %f %f   %d %d %d   %d  %d  %d",
            &pointM0, &pointM12, &pointXsec,
            &n_sig, &n_sb, &n_sig_sl,
            &n_sb_sl, &n_sig_ldp, &n_sb_ldp ) ;



          int nGenPerPoint(10000) ;


          int m0bin  = hsusyscanExcluded->GetXaxis()->FindBin( pointM0 ) ;
          int m12bin = hsusyscanExcluded->GetYaxis()->FindBin( pointM12 ) ;


       //--- Set up the likelihood to include the SUSY contributions to the non-SIG regions.

          float weight = ( pointXsec * DataLumi ) / ( 1.0*nGenPerPoint ) ;

          reinitialize() ;

          rv_mu_susymc_sig     -> setVal( n_sig * weight ) ;
          rv_mu_susymc_sb      -> setVal( n_sb * weight ) ;
          rv_mu_susymc_sig_sl  -> setVal( n_sig_sl * weight ) ;
          rv_mu_susymc_sb_sl   -> setVal( n_sb_sl * weight ) ;
          rv_mu_susymc_sig_ldp -> setVal( n_sig_ldp * weight ) ;
          rv_mu_susymc_sb_ldp  -> setVal( n_sb_ldp * weight ) ;

          parameterSnapshot() ;

          doFit() ;

          parameterSnapshot() ;

          float susySigLow, susySigHigh ;
          profileSusySig( susySigLow, susySigHigh, false ) ;


          float nselWeighted =  n_sig * weight ;


          printf("  Upper limit on SUSY SIG yield : %6.1f\n\n", susySigHigh ) ;
          printf(" m0 = %4.0f (%d),  m1/2 = %4.0f (%d),  Npred = %7.1f", pointM0, m0bin, pointM12, m12bin, nselWeighted ) ;

          hsusyscanNsigul->SetBinContent( m0bin, m12bin, susySigHigh ) ;
          hsusyscanNsigpred->SetBinContent( m0bin, m12bin, nselWeighted ) ;

          if ( nselWeighted > susySigHigh ) {
             printf(" Excluded\n") ;
             hsusyscanExcluded->SetBinContent( m0bin, m12bin, 1. ) ;
          } else {
             printf("\n") ;
          }


       } // pi .

       fclose( infp ) ;




       TStringLong infilestr( inputScanFile ) ;
       TStringLong pngoutputfilestr = infilestr ;
       pngoutputfilestr.ReplaceAll("input","output") ;
       pngoutputfilestr.ReplaceAll(".txt","-scanplot-withcontam.png") ;
       printf("\n\n png output file : %s\n\n", pngoutputfilestr.Data() ) ;

       TStringLong rootoutputfilestr = pngoutputfilestr ;
       rootoutputfilestr.ReplaceAll("png","root") ;
       printf("\n\n root output file : %s\n\n", rootoutputfilestr.Data() ) ;


       TCanvas* csusy = new TCanvas("csusy","SUSY m1/2 vs m0 scan") ;
       gStyle->SetPadGridX(1) ;
       gStyle->SetPadGridY(1) ;
       hsusyscanExcluded->Draw("col") ;
       csusy->SaveAs( pngoutputfilestr.Data() ) ;
       TFile* f = new TFile( rootoutputfilestr.Data(),"recreate") ;
       hsusyscanExcluded->Write() ;
       hsusyscanNsigul->Write() ;
       hsusyscanNsigpred->Write() ;
       f->Write() ;
       f->Close() ;

       return true ;

    } // susyScanWithContam.

  //===================================================================

    bool ra2bRoostatsClass3::setSusyScanPoint( const char* inputScanFile, double m0, double m12 ) {



       printf("\n\n Opening SUSY scan input file : %s\n", inputScanFile ) ;

       FILE* infp ;
       if ( (infp=fopen( inputScanFile,"r"))==NULL ) {
          printf("\n\n *** Problem opening input file: %s.\n\n", inputScanFile ) ;
          return false ;
       }

       int nM0bins ;
       float minM0 ;
       float maxM0 ;
       float deltaM0 ;
       int nM12bins ;
       float minM12 ;
       float maxM12 ;
       float deltaM12 ;
       int nScanPoints ;

       char label[1000] ;

       fscanf( infp, "%s %d %f %f %f", label, &nM0bins, &minM0, &maxM0, &deltaM0 ) ;
       fscanf( infp, "%s %d %f %f %f", label, &nM12bins, &minM12, &maxM12, &deltaM12 ) ;
       fscanf( infp, "%s %d", label, &nScanPoints ) ;

       printf( "\n\n" ) ;
       printf( "  M0   :  Npoints = %4d,  min=%4.0f, max=%4.0f\n", nM0bins, minM0, maxM0 ) ;
       printf( "  M1/2 :  Npoints = %4d,  min=%4.0f, max=%4.0f\n", nM12bins, minM12, maxM12 ) ;
       printf( "\n\n" ) ;


       //--- read in the column headers line.
       char c(0) ;
       c = fgetc( infp ) ;
       c = 0 ;
       while ( c!=10  ) { c = fgetc( infp ) ; }

       bool found(false) ;

       //--- Loop over the scan points.
       for ( int pi = 0 ; pi < nScanPoints ; pi++ ) {

          float pointM0 ;
          float pointM12 ;
          float pointXsec ;
          int    n_sig ;
          int    n_sb ;
          int    n_sig_sl ;
          int    n_sb_sl ;
          int    n_sig_ldp ;
          int    n_sb_ldp ;

          fscanf( infp, "%f %f %f   %d %d %d   %d  %d  %d",
            &pointM0, &pointM12, &pointXsec,
            &n_sig, &n_sb, &n_sig_sl,
            &n_sb_sl, &n_sig_ldp, &n_sb_ldp ) ;


          if (    fabs( pointM0 - m0 ) <= deltaM0/2.
               && fabs( pointM12 - m12 ) <= deltaM12/2. ) {

             int nGenPerPoint(10000) ;
             float nselWeighted = ( n_sig * pointXsec * DataLumi ) / ( 1.0*nGenPerPoint ) ;


             printf("\n\n Found point m0 = %4.0f,  m1/2 = %4.0f,  Npred = %7.1f\n\n\n", pointM0, pointM12, nselWeighted ) ;

          //--- Set up the likelihood to include the SUSY contributions to the non-SIG regions.

             float weight = ( pointXsec * DataLumi ) / ( 1.0*nGenPerPoint ) ;

             rv_mu_susymc_sig     -> setVal( n_sig * weight ) ;
             rv_mu_susymc_sb      -> setVal( n_sb * weight ) ;
             rv_mu_susymc_sig_sl  -> setVal( n_sig_sl * weight ) ;
             rv_mu_susymc_sb_sl   -> setVal( n_sb_sl * weight ) ;
             rv_mu_susymc_sig_ldp -> setVal( n_sig_ldp * weight ) ;
             rv_mu_susymc_sb_ldp  -> setVal( n_sb_ldp * weight ) ;

             found = true ;

             break ;

          } // point match?

       } // pi .

       fclose( infp ) ;

       if ( found ) {
          return true ;
       } else {
          printf("\n\n *** Point not found in scan.\n\n" ) ;
          return false ;
       }

    } // setSusyScanPoint.


  //===================================================================

  //===================================================================

    bool ra2bRoostatsClass3::fitQualityPlot( bool doNorm, double hmax ) {

     //  Shows all of the Likelihood inputs, normalized to the data values,
     //  compared to the fit result.

     //
     //
     //            S
     //            L S
     //      S     S L                   E
     //      I S   I S                   S
     //   | |G B| |G B| |D A| | QCDMC | |F| |
     //    x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x
     //    1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
     //                      1                   2                   3
     //

      int nbins(10) ;

      TH1F* hfitqual_data = new TH1F("hfitqual_data","RA2b likelihood fit results, data",
                            nbins, 0.5, nbins+0.5 ) ;
      TH1F* hfitqual_ttwj = new TH1F("hfitqual_ttwj","RA2b likelihood fit results, ttbar",
                            nbins, 0.5, nbins+0.5 ) ;
      TH1F* hfitqual_qcd = new TH1F("hfitqual_qcd","RA2b likelihood fit results, QCD",
                            nbins, 0.5, nbins+0.5 ) ;
      TH1F* hfitqual_znn = new TH1F("hfitqual_znn","RA2b likelihood fit results, Ztonunu",
                            nbins, 0.5, nbins+0.5 ) ;
      TH1F* hfitqual_ewo = new TH1F("hfitqual_ewo","RA2b likelihood fit results, EW-other",
                            nbins, 0.5, nbins+0.5 ) ;
      TH1F* hfitqual_susy = new TH1F("hfitqual_susy","RA2b likelihood fit results, SUSY",
                            nbins, 0.5, nbins+0.5 ) ;
      TH1F* hfitqual_gaus = new TH1F("hfitqual_gaus","RA2b likelihood fit results, Gaussian constraints",
                            nbins, 0.5, nbins+0.5 ) ;



      hfitqual_ttwj  -> SetFillColor(kBlue-9) ;
      hfitqual_qcd   -> SetFillColor(2) ;
      hfitqual_znn   -> SetFillColor(kGreen-3) ;
      hfitqual_ewo   -> SetFillColor(kGreen-5) ;
      hfitqual_susy  -> SetFillColor(6) ;
      hfitqual_gaus  -> SetFillColor(kOrange+1) ;

      THStack* hfitqual_fit = new THStack( "hfitqual_fit", "RA2b likelihood fit results, fit" ) ;

      hfitqual_data->SetMarkerStyle(20) ;
      hfitqual_data->SetLineWidth(2) ;

      TAxis* xaxis = hfitqual_data->GetXaxis() ;

      int binIndex ;

      binIndex = 2 ;

      double dataVal ;
      double dataErr ;
      double dataErrNorm ;

      double ttwjVal ;
      double qcdVal ;
      double znnVal ;
      double ewoVal ;
      double susyVal ;

      double ttwjValNorm ;
      double qcdValNorm ;
      double znnValNorm ;
      double ewoValNorm ;
      double susyValNorm ;

      double eff_sf ;

      char   binLabel[1000] ;



      eff_sf = rv_eff_sf->getVal() ;


     //-- SIG -------------------------------------------------------

      sprintf( binLabel, "SIG" ) ;
      xaxis->SetBinLabel(binIndex, binLabel ) ;

      dataVal  = rv_Nsig->getVal() ;
      if ( useSigTtwjVar ) {
         ttwjVal = rrv_mu_ttwj_sig->getVal() ;
      } else {
         ttwjVal = rfv_mu_ttwj_sig->getVal() ;
      }
      if ( useLdpVars ) {
         qcdVal   = rfv_mu_qcd_sig->getVal() ;
      } else {
         qcdVal   = rrv_mu_qcd_sig->getVal() ;
      }
      znnVal   = (rv_mu_znn_sig->getVal()) ;
      ewoVal   = (rv_mu_ewo_sig->getVal()) * (rv_lsf_Ewomc->getVal()) ;
      susyVal  = rv_mu_susy_sig->getVal() ;
      ewoVal   = eff_sf * ewoVal ;
      susyVal = eff_sf * susyVal ;


      dataErr = sqrt(dataVal) ;

      dataErrNorm = dataErr ;
      ttwjValNorm = ttwjVal ;
      qcdValNorm = qcdVal ;
      znnValNorm = znnVal ;
      ewoValNorm = ewoVal ;
      susyValNorm = susyVal ;
      hfitqual_data->SetBinContent( binIndex, dataVal ) ;
      if ( dataVal > 0. && doNorm ) { 
         hfitqual_data->SetBinContent( binIndex, 1. ) ;
         dataErrNorm  = dataErr / dataVal ; 
         ttwjValNorm = ttwjVal / dataVal ;
         qcdValNorm   = qcdVal / dataVal ;
         znnValNorm    = znnVal / dataVal ;
         ewoValNorm    = ewoVal / dataVal ;
         susyValNorm  = susyVal / dataVal ;
      }


      hfitqual_data->SetBinError( binIndex, dataErrNorm ) ;

      hfitqual_ttwj -> SetBinContent( binIndex, ttwjValNorm ) ;
      hfitqual_qcd   -> SetBinContent( binIndex, qcdValNorm ) ;
      hfitqual_znn    -> SetBinContent( binIndex, znnValNorm ) ;
      hfitqual_ewo    -> SetBinContent( binIndex, ewoValNorm ) ;
      hfitqual_susy  -> SetBinContent( binIndex, susyValNorm ) ;


      binIndex++ ;





     //-- SB -------------------------------------------------------

      sprintf( binLabel, "SB" ) ;
      xaxis->SetBinLabel(binIndex, binLabel ) ;

      dataVal  = rv_Nsb->getVal() ;
      if ( useSigTtwjVar ) {
         ttwjVal = rfv_mu_ttwj_sb->getVal() ;
      } else {
         ttwjVal = rrv_mu_ttwj_sb->getVal() ;
      }
      if ( useLdpVars ) {
         qcdVal   = rfv_mu_qcd_sb->getVal() ;
      } else {
         qcdVal   = rrv_mu_qcd_sb->getVal() ;
      }
      znnVal   = (rv_mu_znn_sb->getVal()) ;
      ewoVal   = (rv_mu_ewo_sb->getVal()) * (rv_lsf_Ewomc->getVal()) ;
      susyVal  = rv_mu_susy_sb->getVal() ;
      ewoVal   = eff_sf * ewoVal ;
      susyVal = eff_sf * susyVal ;


      dataErr = sqrt(dataVal) ;

      dataErrNorm = dataErr ;
      ttwjValNorm = ttwjVal ;
      qcdValNorm = qcdVal ;
      znnValNorm = znnVal ;
      ewoValNorm = ewoVal ;
      susyValNorm = susyVal ;
      hfitqual_data->SetBinContent( binIndex, dataVal ) ;
      if ( dataVal > 0. && doNorm ) { 
         hfitqual_data->SetBinContent( binIndex, 1. ) ;
         dataErrNorm  = dataErr / dataVal ; 
         ttwjValNorm = ttwjVal / dataVal ;
         qcdValNorm   = qcdVal / dataVal ;
         znnValNorm    = znnVal / dataVal ;
         ewoValNorm    = ewoVal / dataVal ;
         susyValNorm  = susyVal / dataVal ;
      }


      hfitqual_data->SetBinError( binIndex, dataErrNorm ) ;

      hfitqual_ttwj -> SetBinContent( binIndex, ttwjValNorm ) ;
      hfitqual_qcd   -> SetBinContent( binIndex, qcdValNorm ) ;
      hfitqual_znn    -> SetBinContent( binIndex, znnValNorm ) ;
      hfitqual_ewo    -> SetBinContent( binIndex, ewoValNorm ) ;
      hfitqual_susy  -> SetBinContent( binIndex, susyValNorm ) ;


      binIndex++ ;





     //-- SIG,SL -------------------------------------------------------

      sprintf( binLabel, "SIG,SL" ) ;
      xaxis->SetBinLabel(binIndex, binLabel ) ;

      dataVal  = rv_Nsig_sl->getVal() ;
      ttwjVal = rv_mu_ttwj_sig_sl->getVal() ;
      qcdVal   = 0. ;
      znnVal   = 0. ;
      ewoVal   = 0. ;
      susyVal  = rv_mu_susy_sig_sl->getVal() ;
      susyVal = eff_sf * susyVal ;


      dataErr = sqrt(dataVal) ;

      dataErrNorm = dataErr ;
      ttwjValNorm = ttwjVal ;
      qcdValNorm = qcdVal ;
      znnValNorm = znnVal ;
      ewoValNorm = ewoVal ;
      susyValNorm = susyVal ;
      hfitqual_data->SetBinContent( binIndex, dataVal ) ;
      if ( dataVal > 0. && doNorm ) { 
         hfitqual_data->SetBinContent( binIndex, 1. ) ;
         dataErrNorm  = dataErr / dataVal ; 
         ttwjValNorm = ttwjVal / dataVal ;
         qcdValNorm   = qcdVal / dataVal ;
         znnValNorm    = znnVal / dataVal ;
         ewoValNorm    = ewoVal / dataVal ;
         susyValNorm  = susyVal / dataVal ;
      }


      hfitqual_data->SetBinError( binIndex, dataErrNorm ) ;

      hfitqual_ttwj -> SetBinContent( binIndex, ttwjValNorm ) ;
      hfitqual_qcd   -> SetBinContent( binIndex, qcdValNorm ) ;
      hfitqual_znn    -> SetBinContent( binIndex, znnValNorm ) ;
      hfitqual_ewo    -> SetBinContent( binIndex, ewoValNorm ) ;
      hfitqual_susy  -> SetBinContent( binIndex, susyValNorm ) ;


      binIndex++ ;





     //-- SB,SL -------------------------------------------------------

      sprintf( binLabel, "SB,SL" ) ;
      xaxis->SetBinLabel(binIndex, binLabel ) ;

      dataVal  = rv_Nsb_sl->getVal() ;
      ttwjVal = rv_mu_ttwj_sb_sl->getVal() ;
      qcdVal   = 0. ;
      znnVal   = 0. ;
      ewoVal   = 0. ;
      susyVal  = rv_mu_susy_sb_sl->getVal() ;
      susyVal = eff_sf * susyVal ;


      dataErr = sqrt(dataVal) ;

      dataErrNorm = dataErr ;
      ttwjValNorm = ttwjVal ;
      qcdValNorm = qcdVal ;
      znnValNorm = znnVal ;
      ewoValNorm = ewoVal ;
      susyValNorm = susyVal ;
      hfitqual_data->SetBinContent( binIndex, dataVal ) ;
      if ( dataVal > 0. && doNorm ) { 
         hfitqual_data->SetBinContent( binIndex, 1. ) ;
         dataErrNorm  = dataErr / dataVal ; 
         ttwjValNorm = ttwjVal / dataVal ;
         qcdValNorm   = qcdVal / dataVal ;
         znnValNorm    = znnVal / dataVal ;
         ewoValNorm    = ewoVal / dataVal ;
         susyValNorm  = susyVal / dataVal ;
      }


      hfitqual_data->SetBinError( binIndex, dataErrNorm ) ;

      hfitqual_ttwj -> SetBinContent( binIndex, ttwjValNorm ) ;
      hfitqual_qcd   -> SetBinContent( binIndex, qcdValNorm ) ;
      hfitqual_znn    -> SetBinContent( binIndex, znnValNorm ) ;
      hfitqual_ewo    -> SetBinContent( binIndex, ewoValNorm ) ;
      hfitqual_susy  -> SetBinContent( binIndex, susyValNorm ) ;


      binIndex++ ;





     //-- SIG,LDP -------------------------------------------------------

      sprintf( binLabel, "SIG,LDP" ) ;
      xaxis->SetBinLabel(binIndex, binLabel ) ;

      dataVal  = rv_Nsig_ldp->getVal() ;
      ttwjVal = rv_mu_ttwj_sig_ldp->getVal() ;
      if ( useLdpVars ) {
         qcdVal   = rrv_mu_qcd_sig_ldp->getVal() ;
      } else {
         qcdVal   = rfv_mu_qcd_sig_ldp->getVal() ;
      }
      znnVal   = rv_mu_znn_sig_ldp->getVal() ;
      ewoVal   = (rv_mu_ewo_sig_ldp->getVal()) * (rv_lsf_Ewomc->getVal()) ;
      susyVal  = rv_mu_susy_sig_ldp->getVal() ;
      ttwjVal  = eff_sf * ttwjVal ;
      znnVal   = eff_sf * znnVal ;
      ewoVal   = eff_sf * ewoVal ;
      susyVal = eff_sf * susyVal ;


      dataErr = sqrt(dataVal) ;

      dataErrNorm = dataErr ;
      ttwjValNorm = ttwjVal ;
      qcdValNorm = qcdVal ;
      znnValNorm = znnVal ;
      ewoValNorm = ewoVal ;
      susyValNorm = susyVal ;
      hfitqual_data->SetBinContent( binIndex, dataVal ) ;
      if ( dataVal > 0. && doNorm ) { 
         hfitqual_data->SetBinContent( binIndex, 1. ) ;
         dataErrNorm  = dataErr / dataVal ; 
         ttwjValNorm = ttwjVal / dataVal ;
         qcdValNorm   = qcdVal / dataVal ;
         znnValNorm    = znnVal / dataVal ;
         ewoValNorm    = ewoVal / dataVal ;
         susyValNorm  = susyVal / dataVal ;
      }


      hfitqual_data->SetBinError( binIndex, dataErrNorm ) ;

      hfitqual_ttwj -> SetBinContent( binIndex, ttwjValNorm ) ;
      hfitqual_qcd   -> SetBinContent( binIndex, qcdValNorm ) ;
      hfitqual_znn    -> SetBinContent( binIndex, znnValNorm ) ;
      hfitqual_ewo    -> SetBinContent( binIndex, ewoValNorm ) ;
      hfitqual_susy  -> SetBinContent( binIndex, susyValNorm ) ;


      binIndex++ ;





     //-- SB,LDP -------------------------------------------------------

      sprintf( binLabel, "SB,LDP" ) ;
      xaxis->SetBinLabel(binIndex, binLabel ) ;

      dataVal  = rv_Nsb_ldp->getVal() ;
      ttwjVal = rv_mu_ttwj_sb_ldp->getVal() ;
      if ( useLdpVars ) {
         qcdVal   = rrv_mu_qcd_sb_ldp->getVal() ;
      } else {
         qcdVal   = rfv_mu_qcd_sb_ldp->getVal() ;
      }
      znnVal   = rv_mu_znn_sb_ldp->getVal() ;
      ewoVal   = (rv_mu_ewo_sb_ldp->getVal()) * (rv_lsf_Ewomc->getVal()) ;
      susyVal  = rv_mu_susy_sb_ldp->getVal() ;
      ttwjVal  = eff_sf * ttwjVal ;
      znnVal   = eff_sf * znnVal ;
      ewoVal   = eff_sf * ewoVal ;
      susyVal = eff_sf * susyVal ;


      dataErr = sqrt(dataVal) ;

      dataErrNorm = dataErr ;
      ttwjValNorm = ttwjVal ;
      qcdValNorm = qcdVal ;
      znnValNorm = znnVal ;
      ewoValNorm = ewoVal ;
      susyValNorm = susyVal ;
      hfitqual_data->SetBinContent( binIndex, dataVal ) ;
      if ( dataVal > 0. && doNorm ) { 
         hfitqual_data->SetBinContent( binIndex, 1. ) ;
         dataErrNorm  = dataErr / dataVal ; 
         ttwjValNorm = ttwjVal / dataVal ;
         qcdValNorm   = qcdVal / dataVal ;
         znnValNorm    = znnVal / dataVal ;
         ewoValNorm    = ewoVal / dataVal ;
         susyValNorm  = susyVal / dataVal ;
      }


      hfitqual_data->SetBinError( binIndex, dataErrNorm ) ;

      hfitqual_ttwj -> SetBinContent( binIndex, ttwjValNorm ) ;
      hfitqual_qcd   -> SetBinContent( binIndex, qcdValNorm ) ;
      hfitqual_znn    -> SetBinContent( binIndex, znnValNorm ) ;
      hfitqual_ewo    -> SetBinContent( binIndex, ewoValNorm ) ;
      hfitqual_susy  -> SetBinContent( binIndex, susyValNorm ) ;


      binIndex++ ;

















//    binIndex++ ;

//  //-- lsf, W+jets

//    sprintf( binLabel, "LSF, W+jets" ) ;
//    xaxis->SetBinLabel(binIndex, binLabel ) ;

//    dataVal = lsf_WJmc ;
//    mcVal   = rv_lsf_wjmc->getVal() ;

//    dataErr = lsf_WJmc_err ;

//    dataErrNorm = dataErr ;
//    mcValNorm = mcVal ;

//    hfitqual_data->SetBinContent( binIndex, dataVal ) ;
//    if ( dataVal > 0. && doNorm ) { 
//       hfitqual_data->SetBinContent( binIndex, 1. ) ;
//       dataErrNorm  = dataErr / dataVal ; 
//       mcValNorm = mcVal / dataVal ;
//    }
//    if ( !doNorm ) {
//       hfitqual_data->SetBinContent( binIndex, dataVal*0.1 ) ;
//       dataErrNorm = dataErrNorm*0.1 ;
//       mcValNorm = mcValNorm*0.1 ;
//    }


//    hfitqual_data->SetBinError( binIndex, dataErrNorm ) ;

//    hfitqual_gaus -> SetBinContent( binIndex, mcValNorm ) ;

//    printf(" %10s : err=%4.2f ;   MC = %4.2f\n", binLabel, dataErrNorm, mcValNorm ) ;

//    binIndex++ ;




//  //-- lsf, Z to nunu

//    sprintf( binLabel, "LSF, Z invis" ) ;
//    xaxis->SetBinLabel(binIndex, binLabel ) ;

//    dataVal = lsf_Znnmc ;
//    mcVal   = rv_lsf_znnmc->getVal() ;

//    dataErr = lsf_Znnmc_err ;

//    dataErrNorm = dataErr ;
//    mcValNorm = mcVal ;

//    hfitqual_data->SetBinContent( binIndex, dataVal ) ;
//    if ( dataVal > 0. && doNorm ) { 
//       hfitqual_data->SetBinContent( binIndex, 1. ) ;
//       dataErrNorm  = dataErr / dataVal ; 
//       mcValNorm = mcVal / dataVal ;
//    }
//    if ( !doNorm ) {
//       hfitqual_data->SetBinContent( binIndex, dataVal*0.1 ) ;
//       dataErrNorm = dataErrNorm*0.1 ;
//       mcValNorm = mcValNorm*0.1 ;
//    }


//    hfitqual_data->SetBinError( binIndex, dataErrNorm ) ;

//    hfitqual_gaus -> SetBinContent( binIndex, mcValNorm ) ;

//    printf(" %10s : err=%4.2f ;   MC = %4.2f\n", binLabel, dataErrNorm, mcValNorm ) ;

//    binIndex++ ;




//  //-- lsf, EW other

//    sprintf( binLabel, "LSF, W+jets" ) ;
//    xaxis->SetBinLabel(binIndex, binLabel ) ;

//    dataVal = lsf_WJmc ;
//    mcVal   = rv_lsf_wjmc->getVal() ;

//    dataErr = lsf_WJmc_err ;

//    dataErrNorm = dataErr ;
//    mcValNorm = mcVal ;

//    hfitqual_data->SetBinContent( binIndex, dataVal ) ;
//    if ( dataVal > 0. && doNorm ) { 
//       hfitqual_data->SetBinContent( binIndex, 1. ) ;
//       dataErrNorm  = dataErr / dataVal ; 
//       mcValNorm = mcVal / dataVal ;
//    }
//    if ( !doNorm ) {
//       hfitqual_data->SetBinContent( binIndex, dataVal*0.1 ) ;
//       dataErrNorm = dataErrNorm*0.1 ;
//       mcValNorm = mcValNorm*0.1 ;
//    }


//    hfitqual_data->SetBinError( binIndex, dataErrNorm ) ;

//    hfitqual_gaus -> SetBinContent( binIndex, mcValNorm ) ;

//    printf(" %10s : err=%4.2f ;   MC = %4.2f\n", binLabel, dataErrNorm, mcValNorm ) ;

//    binIndex++ ;










      binIndex++ ;

    //-- Eff SF

      double effsfScale(1.5) ;

      printf(" hist max: %g\n", hfitqual_data->GetMaximum()) ;
      double effsfsf = hmax*(hfitqual_data->GetMaximum())/(effsfScale)  ;

      sprintf( binLabel, "Eff SF (right scale)" ) ;
      xaxis->SetBinLabel(binIndex, binLabel ) ;

      dataVal = EffScaleFactor ;
      float mcVal   = rv_eff_sf->getVal() ;

      dataErr = EffScaleFactorErr ;

      dataErrNorm = dataErr ;
      float mcValNorm = mcVal ;


      hfitqual_data->SetBinContent( binIndex, effsfsf*dataVal ) ;
      if ( dataVal > 0. && doNorm ) { 
         hfitqual_data->SetBinContent( binIndex, 1. ) ;
         dataErrNorm  = dataErr / dataVal ; 
         mcValNorm = mcVal / dataVal ;
      }


      hfitqual_data->SetBinError( binIndex, effsfsf*dataErrNorm ) ;

      hfitqual_gaus -> SetBinContent( binIndex, effsfsf*mcValNorm ) ;

      printf(" %10s : err=%4.2f ;   MC = %4.2f\n", binLabel, dataErrNorm, mcValNorm ) ;

      binIndex++ ;




     //-- final formatting

      hfitqual_fit->Add( hfitqual_znn ) ;
      hfitqual_fit->Add( hfitqual_qcd ) ;
      hfitqual_fit->Add( hfitqual_ttwj ) ;
      hfitqual_fit->Add( hfitqual_susy ) ;
      hfitqual_fit->Add( hfitqual_gaus ) ;

      TLegend* legend = new TLegend(0.82,0.3,0.98,0.9) ;

      legend->AddEntry( hfitqual_data,  "data" ) ;
      legend->AddEntry( hfitqual_susy,  "SUSY" ) ;
      legend->AddEntry( hfitqual_ttwj, "ttwj" ) ;
      legend->AddEntry( hfitqual_qcd,   "QCD" ) ;
      legend->AddEntry( hfitqual_znn,   "Znunu" ) ;
      legend->AddEntry( hfitqual_gaus,  "Gaus.C." ) ;

      if ( doNorm ) {
         hfitqual_data->SetMaximum( hmax ) ;
      } else {
         hfitqual_data->SetMaximum( hmax*(hfitqual_data->GetMaximum()) ) ;
      }


      double oldBM = gStyle->GetPadBottomMargin() ;
      double oldRM = gStyle->GetPadRightMargin() ;


      gStyle->SetPadBottomMargin(0.3) ;
      gStyle->SetPadRightMargin(0.27) ;


      TCanvas* cfitqual = new TCanvas("cfitqual","RA2b fit quality", 600, 550 ) ;

      gPad->SetTicks(1,0) ;


      hfitqual_data->SetLabelSize(0.05,"x") ;
      hfitqual_data->GetXaxis()->LabelsOption("v") ;

      hfitqual_data->Draw() ;
      hfitqual_fit->Draw("same") ;
      hfitqual_data->Draw("same") ;
      legend->Draw() ;



      TGaxis* axis = new TGaxis() ;
      axis->DrawAxis( nbins+0.5, 0, nbins+0.5, hfitqual_data->GetMaximum(), 0., effsfScale, 510, "+L") ;

      TLine* line = new TLine() ;
      line->SetLineWidth(2) ;

      line->DrawLine( nbins-2., 0., nbins-2., hfitqual_data->GetMaximum() ) ;


      cfitqual->SaveAs("fitqual.png") ;


      gStyle->SetPadBottomMargin(oldBM) ;
      gStyle->SetPadRightMargin(oldRM) ;

      return true ;

    } // fitQualityPlot

  //===================================================================


    void ra2bRoostatsClass3::setAndFixSusySig( double setVal ) {

       rv_mu_susy_sig->setVal( setVal ) ;
       rv_mu_susy_sig->setConstant( kTRUE ) ;

       printf("\n\n Set SUSY SIG yield to %.1f and fixed it.\n\n\n", setVal ) ;

    }

  //===================================================================


    void ra2bRoostatsClass3::freeSusySig( ) {

       rv_mu_susy_sig->setConstant( kFALSE ) ;

       printf("\n\n Freed SUSY SIG yield fit parameter.\n\n\n" ) ;

    }

  //===================================================================



    void ra2bRoostatsClass3::parameterSnapshot() {

       printf("\n\n====================================================================\n\n") ;

       printf("       Snapshot of observables and likelihood parameters.\n\n") ;


       printf("\n\n") ;


       printf("                     Nobs      all  |     ttwj      qcd     Znn       Ewo     SUSY \n") ;
       printf("------------------------------------+--------------------------------------------------------\n") ;

       float ttwjSig ;
       float ttwjSb ;

       if ( useSigTtwjVar ) {
          ttwjSig = rrv_mu_ttwj_sig->getVal() ;
          ttwjSb  = rfv_mu_ttwj_sb->getVal() ;
       } else {
          ttwjSig = rfv_mu_ttwj_sig->getVal() ;
          ttwjSb  = rrv_mu_ttwj_sb->getVal() ;
       }

       float qcdSig, qcdSb, qcdSigLdp, qcdSbLdp ;
       if ( useLdpVars ) {
          qcdSig    = rfv_mu_qcd_sig->getVal() ;
          qcdSb     = rfv_mu_qcd_sb->getVal() ;
          qcdSigLdp = rrv_mu_qcd_sig_ldp->getVal() ;
          qcdSbLdp  = rrv_mu_qcd_sb_ldp->getVal() ;
       } else {
          qcdSig    = rrv_mu_qcd_sig->getVal() ;
          qcdSb     = rrv_mu_qcd_sb->getVal() ;
          qcdSigLdp = rfv_mu_qcd_sig_ldp->getVal() ;
          qcdSbLdp  = rfv_mu_qcd_sb_ldp->getVal() ;
       }

       printf("                                    |\n") ;
       printf("     Nsig        :  %5.0f   %7.1f |  %7.1f  %7.1f  %7.1f  %7.1f  %7.1f (%7.1f)  \n",
              rv_Nsig->getVal(),
              rv_n_sig->getVal(),
              ttwjSig,
              qcdSig,
              rv_mu_znn_sig->getVal(),
              (rv_mu_ewo_sig->getVal())*(rv_eff_sf->getVal()),
              (rv_mu_susy_sig->getVal())*(rv_eff_sf->getVal()),
              rv_mu_susy_sig->getVal()
              ) ;


       printf("     Nsb         :  %5.0f   %7.1f |  %7.1f  %7.1f  %7.1f  %7.1f  %7.1f (%7.1f)  \n",
              rv_Nsb->getVal(),
              rv_n_sb->getVal(),
              ttwjSb,
              qcdSb,
              rv_mu_znn_sb->getVal(),
              (rv_mu_ewo_sb->getVal())*(rv_eff_sf->getVal()),
              (rv_mu_susy_sb->getVal())*(rv_eff_sf->getVal()),
              rv_mu_susy_sb->getVal()
              ) ;


       printf("                                    |\n") ;
       printf("     Nsig,SL     :  %5.0f   %7.1f |  %7.1f  -------  -------  -------  %7.1f (%7.1f)  \n",
              rv_Nsig_sl->getVal(),
              rv_n_sig_sl->getVal(),
              rv_mu_ttwj_sig_sl->getVal(),
              (rv_mu_susy_sig_sl->getVal())*(rv_eff_sf->getVal()),
              rv_mu_susy_sig_sl->getVal()
              ) ;


       printf("     Nsb,SL      :  %5.0f   %7.1f |  %7.1f  -------  -------  -------  %7.1f (%7.1f)  \n",
              rv_Nsb_sl->getVal(),
              rv_n_sb_sl->getVal(),
              rv_mu_ttwj_sb_sl->getVal(),
              (rv_mu_susy_sb_sl->getVal())*(rv_eff_sf->getVal()),
              rv_mu_susy_sb_sl->getVal()
              ) ;


       printf("                                    |\n") ;
       printf("     Nsig,ldp    :  %5.0f   %7.1f |  %7.1f  %7.1f  %7.1f  %7.1f  %7.1f (%7.1f)  \n",
              rv_Nsig_ldp->getVal(),
              rv_n_sig_ldp->getVal(),
              rv_mu_ttwj_sig_ldp->getVal(),
              qcdSigLdp,
              rv_mu_znn_sig_ldp->getVal(),
              (rv_mu_ewo_sig_ldp->getVal())*(rv_eff_sf->getVal()),
              (rv_mu_susy_sig_ldp->getVal())*(rv_eff_sf->getVal()),
              rv_mu_susy_sig_ldp->getVal()
              ) ;

       printf("     Nsb,ldp     :  %5.0f   %7.1f |  %7.1f  %7.1f  %7.1f  %7.1f  %7.1f (%7.1f)  \n",
              rv_Nsb_ldp->getVal(),
              rv_n_sb_ldp->getVal(),
              rv_mu_ttwj_sb_ldp->getVal(),
              qcdSbLdp,
              rv_mu_znn_sb_ldp->getVal(),
              (rv_mu_ewo_sb_ldp->getVal())*(rv_eff_sf->getVal()),
              (rv_mu_susy_sb_ldp->getVal())*(rv_eff_sf->getVal()),
              rv_mu_susy_sb_ldp->getVal()
              ) ;

       printf("                                    |\n") ;
       printf("------------------------------------+--------------------------------------------------------\n") ;

       printf("\n") ;

       float delta ;

       printf("\n") ;
       delta = 0. ;
       if ( EffScaleFactorErr>0 ) { delta = (rv_eff_sf->getVal()-EffScaleFactor)/EffScaleFactorErr ; }
       printf("  Eff scale fac. :  input = %4.2f +/- %4.2f ,   fit = %4.2f, delta = %4.2f sigma.  \n",
              EffScaleFactor,
              EffScaleFactorErr,
              rv_eff_sf->getVal(),
              delta ) ;

    // printf("\n") ;
    // delta = 0. ;
    // if ( lsf_WJmc_err>0 ) { delta = (rv_lsf_WJmc->getVal()-lsf_WJmc)/lsf_WJmc_err ; }
    // printf("  LSF W+jets     :  input = %4.2f +/- %4.2f ,   fit = %4.2f, delta = %4.2f sigma.  \n",
    //        lsf_WJmc,
    //        lsf_WJmc_err,
    //        rv_lsf_WJmc->getVal(),
    //        delta ) ;

    // delta = 0. ;
    // if ( lsf_Znnmc_err>0 ) { delta = (rv_lsf_Znnmc->getVal()-lsf_Znnmc)/lsf_Znnmc_err ; }
    // printf("  LSF Z->invis   :  input = %4.2f +/- %4.2f ,   fit = %4.2f, delta = %4.2f sigma.  \n",
    //        lsf_Znnmc,
    //        lsf_Znnmc_err,
    //        rv_lsf_Znnmc->getVal(),
    //        delta ) ;


       printf("\n\n====================================================================\n\n") ;

    }

  //===================================================================