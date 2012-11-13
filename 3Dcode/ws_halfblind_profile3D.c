
#include "TRandom2.h"
#include "TLegend.h"
#include "TFile.h"
#include "TLine.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TTree.h"
#include "TF1.h"
#include "TH1F.h"
#include "TString.h"
#include "TStyle.h"
#include "TText.h"
#include "RooGaussian.h"
#include "RooProdPdf.h"
#include "RooAbsPdf.h"
#include "RooWorkspace.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "RooRealVar.h"
#include "RooMCStudy.h"
#include "RooNLLVar.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooStats/ModelConfig.h"
#include "RooMinuit.h"

#include <string.h>

  using namespace RooFit;
  using namespace RooStats;

  //==============================================================================================

   void ws_halfblind_profile3D( const char* wsfile = "rootfiles/ws-met3-ht3-v2.root",
                                   const char* new_poi_name = "n_M2_H2_1b",
                                   int npoiPoints = 10,
                                   double poiMinVal = -1.,
                                   double poiMaxVal = -1.,
                                   double constraintWidth = 4.,
                                   double ymax = 5.,
                                   int verbLevel=0 ) {


     gStyle->SetOptStat(0) ;

     //--- make output directory.

     TString outputdir( wsfile ) ;
     outputdir.ReplaceAll("rootfiles/","outputfiles/scans-") ;
     outputdir.ReplaceAll(".root","") ;
     printf("\n\n Creating output directory: %s\n\n", outputdir.Data() ) ;
     char command[10000] ;
     sprintf(command, "mkdir -p %s", outputdir.Data() ) ;
     gSystem->Exec( command ) ;


     //--- Tell RooFit to shut up about anything less important than an ERROR.
      RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;



       if ( verbLevel > 0 ) { printf("\n\n Verbose level : %d\n\n", verbLevel) ; }


       TFile* wstf = new TFile( wsfile ) ;

       RooWorkspace* ws = dynamic_cast<RooWorkspace*>( wstf->Get("ws") );

       if ( verbLevel > 0 ) { ws->Print() ; }






       RooDataSet* rds = (RooDataSet*) ws->obj( "ra2b_observed_rds" ) ;

       if ( verbLevel > 0 ) {
          printf("\n\n\n  ===== RooDataSet ====================\n\n") ;
          rds->Print() ;
          rds->printMultiline(cout, 1, kTRUE, "") ;
       }





       ModelConfig* modelConfig = (ModelConfig*) ws->obj( "SbModel" ) ;
       RooAbsPdf* likelihood = modelConfig->GetPdf() ;

       RooRealVar* rrv_mu_susy_all0lep = ws->var("mu_susy_all0lep") ;
       if ( rrv_mu_susy_all0lep == 0x0 ) {
          printf("\n\n\n *** can't find mu_susy_all0lep in workspace.  Quitting.\n\n\n") ;
          return ;
       }





       //-- do BG only for now.
       rrv_mu_susy_all0lep->setVal(0.) ;
       rrv_mu_susy_all0lep->setConstant( kTRUE ) ;


  //// //-- Include floating susy
  //// rrv_mu_susy_all0lep->setVal(0.) ;
  //// rrv_mu_susy_all0lep->setConstant( kFALSE ) ;








       //-- do a prefit.

       printf("\n\n\n ====== Pre fit with unmodified nll var.\n\n") ;

       RooFitResult* dataFitResultSusyFixed = likelihood->fitTo(*rds, Save(true),Hesse(false),Minos(false),Strategy(1),PrintLevel(verbLevel));
       int dataSusyFixedFitCovQual = dataFitResultSusyFixed->covQual() ;
       if ( dataSusyFixedFitCovQual < 2 ) { printf("\n\n\n *** Failed fit!  Cov qual %d.  Quitting.\n\n", dataSusyFixedFitCovQual ) ; return ; }
       double dataFitSusyFixedNll = dataFitResultSusyFixed->minNll() ;

       if ( verbLevel > 0 ) {
          dataFitResultSusyFixed->Print("v") ;
       }

       printf("\n\n Nll value, from fit result : %.3f\n\n", dataFitSusyFixedNll ) ;

       delete dataFitResultSusyFixed ;






       //-- find the new POI parameter.
       RooAbsReal* new_poi_rar(0x0) ;

       new_poi_rar = ws->var( new_poi_name ) ;
       if ( new_poi_rar == 0x0 ) {
          printf("\n\n New POI %s is not a variable.  Trying function.\n\n", new_poi_name ) ;
          new_poi_rar = ws->function( new_poi_name ) ;
          if ( new_poi_rar == 0x0 ) {
             printf("\n\n New POI %s is not a function.  I quit.\n\n", new_poi_name ) ;
             return ;
          }
       } else {
          printf("\n\n     New POI %s is a variable with current value %.1f.\n\n", new_poi_name, new_poi_rar->getVal() ) ;
       }

       double startPoiVal = new_poi_rar->getVal() ;

       if ( npoiPoints <=0 ) {
          printf("\n\n Quitting now.\n\n" ) ;
          return ;
       }


      //--- The RooNLLVar is NOT equivalent to what minuit uses.
  //   RooNLLVar* nll = new RooNLLVar("nll","nll", *likelihood, *rds ) ;
  //   printf("\n\n Nll value, from construction : %.3f\n\n", nll->getVal() ) ;

      //--- output of createNLL IS what minuit uses, so use that.
       RooAbsReal* nll = likelihood -> createNLL( *rds, Verbose(true) ) ;

       RooRealVar* rrv_poiValue = new RooRealVar( "poiValue", "poiValue", 0., -10000., 10000. ) ;
   /// rrv_poiValue->setVal( poiMinVal ) ;
   /// rrv_poiValue->setConstant(kTRUE) ;

       RooRealVar* rrv_constraintWidth = new RooRealVar("constraintWidth","constraintWidth", 0.1, 0.1, 1000. ) ;
       rrv_constraintWidth -> setVal( constraintWidth ) ;
       rrv_constraintWidth -> setConstant(kTRUE) ;




       if ( verbLevel > 0 ) {
          printf("\n\n ======= debug likelihood print\n\n") ;
          likelihood->Print("v") ;
          printf("\n\n ======= debug nll print\n\n") ;
          nll->Print("v") ;
       }






    //----------------------------------------------------------------------------------------------

       RooMinuit* rminuit( 0x0 ) ;

       RooFormulaVar* plot_var( 0x0 ) ;

       char ignore_observable_name[50][100] ;

       sprintf( ignore_observable_name[0], "N_0lep_M4_H2_2b" ) ;
       sprintf( ignore_observable_name[1], "N_0lep_M4_H3_2b" ) ;
       sprintf( ignore_observable_name[2], "N_0lep_M4_H4_2b" ) ;

       sprintf( ignore_observable_name[3], "N_0lep_M2_H1_3b" ) ;
       sprintf( ignore_observable_name[4], "N_0lep_M2_H2_3b" ) ;
       sprintf( ignore_observable_name[5], "N_0lep_M2_H3_3b" ) ;
       sprintf( ignore_observable_name[6], "N_0lep_M2_H4_3b" ) ;

       sprintf( ignore_observable_name[7], "N_0lep_M3_H1_3b" ) ;
       sprintf( ignore_observable_name[8], "N_0lep_M3_H2_3b" ) ;
       sprintf( ignore_observable_name[9], "N_0lep_M3_H3_3b" ) ;
       sprintf( ignore_observable_name[10],"N_0lep_M3_H4_3b" ) ;

       sprintf( ignore_observable_name[11],"N_0lep_M4_H2_3b" ) ;
       sprintf( ignore_observable_name[12],"N_0lep_M4_H3_3b" ) ;
       sprintf( ignore_observable_name[13],"N_0lep_M4_H4_3b" ) ;

       int n_ignore_observable(14) ;

       RooAbsReal* rar_ignore_pdf[100] ;
       RooAbsReal* rar_ignore_obs[100] ;

       char ignoreTermFormula[10000] ;
       RooArgSet  ignorePdfList ;

       for ( int ii=0; ii < n_ignore_observable; ii++ ) {

          char name[100] ;

          sprintf( name, "pdf_%s", ignore_observable_name[ii] ) ;
          rar_ignore_pdf[ii] = ws -> pdf( name ) ;
          if ( rar_ignore_pdf[ii] == 0x0 ) {
             printf("\n\n\n *** Told to ignore %s but can't find %s\n\n", ignore_observable_name[ii], name ) ;
             return ;
          }

          rar_ignore_obs[ii] = ws -> var( ignore_observable_name[ii] ) ;
          ( (RooRealVar*) rar_ignore_obs[ii] ) -> setConstant(kTRUE) ; // probably not necessary, but can't hurt.

          if ( ii==0 ) {
             sprintf( ignoreTermFormula, "log(@%d)", ii ) ;
          } else {
             char buffer[10000] ;
             sprintf( buffer, "%s+log(@%d)", ignoreTermFormula, ii ) ;
             sprintf( ignoreTermFormula, "%s", buffer ) ;
          }

          ignorePdfList.add( *rar_ignore_pdf[ii] ) ;

       } // ii

       printf("\n\n Creating ignore formula var with : %s\n", ignoreTermFormula ) ;

       RooFormulaVar* rfv_ignorePdfTerm = new RooFormulaVar("ignorePdfTerm", ignoreTermFormula, ignorePdfList ) ;
       rfv_ignorePdfTerm -> Print() ;


       char minuit_formula[10000] ;
       sprintf( minuit_formula, "%s+%s*(%s-%s)*(%s-%s)+%s",
         nll->GetName(),
         rrv_constraintWidth->GetName(),
         new_poi_rar->GetName(), rrv_poiValue->GetName(),
         new_poi_rar->GetName(), rrv_poiValue->GetName(),
         rfv_ignorePdfTerm->GetName() ) ;

       printf("\n\n Creating new minuit variable with formula: %s\n\n", minuit_formula ) ;
       RooFormulaVar* new_minuit_var = new RooFormulaVar("new_minuit_var", minuit_formula,
           RooArgList( *nll,
                       *rrv_constraintWidth,
                       *new_poi_rar, *rrv_poiValue,
                       *new_poi_rar, *rrv_poiValue,
                       *rfv_ignorePdfTerm
                       ) ) ;

       printf("\n\n Current value is %.2f\n\n",
            new_minuit_var->getVal() ) ;

       rminuit = new RooMinuit( *new_minuit_var ) ;

       char plotvar_formula[10000] ;
       sprintf( plotvar_formula, "%s+%s",
              nll->GetName(),
              rfv_ignorePdfTerm->GetName()
              ) ;

       printf("\n\n Creating new plot variable with formula: %s\n\n", plotvar_formula ) ;
       plot_var = new RooFormulaVar("plot_var", plotvar_formula,
           RooArgList( *nll,
                       *rfv_ignorePdfTerm
                       ) ) ;

       printf("\n\n Current value is %.2f\n\n",
            plot_var->getVal() ) ;




       rminuit->setPrintLevel(verbLevel-1) ;
       if ( verbLevel <=0 ) { rminuit->setNoWarn() ; }

    //----------------------------------------------------------------------------------------------

       //-- If POI range is -1 to -1, automatically determine the range using the set value.

       if ( poiMinVal < 0. && poiMaxVal < 0. ) {

          printf("\n\n Automatic determination of scan range.\n\n") ;

          if ( startPoiVal <= 0. ) {
             printf("\n\n *** POI starting value zero or negative %g.  Quit.\n\n\n", startPoiVal ) ;
             return ;
          }

          poiMinVal = startPoiVal - 3.5 * sqrt(startPoiVal) ;
          poiMaxVal = startPoiVal + 6.0 * sqrt(startPoiVal) ;

          if ( poiMinVal < 0. ) { poiMinVal = 0. ; }

          printf("    Start val = %g.   Scan range:   %g  to  %g\n\n", startPoiVal, poiMinVal, poiMaxVal ) ;


       }



    //----------------------------------------------------------------------------------------------


       double poiVals[1000] ;
       double nllVals[1000] ;
       double tcmu_qcd_Vals[100] ;
       double tcmu_ttwj_Vals[100] ;
       double tcmu_znn_Vals[100] ;
       double tcmu_vv_Vals[100] ;
       double tcmu_susy_Vals[100] ;
       double mu_qcd_ldp_Vals[100] ;
       double mu_ttwj_sl_Vals[100] ;
       double trigeff_Vals[100] ;
       double trigeff_sl_Vals[100] ;
       double sf_ttwj_Vals[100] ;
       double sf_qcd_Vals[100] ;
       double allsmcomps_Vals[100] ;
       double allcomps_Vals[100] ;
       double pdf_sf_qcd_Vals[100] ;
       double pdf_sf_ttwj_Vals[100] ;
       double pdf_N_ldp_Vals[100] ;
       double pdf_N_sl_Vals[100] ;
       double minNllVal(1.e9) ;


       double allsmcomps_max(0.) ;
       double allcomps_max(0.) ;
       double nptf_max(0.) ;
       double ctrlmu_max(0.) ;
       double pdfs_max(0.) ;

       for ( int poivi=0; poivi < npoiPoints ; poivi++ ) {

          double poiValue = poiMinVal + poivi*(poiMaxVal-poiMinVal)/(1.*(npoiPoints-1)) ;

          rrv_poiValue -> setVal( poiValue ) ;
          rrv_poiValue -> setConstant( kTRUE ) ;


       //+++++++++++++++++++++++++++++++++++

          rminuit->migrad() ;
          rminuit->hesse() ;
          RooFitResult* rfr = rminuit->save() ;

       //+++++++++++++++++++++++++++++++++++


          if ( verbLevel > 0 ) { rfr->Print("v") ; }


          float fit_minuit_var_val = rfr->minNll() ;

          printf(" %02d : poi constraint = %.2f : allvars : MinuitVar, createNLL, PV, POI :    %.5f   %.5f   %.5f   %.5f\n",
                poivi, rrv_poiValue->getVal(), fit_minuit_var_val, nll->getVal(), plot_var->getVal(), new_poi_rar->getVal() ) ;
          cout << flush ;



          TString binstring( new_poi_name ) ;
          binstring.ReplaceAll("n_","") ;
          TString trigbinstring( binstring ) ;
          trigbinstring.Resize(5) ;

          char pname[1000] ;
          RooAbsReal* par(0x0) ;
          double mu_qcd(0.), mu_ttwj(0.), mu_znn(0.), mu_vv(0.), mu_susy(0.), trigeff(0.), trigeff_sl(0.), sf_ttwj(0.), sf_qcd(0.) ;
          double mu_qcd_ldp(0.), mu_ttwj_sl(0.) ;
          double pdf_sf_qcd(0.), pdf_sf_ttwj(0.), pdf_N_ldp(0.), pdf_N_sl(0.) ;

          sprintf( pname, "pdf_N_ldp_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             pdf_N_ldp = par -> getVal() ;
          }

          sprintf( pname, "pdf_N_1lep_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             pdf_N_sl = par -> getVal() ;
          }

          sprintf( pname, "pdf_sf_qcd_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             pdf_sf_qcd = par -> getVal() ;
          }

          sprintf( pname, "pdf_sf_ttwj_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             pdf_sf_ttwj = par -> getVal() ;
          }

          sprintf( pname, "mu_qcd_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             mu_qcd = par -> getVal() ;
          }

          sprintf( pname, "mu_ttwj_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             mu_ttwj = par -> getVal() ;
          }

          sprintf( pname, "mu_znn_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             mu_znn = par -> getVal() ;
          }

          sprintf( pname, "mu_vv_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             mu_vv = par -> getVal() ;
          }

          sprintf( pname, "mu_susy_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             mu_susy = par -> getVal() ;
          }

          sprintf( pname, "trigeff_%s", trigbinstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             trigeff = par -> getVal() ;
          }

          sprintf( pname, "trigeff_sl_%s", trigbinstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             trigeff_sl = par -> getVal() ;
          }

          sprintf( pname, "sf_ttwj_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             sf_ttwj = par -> getVal() ;
          }

          sprintf( pname, "sf_qcd_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             sf_qcd = par -> getVal() ;
          }

          sprintf( pname, "mu_qcd_ldp_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             mu_qcd_ldp = par -> getVal() ;
          }

          sprintf( pname, "mu_ttwj_sl_%s", binstring.Data() ) ;
          par = (RooAbsReal*) ws -> obj( pname ) ;
          if ( par == 0x0 ) {
             printf( " *** %s missing from workspace.\n", pname ) ;
          } else {
             mu_ttwj_sl = par -> getVal() ;
          }


          printf(" *** debug1 : mu_qcd=%6.2f, mu_ttwj=%6.2f, mu_znn=%6.2f, mu_vv=%6.2f, mu_susy=%6.2f, trigeff=%5.3f, trigeff_sl=%5.3f\n",
              mu_qcd, mu_ttwj, mu_znn, mu_vv, mu_susy, trigeff, trigeff_sl ) ;
          printf(" *** debug2 : mu_qcd_ldp=%6.2f, mu_ttwj_sl=%6.2f, sf_ttwj=%5.3f, sf_qcd=%6.3f\n",
              mu_qcd_ldp, mu_ttwj_sl, sf_ttwj, sf_qcd ) ;
          printf(" *** debug3 : pdf_sf_qcd=%12.10f, pdf_sf_ttwj=%12.10f, pdf_N_ldp=%12.10f, pdf_N_sl=%12.10f\n",
              pdf_sf_qcd, pdf_sf_ttwj, pdf_N_ldp, pdf_N_sl ) ;
          cout << flush ;

          tcmu_qcd_Vals[poivi] = trigeff * mu_qcd ;
          tcmu_ttwj_Vals[poivi] = trigeff_sl * mu_ttwj ;
          tcmu_znn_Vals[poivi] = trigeff_sl * mu_znn ;
          tcmu_vv_Vals[poivi] = trigeff_sl * mu_vv ;
          tcmu_susy_Vals[poivi] = trigeff_sl * mu_susy ;
          mu_qcd_ldp_Vals[poivi] = mu_qcd_ldp ;
          mu_ttwj_sl_Vals[poivi] = mu_ttwj_sl ;
          trigeff_Vals[poivi] = trigeff ;
          trigeff_sl_Vals[poivi] = trigeff_sl ;
          sf_ttwj_Vals[poivi] = sf_ttwj ;
          sf_qcd_Vals[poivi] = sf_qcd ;
          pdf_sf_qcd_Vals[poivi] = -2*log(pdf_sf_qcd) ;
          pdf_sf_ttwj_Vals[poivi] = -2*log(pdf_sf_ttwj) ;
          pdf_N_ldp_Vals[poivi] = -2*log(pdf_N_ldp) ;
          pdf_N_sl_Vals[poivi] = -2*log(pdf_N_sl) ;

          allsmcomps_Vals[poivi] = trigeff * mu_qcd + trigeff_sl * ( mu_ttwj + mu_qcd + mu_znn + mu_vv ) ;
          allcomps_Vals[poivi]   = new_poi_rar->getVal() ;

          if ( allsmcomps_Vals[poivi] > allsmcomps_max ) { allsmcomps_max = allsmcomps_Vals[poivi] ; }
          if ( allcomps_Vals[poivi] > allcomps_max ) { allcomps_max = allcomps_Vals[poivi] ; }

          if ( sf_ttwj > nptf_max ) { nptf_max = sf_ttwj ; }
          if ( sf_qcd > nptf_max ) { nptf_max = sf_qcd ; }
          if ( trigeff > nptf_max ) { nptf_max = trigeff ; }
          if ( trigeff_sl > nptf_max ) { nptf_max = trigeff_sl ; }

          if ( mu_qcd_ldp > ctrlmu_max ) { ctrlmu_max = mu_qcd_ldp ; }
          if ( mu_ttwj_sl > ctrlmu_max ) { ctrlmu_max = mu_ttwj_sl ; }

          if ( -2*log(pdf_sf_qcd) > pdfs_max ) { pdfs_max = -2*log(pdf_sf_qcd) ; }
          if ( -2*log(pdf_sf_ttwj) > pdfs_max ) { pdfs_max = -2*log(pdf_sf_ttwj) ; }
          if ( -2*log(pdf_N_ldp) > pdfs_max ) { pdfs_max = -2*log(pdf_N_ldp) ; }
          if ( -2*log(pdf_N_sl) > pdfs_max ) { pdfs_max = -2*log(pdf_N_sl) ; }


          poiVals[poivi] = new_poi_rar->getVal() ;
          nllVals[poivi] = plot_var->getVal() ;

          if ( nllVals[poivi] < minNllVal ) { minNllVal = nllVals[poivi] ; }

          delete rfr ;


       } // poivi

       double nllDiffVals[1000] ;

       double poiAtMinlnL(-1.) ;
       double poiAtMinusDelta2(-1.) ;
       double poiAtPlusDelta2(-1.) ;
       for ( int poivi=0; poivi < npoiPoints ; poivi++ ) {
          nllDiffVals[poivi] = 2.*(nllVals[poivi] - minNllVal) ;
          double poiValue = poiMinVal + poivi*(poiMaxVal-poiMinVal)/(1.*npoiPoints) ;
          if ( nllDiffVals[poivi] < 0.01 ) { poiAtMinlnL = poiValue ; }
          if ( poiAtMinusDelta2 < 0. && nllDiffVals[poivi] < 2.5 ) { poiAtMinusDelta2 = poiValue ; }
          if ( poiAtMinlnL > 0. && poiAtPlusDelta2 < 0. && nllDiffVals[poivi] > 2.0 ) { poiAtPlusDelta2 = poiValue ; }
       } // poivi

       printf("\n\n Estimates for poi at delta ln L = -2, 0, +2:  %g ,   %g ,   %g\n\n", poiAtMinusDelta2, poiAtMinlnL, poiAtPlusDelta2 ) ;




      //--- Main canvas

       TCanvas* cscan = (TCanvas*) gDirectory->FindObject("cscan") ;
       if ( cscan == 0x0 ) {
          printf("\n Creating canvas.\n\n") ;
          cscan = new TCanvas("cscan","Delta nll") ;
       }


       char gname[1000] ;

       TGraph* graph = new TGraph( npoiPoints, poiVals, nllDiffVals ) ;
       sprintf( gname, "scan_%s", new_poi_name ) ;
       graph->SetName( gname ) ;

       double poiBest(-1.) ;
       double poiMinus1stdv(-1.) ;
       double poiPlus1stdv(-1.) ;
       double twoDeltalnLMin(1e9) ;

       int nscan(1000) ;
       for ( int xi=0; xi<nscan; xi++ ) {

          double x = poiVals[0] + xi*(poiVals[npoiPoints-1]-poiVals[0])/(nscan-1) ;

          double twoDeltalnL = graph -> Eval( x ) ;

          if ( poiMinus1stdv < 0. && twoDeltalnL < 1.0 ) { poiMinus1stdv = x ; printf(" set m1 : %d, x=%g, 2dnll=%g\n", xi, x, twoDeltalnL) ;}
          if ( twoDeltalnL < twoDeltalnLMin ) { poiBest = x ; twoDeltalnLMin = twoDeltalnL ; }
          if ( twoDeltalnLMin < 0.3 && poiPlus1stdv < 0. && twoDeltalnL > 1.0 ) { poiPlus1stdv = x ; printf(" set p1 : %d, x=%g, 2dnll=%g\n", xi, x, twoDeltalnL) ;}

          if ( xi%100 == 0 ) { printf( " %4d : poi=%6.2f,  2DeltalnL = %6.2f\n", xi, x, twoDeltalnL ) ; }

       }
       printf("\n\n POI estimate :  %g  +%g  -%g    [%g,%g]\n\n",
               poiBest, (poiPlus1stdv-poiBest), (poiBest-poiMinus1stdv), poiMinus1stdv, poiPlus1stdv ) ;



       char htitle[1000] ;
       sprintf(htitle, "%s profile likelihood scan: -2ln(L/Lm)", new_poi_name ) ;
       TH1F* hscan = new TH1F("hscan", htitle, 10, poiMinVal, poiMaxVal ) ;
       hscan->SetMinimum(0.) ;
       hscan->SetMaximum(ymax) ;


       hscan->Draw() ;
       graph->SetLineColor(4) ;
       graph->SetLineWidth(3) ;
       graph->Draw("CP") ;
       gPad->SetGridx(1) ;
       gPad->SetGridy(1) ;
       cscan->Update() ;

       TLine* line = new TLine() ;
       line->SetLineColor(2) ;
       line->DrawLine(poiMinVal, 1., poiPlus1stdv, 1.) ;
       line->DrawLine(poiMinus1stdv,0., poiMinus1stdv, 1.) ;
       line->DrawLine(poiPlus1stdv ,0., poiPlus1stdv , 1.) ;

       TText* text = new TText() ;
       text->SetTextSize(0.04) ;
       char tstring[1000] ;

       sprintf( tstring, "%s = %.1f +%.1f -%.1f", new_poi_name, poiBest, (poiPlus1stdv-poiBest), (poiBest-poiMinus1stdv) ) ;
       text -> DrawTextNDC( 0.15, 0.85, tstring ) ;

       sprintf( tstring, "68%% interval [%.1f,  %.1f]", poiMinus1stdv, poiPlus1stdv ) ;
       text -> DrawTextNDC( 0.15, 0.78, tstring ) ;


       char hname[1000] ;
       sprintf( hname, "hscanout_%s", new_poi_name ) ;
       TH1F* hsout = new TH1F( hname,"scan results",4,0.,4.) ;
       double obsVal(-1.) ;
       hsout->SetBinContent(1, obsVal ) ;
       hsout->SetBinContent(2, poiPlus1stdv ) ;
       hsout->SetBinContent(3, poiBest ) ;
       hsout->SetBinContent(4, poiMinus1stdv ) ;
       TAxis* xaxis = hsout->GetXaxis() ;
       xaxis->SetBinLabel(1,"Observed val.") ;
       xaxis->SetBinLabel(2,"Model+1sd") ;
       xaxis->SetBinLabel(3,"Model") ;
       xaxis->SetBinLabel(4,"Model-1sd") ;

       char outrootfile[10000] ;
       sprintf( outrootfile, "%s/scan-hb-%s.root", outputdir.Data(), new_poi_name ) ;

       char outpdffile[10000] ;
       sprintf( outpdffile, "%s/scan-hb-%s.pdf", outputdir.Data(), new_poi_name ) ;

       cscan->Update() ; cscan->Draw() ;

       printf("\n Saving %s\n", outpdffile ) ;
       cscan->SaveAs( outpdffile ) ;




      //--- canvas with components.

       TCanvas* ccomps = (TCanvas*) gDirectory->FindObject("ccomps") ;
       if ( ccomps == 0x0 ) {
          printf("\n Creating canvas.\n\n") ;
          ccomps = new TCanvas("ccomps","Components") ;
       }

       TGraph* gr_tcmu_qcd = new TGraph( npoiPoints, poiVals, tcmu_qcd_Vals ) ;
       sprintf( gname, "scan_%s_tcmu_qcd", new_poi_name ) ;
       gr_tcmu_qcd->SetName( gname ) ;
       gr_tcmu_qcd->SetLineWidth( 2 ) ;
       gr_tcmu_qcd->SetLineColor( 2 ) ;
       gr_tcmu_qcd->SetFillColor( kWhite ) ;

       TGraph* gr_tcmu_ttwj = new TGraph( npoiPoints, poiVals, tcmu_ttwj_Vals ) ;
       sprintf( gname, "scan_%s_tcmu_ttwj", new_poi_name ) ;
       gr_tcmu_ttwj->SetName( gname ) ;
       gr_tcmu_ttwj->SetLineWidth( 2 ) ;
       gr_tcmu_ttwj->SetLineColor( 4 ) ;
       gr_tcmu_ttwj->SetFillColor( kWhite ) ;

       TGraph* gr_tcmu_znn = new TGraph( npoiPoints, poiVals, tcmu_znn_Vals ) ;
       sprintf( gname, "scan_%s_tcmu_znn", new_poi_name ) ;
       gr_tcmu_znn->SetName( gname ) ;
       gr_tcmu_znn->SetLineWidth( 2 ) ;
       gr_tcmu_znn->SetLineColor( 3 ) ;
       gr_tcmu_znn->SetFillColor( kWhite ) ;

       TGraph* gr_tcmu_vv = new TGraph( npoiPoints, poiVals, tcmu_vv_Vals ) ;
       sprintf( gname, "scan_%s_tcmu_vv", new_poi_name ) ;
       gr_tcmu_vv->SetName( gname ) ;
       gr_tcmu_vv->SetLineWidth( 2 ) ;
       gr_tcmu_vv->SetLineColor( kOrange+1 ) ;
       gr_tcmu_vv->SetFillColor( kWhite ) ;

       TGraph* gr_tcmu_susy = new TGraph( npoiPoints, poiVals, tcmu_susy_Vals ) ;
       sprintf( gname, "scan_%s_tcmu_susy", new_poi_name ) ;
       gr_tcmu_susy->SetName( gname ) ;
       gr_tcmu_susy->SetLineWidth( 2 ) ;
       gr_tcmu_susy->SetLineColor( 6 ) ;
       gr_tcmu_susy->SetFillColor( kWhite ) ;

       TGraph* gr_allsmcomps = new TGraph( npoiPoints, poiVals, allsmcomps_Vals ) ;
       sprintf( gname, "scan_%s_allsmcomps", new_poi_name ) ;
       gr_allsmcomps->SetName( gname ) ;
       gr_allsmcomps->SetLineWidth( 2 ) ;
       gr_allsmcomps->SetLineStyle( 2 ) ;
       gr_allsmcomps->SetLineColor( 1 ) ;
       gr_allsmcomps->SetFillColor( kWhite ) ;


       TGraph* gr_allcomps = new TGraph( npoiPoints, poiVals, allcomps_Vals ) ;
       sprintf( gname, "scan_%s_allcomps", new_poi_name ) ;
       gr_allcomps->SetName( gname ) ;
       gr_allcomps->SetLineWidth( 2 ) ;
       gr_allcomps->SetLineColor( 1 ) ;
       gr_allcomps->SetFillColor( kWhite ) ;


       TLegend* legend_comps = new TLegend(0.7,0.8, 0.85, 0.95) ;
       legend_comps->SetFillColor(kWhite) ;
       legend_comps -> AddEntry( gr_allcomps, "All" ) ;
       legend_comps -> AddEntry( gr_allsmcomps, "All SM" ) ;
       legend_comps -> AddEntry( gr_tcmu_qcd, "qcd" ) ;
       legend_comps -> AddEntry( gr_tcmu_ttwj, "ttwj" ) ;
       legend_comps -> AddEntry( gr_tcmu_znn, "Znn" ) ;
       legend_comps -> AddEntry( gr_tcmu_vv, "VV" ) ;
       legend_comps -> AddEntry( gr_tcmu_susy, "SUSY" ) ;


       sprintf(htitle, "%s SM components (in events)", new_poi_name ) ;
       TH1F* hcomps = new TH1F("hcomps", htitle, 10, poiMinVal, poiMaxVal ) ;
       hcomps->SetMinimum(0.) ;
       hcomps->SetMaximum(1.2 * allcomps_max ) ;

       hcomps->Draw() ;
       gr_tcmu_qcd->Draw("c") ;
       gr_tcmu_ttwj->Draw("c") ;
       gr_tcmu_znn->Draw("c") ;
       gr_tcmu_vv->Draw("c") ;
       gr_tcmu_susy->Draw("c") ;
       gr_allsmcomps->Draw("c") ;

       legend_comps->Draw() ;

       gPad->SetGridx(1) ;
       gPad->SetGridy(1) ;

       sprintf( outrootfile, "%s/scan-hb-%s-smcomps.root", outputdir.Data(), new_poi_name ) ;
       sprintf( outpdffile, "%s/scan-hb-%s-smcomps.pdf", outputdir.Data(), new_poi_name ) ;

       ccomps->Update() ; ccomps->Draw() ;

       printf("\n Saving %s\n", outpdffile ) ;
       ccomps->SaveAs( outpdffile ) ;








      //--- canvas with NP scale factors and trig eff.

       TCanvas* cnptf = (TCanvas*) gDirectory->FindObject("cnptf") ;
       if ( cnptf == 0x0 ) {
          printf("\n Creating canvas.\n\n") ;
          cnptf = new TCanvas("cnptf","Components") ;
       }

       TGraph* gr_trigeff = new TGraph( npoiPoints, poiVals, trigeff_Vals ) ;
       sprintf( gname, "scan_%s_trigeff", new_poi_name ) ;
       gr_trigeff->SetName( gname ) ;
       gr_trigeff->SetLineWidth( 2 ) ;
       gr_trigeff->SetLineColor( 1 ) ;
       gr_trigeff->SetLineStyle( 2 ) ;
       gr_trigeff->SetFillColor( kWhite ) ;

       TGraph* gr_trigeff_sl = new TGraph( npoiPoints, poiVals, trigeff_sl_Vals ) ;
       sprintf( gname, "scan_%s_trigeff_sl", new_poi_name ) ;
       gr_trigeff_sl->SetName( gname ) ;
       gr_trigeff_sl->SetLineWidth( 2 ) ;
       gr_trigeff_sl->SetLineColor( 1 ) ;
       gr_trigeff_sl->SetLineStyle( 3 ) ;
       gr_trigeff_sl->SetFillColor( kWhite ) ;

       TGraph* gr_sf_ttwj = new TGraph( npoiPoints, poiVals, sf_ttwj_Vals ) ;
       sprintf( gname, "scan_%s_sf_ttwj", new_poi_name ) ;
       gr_sf_ttwj->SetName( gname ) ;
       gr_sf_ttwj->SetLineWidth( 2 ) ;
       gr_sf_ttwj->SetLineColor( 4 ) ;
       gr_sf_ttwj->SetFillColor( kWhite ) ;

       TGraph* gr_sf_qcd = new TGraph( npoiPoints, poiVals, sf_qcd_Vals ) ;
       sprintf( gname, "scan_%s_sf_qcd", new_poi_name ) ;
       gr_sf_qcd->SetName( gname ) ;
       gr_sf_qcd->SetLineWidth( 2 ) ;
       gr_sf_qcd->SetLineColor( 2 ) ;
       gr_sf_qcd->SetFillColor( kWhite ) ;


       TLegend* legend_nptf = new TLegend(0.7,0.8, 0.85, 0.95) ;
       legend_nptf->SetFillColor(kWhite) ;
       legend_nptf -> AddEntry( gr_trigeff, "Trig eff, 0lep" ) ;
       legend_nptf -> AddEntry( gr_trigeff_sl, "Trig eff, 1lep" ) ;
       legend_nptf -> AddEntry( gr_sf_ttwj, "SF ttwj" ) ;
       legend_nptf -> AddEntry( gr_sf_qcd, "SF QCD" ) ;


       sprintf(htitle, "%s Closure NPs and trigger eff.", new_poi_name ) ;
       TH1F* hnptf = new TH1F("hnptf", htitle, 10, poiMinVal, poiMaxVal ) ;
       hnptf->SetMinimum(0.) ;
       hnptf->SetMaximum(1.1 * nptf_max ) ;

       hnptf->Draw() ;
       gr_trigeff->Draw("c") ;
       gr_trigeff_sl->Draw("c") ;
       gr_sf_ttwj->Draw("c") ;
       gr_sf_qcd->Draw("c") ;

       legend_nptf->Draw() ;

       gPad->SetGridx(1) ;
       gPad->SetGridy(1) ;

       sprintf( outrootfile, "%s/scan-hb-%s-nptf.root", outputdir.Data(), new_poi_name ) ;
       sprintf( outpdffile, "%s/scan-hb-%s-nptf.pdf", outputdir.Data(), new_poi_name ) ;

       cnptf->Update() ; cnptf->Draw() ;

       printf("\n Saving %s\n", outpdffile ) ;
       cnptf->SaveAs( outpdffile ) ;









      //--- canvas with control sample mus.

       TCanvas* cctrlmu = (TCanvas*) gDirectory->FindObject("cctrlmu") ;
       if ( cctrlmu == 0x0 ) {
          printf("\n Creating canvas.\n\n") ;
          cctrlmu = new TCanvas("cctrlmu","Control sample mus") ;
       }

       TGraph* gr_mu_qcd_ldp = new TGraph( npoiPoints, poiVals, mu_qcd_ldp_Vals ) ;
       sprintf( gname, "scan_%s_mu_qcd_ldp", new_poi_name ) ;
       gr_mu_qcd_ldp->SetName( gname ) ;
       gr_mu_qcd_ldp->SetLineWidth( 2 ) ;
       gr_mu_qcd_ldp->SetLineColor( 2 ) ;
       gr_mu_qcd_ldp->SetLineStyle( 1 ) ;
       gr_mu_qcd_ldp->SetFillColor( kWhite ) ;


       TGraph* gr_mu_ttwj_sl = new TGraph( npoiPoints, poiVals, mu_ttwj_sl_Vals ) ;
       sprintf( gname, "scan_%s_mu_ttwj_sl", new_poi_name ) ;
       gr_mu_ttwj_sl->SetName( gname ) ;
       gr_mu_ttwj_sl->SetLineWidth( 2 ) ;
       gr_mu_ttwj_sl->SetLineColor( 4 ) ;
       gr_mu_ttwj_sl->SetLineStyle( 1 ) ;
       gr_mu_ttwj_sl->SetFillColor( kWhite ) ;


       TLegend* legend_ctrlmu = new TLegend(0.7,0.8, 0.85, 0.95) ;
       legend_ctrlmu->SetFillColor(kWhite) ;

       legend_ctrlmu->AddEntry( gr_mu_qcd_ldp, "mu qcd LDP") ;
       legend_ctrlmu->AddEntry( gr_mu_ttwj_sl, "mu ttwj SL") ;


       sprintf(htitle, "%s Control sample mus", new_poi_name ) ;
       TH1F* hctrlmu = new TH1F("hctrlmu", htitle, 10, poiMinVal, poiMaxVal ) ;
       hctrlmu->SetMinimum(0.) ;
       hctrlmu->SetMaximum(1.1 * ctrlmu_max ) ;

       hctrlmu->Draw() ;
       gr_mu_qcd_ldp->Draw("c") ;
       gr_mu_ttwj_sl->Draw("c") ;

       legend_ctrlmu->Draw() ;

       gPad->SetGridx(1) ;
       gPad->SetGridy(1) ;

       sprintf( outrootfile, "%s/scan-hb-%s-ctrlmu.root", outputdir.Data(), new_poi_name ) ;
       sprintf( outpdffile, "%s/scan-hb-%s-ctrlmu.pdf", outputdir.Data(), new_poi_name ) ;

       cctrlmu->Update() ; cctrlmu->Draw() ;

       printf("\n Saving %s\n", outpdffile ) ;
       cctrlmu->SaveAs( outpdffile ) ;









      //--- canvas with pdfs.

       TCanvas* cpdfs = (TCanvas*) gDirectory->FindObject("cpdfs") ;
       if ( cpdfs == 0x0 ) {
          printf("\n Creating canvas.\n\n") ;
          cpdfs = new TCanvas("cpdfs","-ln(PDF)") ;
       }

       TGraph* gr_pdf_sf_qcd = new TGraph( npoiPoints, poiVals, pdf_sf_qcd_Vals ) ;
       sprintf( gname, "scan_%s_pdf_sf_qcd", new_poi_name ) ;
       gr_pdf_sf_qcd->SetName( gname ) ;
       gr_pdf_sf_qcd->SetLineWidth( 2 ) ;
       gr_pdf_sf_qcd->SetLineColor( 2 ) ;
       gr_pdf_sf_qcd->SetLineStyle( 1 ) ;
       gr_pdf_sf_qcd->SetFillColor( kWhite ) ;


       TGraph* gr_pdf_sf_ttwj = new TGraph( npoiPoints, poiVals, pdf_sf_ttwj_Vals ) ;
       sprintf( gname, "scan_%s_pdf_sf_ttwj", new_poi_name ) ;
       gr_pdf_sf_ttwj->SetName( gname ) ;
       gr_pdf_sf_ttwj->SetLineWidth( 2 ) ;
       gr_pdf_sf_ttwj->SetLineColor( 4 ) ;
       gr_pdf_sf_ttwj->SetLineStyle( 1 ) ;
       gr_pdf_sf_ttwj->SetFillColor( kWhite ) ;


       TGraph* gr_pdf_N_ldp = new TGraph( npoiPoints, poiVals, pdf_N_ldp_Vals ) ;
       sprintf( gname, "scan_%s_pdf_N_ldp", new_poi_name ) ;
       gr_pdf_N_ldp->SetName( gname ) ;
       gr_pdf_N_ldp->SetLineWidth( 2 ) ;
       gr_pdf_N_ldp->SetLineColor( 2 ) ;
       gr_pdf_N_ldp->SetLineStyle( 2 ) ;
       gr_pdf_N_ldp->SetFillColor( kWhite ) ;


       TGraph* gr_pdf_N_sl = new TGraph( npoiPoints, poiVals, pdf_N_sl_Vals ) ;
       sprintf( gname, "scan_%s_pdf_N_sl", new_poi_name ) ;
       gr_pdf_N_sl->SetName( gname ) ;
       gr_pdf_N_sl->SetLineWidth( 2 ) ;
       gr_pdf_N_sl->SetLineColor( 4 ) ;
       gr_pdf_N_sl->SetLineStyle( 2 ) ;
       gr_pdf_N_sl->SetFillColor( kWhite ) ;


       TLegend* legend_pdfs = new TLegend(0.7,0.8, 0.85, 0.95) ;
       legend_pdfs->SetFillColor(kWhite) ;

       legend_pdfs->AddEntry( gr_pdf_sf_qcd, "-2ln(pdf_sf_qcd)") ;
       legend_pdfs->AddEntry( gr_pdf_sf_ttwj, "-2ln(pdf_sf_ttwj)") ;
       legend_pdfs->AddEntry( gr_pdf_N_ldp, "-2ln(pdf_N_ldp)") ;
       legend_pdfs->AddEntry( gr_pdf_N_sl, "-2ln(pdf_N_sl)") ;


       sprintf(htitle, "%s -2ln(PDF) vals", new_poi_name ) ;
       TH1F* hpdfs = new TH1F("hpdfs", htitle, 10, poiMinVal, poiMaxVal ) ;
       hpdfs->SetMinimum(0.) ;
       hpdfs->SetMaximum(1.1 * pdfs_max ) ;

       hpdfs->Draw() ;
       gr_pdf_sf_qcd->Draw("c") ;
       gr_pdf_sf_ttwj->Draw("c") ;
       gr_pdf_N_ldp->Draw("c") ;
       gr_pdf_N_sl->Draw("c") ;

       legend_pdfs->Draw() ;

       gPad->SetGridx(1) ;
       gPad->SetGridy(1) ;

       sprintf( outrootfile, "%s/scan-hb-%s-pdfs.root", outputdir.Data(), new_poi_name ) ;
       sprintf( outpdffile, "%s/scan-hb-%s-pdfs.pdf", outputdir.Data(), new_poi_name ) ;

       cpdfs->Update() ; cpdfs->Draw() ;

       printf("\n Saving %s\n", outpdffile ) ;
       cpdfs->SaveAs( outpdffile ) ;












     //--- save in root file

       printf("\n Saving %s\n", outrootfile ) ;
       TFile fout(outrootfile,"recreate") ;
       graph->Write() ;
       gr_tcmu_qcd->Write() ;
       gr_tcmu_ttwj->Write() ;
       gr_tcmu_znn->Write() ;
       gr_tcmu_vv->Write() ;
       gr_allsmcomps->Write() ;
       hsout->Write() ;
       fout.Close() ;

       delete ws ;
       wstf->Close() ;

   }
