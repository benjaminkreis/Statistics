//
//   Owen Long, UCR
//   Harrison Prosper, FSU
//   Sezen Sekmen, FSU
//
//   a.g. : implementing trigger efficiency corrections 
//

#include "ra2bRoostatsClass7_noNSig_wideSideband_moreObservables.h"

#include <iostream>
#include <string.h>
#include <complex>
#include <map>

#include "TCanvas.h"
#include "TStyle.h"
#include "THStack.h"
#include "TLegend.h"
#include "TText.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom1.h"
#include "TRandom2.h"
#include "TH2F.h"
#include "TGaxis.h"
#include "TLine.h"
#include "TStringLong.h"
#include "TString.h"
#include "TPRegexp.h"

#include "RooArgSet.h"
#include "RooConstVar.h"
#include "RooTrace.h"
#include "RooUniform.h"
#include "RooAddition.h"
#include "RooGammaPdf.h"
#include "RooBetaPdf.h"
#include "RooBetaPdfWithPoissonGenerator.h"
#include "RooProduct.h"
#include "RooAddition.h"
#include "RooLinearVar.h"

#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooStats/LikelihoodIntervalPlot.h"
#include "RooStats/HypoTestResult.h"

#include "RooCFunction4Binding.h" 
#include "RooCFunction3Binding.h"
#include "RooTFnBinding.h" 
#include "TMath.h"

#include "TF1.h" 
#include "Math/WrappedTF1.h"
#include "Math/RootFinderAlgorithms.h"


//////////#include "LikelihoodIntervalPlot.cxx"

using namespace RooFit ;
using namespace RooStats ;
using namespace ROOT::Math ;


class gammaScalingFunctor
{
private:
  
  double currentSigma,desiredSigma;
  
public:

  gammaScalingFunctor(const double& currentSigma_,const double& desiredSigma_):currentSigma(currentSigma_),desiredSigma(desiredSigma_){};
  double sigmaModeCalc(double alpha)
  {
    long double scale = pow(-1 + sqrt(1 + 4*pow(currentSigma,2)),2*alpha)*pow(4,-alpha);
    long double linGamma = tgammal((long double) ((2 + 4*alpha + (1 + sqrt(1 + 4*pow(currentSigma,2)))/pow(currentSigma,2))/2.))/
      tgammal((long double) ((1 + 2*pow(currentSigma,2) + sqrt(1 + 4*pow(currentSigma,2)))/(2.*pow(currentSigma,2))));
    long double quadGamma = tgammal((long double) (alpha + (1 + 2*pow(currentSigma,2) + sqrt(1 + 4*pow(currentSigma,2)))/(2.*pow(currentSigma,2))))/
      tgammal((long double)((1 + 2*pow(currentSigma,2) + sqrt(1 + 4*pow(currentSigma,2)))/(2.*pow(currentSigma,2))));
    return sqrt(scale*(linGamma-quadGamma*quadGamma));
  };
  double sigmaModeIntersect(double* alpha, double*)
  {
    return sigmaModeCalc(*alpha) - desiredSigma;
  };

  double sigmaMeanCalc(double alpha)
  {
    long double scale = pow(pow(currentSigma,2),2*alpha) ;
    long double linGamma = tgammal(2*alpha + pow(currentSigma,-2))/tgammal(pow(currentSigma,-2));
    long double quadGamma = tgammal(alpha + pow(currentSigma,-2))/tgammal(pow(currentSigma,-2));
    return sqrt(scale*(linGamma-quadGamma*quadGamma));
  };
  double sigmaMeanIntersect(double* alpha, double*)
  {
    return sigmaMeanCalc(*alpha) - desiredSigma;
  };

  void setCurrentSigma(double currentSigma_){currentSigma=currentSigma_;};
  void setDesiredSigma(double desiredSigma_){desiredSigma=desiredSigma_;};
};

void gammaModeTransform(const double& mu_measured , const double& sigma_measured ,
		    double& k , double& theta ) 
{
  double mu2 = mu_measured*mu_measured;
  double sigma2 = sigma_measured*sigma_measured;
  // from Wikipedia's http://en.wikipedia.org/wiki/Gamma_distribution parameterization.
  theta = 0.5*(sqrt(mu2+4*sigma2)-mu_measured);
  k = sigma2/(theta*theta);

  cout << "For mu    : " << mu_measured << endl;
  cout << "For sigma : " << sigma_measured << endl;
  cout << "k is      : " << k << endl;
  cout << "theta is  : " << theta << endl;
}

void gammaMeanTransform(const double& mu_measured , const double& sigma_measured ,
			double& k , double& theta ) 
{
  double mu2 = mu_measured*mu_measured;
  double sigma2 = sigma_measured*sigma_measured;
  // from Wikipedia's http://en.wikipedia.org/wiki/Gamma_distribution parameterization.
  theta = sigma2/mu_measured;
  k = mu2/sigma2;

  cout << "For mu    : " << mu_measured << endl;
  cout << "For sigma : " << sigma_measured << endl;
  cout << "k is      : " << k << endl;
  cout << "theta is  : " << theta << endl;
}

void gammaCousinsTransform(const double& mu_measured , const double& sigma_measured ,
			double& k , double& theta ) 
{
  double mu2 = mu_measured*mu_measured;
  double sigma2 = sigma_measured*sigma_measured;
  // from Wikipedia's http://en.wikipedia.org/wiki/Gamma_distribution parameterization.
  theta = sigma2/mu_measured;
  k = mu2/sigma2+1;

  cout << "For mu    : " << mu_measured << endl;
  cout << "For sigma : " << sigma_measured << endl;
  cout << "k is      : " << k << endl;
  cout << "theta is  : " << theta << endl;
}

void betaPrimeModeTransform(const double& mu_measured , const double& sigma_measured ,
			    double& alpha , double& beta ) 
{
  complex<double> mu ( mu_measured, 0.0 ) ;
  complex<double> sigma2 ( sigma_measured * sigma_measured , 0.0 ) ;
  //complex<double> complex_beta = sigma2*2.;
  //complex<double> cubeRoot = pow( sqrt(pow(sigma2,3)+pow(sigma2,2)*105.+ sigma2*3.-1.) / (sigma2*sqrt(3)) + (pow(sigma2,3)+pow(sigma2,2)*168.+sigma2*93.+8.)/(pow(sigma2,3)*27.) , 1.0/3.0);
  //complex<double> complex_beta = cubeRoot + (pow(sigma2,2) + sigma2*31. + 4.)/(pow(sigma2,2)*9.*cubeRoot) + (sigma2*4.+2.)/(sigma2*3.);
  complex<double> complex_beta = (-0.3333333333333333*(-1.*mu - 1.*pow(mu,2) - 4.*sigma2))/sigma2 - (0.41997368329829105*
      (-1.*pow(-1.*mu - 1.*pow(mu,2) - 4.*sigma2,2) + 3.*sigma2*(-1. - 2.*mu - 2.*pow(mu,2) + 5.*sigma2)))/
    (sigma2*pow(2.*pow(mu,3) + 6.*pow(mu,4) + 6.*pow(mu,5) + 2.*pow(mu,6) + 9.*mu*sigma2 + 51.*pow(mu,2)*sigma2 + 84.*pow(mu,3)*sigma2 + 42.*pow(mu,4)*sigma2 + 
        36.*pow(sigma2,2) + 150.*mu*pow(sigma2,2) + 150.*pow(mu,2)*pow(sigma2,2) + 2.*pow(sigma2,3) + 
        sqrt(pow(2.*pow(mu,3) + 6.*pow(mu,4) + 6.*pow(mu,5) + 2.*pow(mu,6) + 9.*mu*sigma2 + 51.*pow(mu,2)*sigma2 + 84.*pow(mu,3)*sigma2 + 42.*pow(mu,4)*sigma2 + 
            36.*pow(sigma2,2) + 150.*mu*pow(sigma2,2) + 150.*pow(mu,2)*pow(sigma2,2) + 2.*pow(sigma2,3),2) + 
          4.*pow(-1.*pow(-1.*mu - 1.*pow(mu,2) - 4.*sigma2,2) + 3.*sigma2*(-1. - 2.*mu - 2.*pow(mu,2) + 5.*sigma2),3)),0.3333333333333333)) + 
   (0.26456684199469993*pow(2.*pow(mu,3) + 6.*pow(mu,4) + 6.*pow(mu,5) + 2.*pow(mu,6) + 9.*mu*sigma2 + 51.*pow(mu,2)*sigma2 + 84.*pow(mu,3)*sigma2 + 
        42.*pow(mu,4)*sigma2 + 36.*pow(sigma2,2) + 150.*mu*pow(sigma2,2) + 150.*pow(mu,2)*pow(sigma2,2) + 2.*pow(sigma2,3) + 
        sqrt(pow(2.*pow(mu,3) + 6.*pow(mu,4) + 6.*pow(mu,5) + 2.*pow(mu,6) + 9.*mu*sigma2 + 51.*pow(mu,2)*sigma2 + 84.*pow(mu,3)*sigma2 + 42.*pow(mu,4)*sigma2 + 
            36.*pow(sigma2,2) + 150.*mu*pow(sigma2,2) + 150.*pow(mu,2)*pow(sigma2,2) + 2.*pow(sigma2,3),2) + 
	     4.*pow(-1.*pow(-1.*mu - 1.*pow(mu,2) - 4.*sigma2,2) + 3.*sigma2*(-1. - 2.*mu - 2.*pow(mu,2) + 5.*sigma2),3)),0.3333333333333333))/sigma2;
  beta = complex_beta.real();
  alpha = 1. + mu_measured + mu_measured*beta;
  cout << "Beta Prime Mode Transform " << endl;
  cout << "The calculated value of alpha is : " << alpha << endl;
  cout << "The calculated value of beta is  : " << beta << endl;
  cout << "The input value of mu is         : " << mu_measured << endl;
  cout << "The input value of sigma is      : " << sigma_measured << endl;
  cout << "The calculated value of mu is    : " << (alpha - 1) / ( beta + 1 ) << endl;
  cout << "The calculated value of sigma is : " << sqrt( alpha * (alpha + beta - 1 ) / ( pow(beta - 1 , 2 ) * (beta - 2) ) ) << endl;
}

void betaModeTransform(const double& mu_measured , const double& sigma_measured ,
		   double& alpha , double& beta ) 
{
  //double sigma2 = sigma_measured*sigma_measured;
  //double mu2 = mu_measured*mu_measured;
  //double mu3 = mu_measured*mu2;
  //alpha = (mu2 - mu3 - mu_measured * sigma2) / sigma2 ;
  //beta = (alpha - alpha * mu_measured) / mu_measured ;
  complex<double> mu ( mu_measured, 0.0 ) ;
  complex<double> mu2 = mu * mu ;
  complex<double> mu3 = mu2 * mu ;
  complex<double> muMinusOne ( mu_measured - 1 , 0.0 ) ;
  complex<double> muMinusOne2 = muMinusOne * muMinusOne ;
  complex<double> muMinusOne3 = muMinusOne2 * muMinusOne ;
  complex<double> muMinusOne6 = muMinusOne3 * muMinusOne3 ;
  complex<double> sigma2 ( sigma_measured * sigma_measured , 0.0 ) ;
  complex<double> sigma4 = sigma2 * sigma2 ;
  complex<double> sigma6 = sigma4 * sigma2 ;
  complex<double> common_expression = pow( muMinusOne3 * ( muMinusOne3 * mu3 * -2. - 
							   muMinusOne * mu * 3. * ( muMinusOne * mu * 14. + 3. ) * sigma2 - 
							   ( mu * 5. - 3. ) * ( mu * 5. - 2. ) * sigma4 * 6. - sigma6 * 2. ) + 
					   sqrt( muMinusOne6 * pow( sigma2 - 2. * mu * sigma2 , 2 ) *
						 ( -1. * muMinusOne2 * mu2 + 
						   4. * ( -1. + ( muMinusOne ) * mu * ( -4. + 3. * ( muMinusOne ) * mu ) ) * sigma2 + 
						   4. * ( 11. + 47. * ( muMinusOne ) * mu ) * sigma4 + 4. * sigma6 ) ) * 5.196152422706632 ,
					   0.3333333333333333 );

  complex<double> complex_beta = ( ( muMinusOne2 * mu * 4. + ( mu * 7. - 4. ) * sigma2 * 4. + 
				     ( complex<double> ( 2.519842099789747 , 4.364494543886885 ) * muMinusOne2 *
				       ( muMinusOne2 * mu2 + ( muMinusOne * mu * 14. + 3. ) * sigma2 + sigma4 ) ) /
				     common_expression + complex<double> ( 1.5874010519681996 , -2.7494592739972052 ) * 
				     common_expression ) * 0.08333333333333333 ) / sigma2;
  beta = complex_beta.real();
  alpha = (2 * mu_measured - beta * mu_measured - 1) / ( mu_measured - 1 ) ;

  if( !(alpha >= 1) && !(beta >= 1) )
    {
      cout << "mode and variance impossible with beta pdf, setting uniform distribution" << endl;
      alpha = 1;
      beta = 1;
    }

  cout << "The calculated value of alpha is : " << alpha << endl;
  cout << "The calculated value of beta is  : " << beta << endl;
  cout << "The input value of mu is         : " << mu_measured << endl;
  cout << "The input value of sigma is      : " << sigma_measured << endl;
  //cout << "The calculated value of mu is    : " << (alpha - 1) / ( alpha + beta - 2 ) << endl;
  //cout << "The calculated value of sigma is : " << sqrt( alpha * beta / ( pow(alpha + beta,2) * (alpha + beta + 1) ) ) << endl;

  //beta = exp(sigma_measured);
  //alpha = (mu_measured);
  //
  //cout << "The input value of mu is         : " << mu_measured << endl;
  //cout << "The input value of sigma is      : " << sigma_measured << endl;
  //cout << "The input value of sigma/mu is   : " << sigma_measured/mu_measured << endl;
  //
  //cout << "The calculated value of alpha is : " << log(alpha) << endl;
  //cout << "The calculated value of beta is  : " << log(beta) << endl;

}

void betaMeanTransform(const double& mu_measured , const double& sigma_measured ,
		   double& alpha , double& beta ) 
{
  double mu2 = mu_measured*mu_measured;
  double mu3 = mu_measured*mu2;
  double sigma2 = sigma_measured*sigma_measured;

  alpha = (mu2-mu3-mu_measured*sigma2)/sigma2;
  beta = (mu_measured-1.)*(mu2 - mu_measured + sigma2)/sigma2;

  if( alpha<1 && beta<1 )
    {
      cout << "mean and variance un-physical with beta pdf, setting uniform distribution" << endl;
      alpha = 1;
      beta = 1;
    }

}


  //=====================================================================================================


   ra2bRoostatsClass7::ra2bRoostatsClass7( bool ArgUseSigTtwjVar, bool ArgUseLdpVars ) {

      gStyle->SetOptStat(0) ;

      useSigTtwjVar = ArgUseSigTtwjVar ;
      useLdpVars = ArgUseLdpVars ;


     //--- Tell RooFit to shut up about anything less important than an ERROR.
      //RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
  /// RooMsgService::instance().addStream(DEBUG,Topic(Tracing),OutputFile("debug.log")) ;
      printf("\n\n ==== RooFit output configuration ===============\n") ;
      RooMsgService::instance().Print("v") ;
      printf("\n\n ================================================\n") ;

      varsAtFitVals = false ;
      initialized = false ;




   }





  //===================================================================

   ra2bRoostatsClass7::~ra2bRoostatsClass7() { }



  //===================================================================

    bool ra2bRoostatsClass7::initialize( const char* infile ,
                                         const char* inputScanFile, double m0, double m12, bool isT1bbbb, double t1bbbbXsec,
					 const char* outfile, bool effInitFixed, bool constantNonPoisson , bool constantSigEff ) {


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
       int    Nsb_sl_e              ;
       int    Nsb_sl_mu             ;
       int    Nsig_ldp              ;
       int    Nsb_ldp               ;
   /// int    Nhtonlytrig_lsb_0b      ;
   /// int    Nhtonlytrig_lsb_0b_ldp  ;
       int    Nsb_ee                  ;
       int    Nsig_ee                 ;
       int    Nsb_mm                  ;
       int    Nsig_mm                 ;
       float  Nttbarmc_sig_ldp      ;
       float  Nttbarmc_sb_ldp       ;
       float  NWJmc_sig_ldp         ;
       float  NWJmc_sb_ldp          ;
       float  NZnnmc_sig_ldp        ;
       float  NZnnmc_sb_ldp         ;
       float  NEwomc_sig_ldp        ;
       float  NEwomc_sb_ldp         ;
       float  NEwomc_sig  ;
       float  NEwomc_sb   ;


       float  sf_mc            ;
       float  sf_mc_err        ;
       float  sf_qcd_sb        ;
       float  sf_qcd_sb_err    ;
       float  sf_qcd_sig       ;
       float  sf_qcd_sig_err   ;
       float  sf_ttwj_sig      ;
       float  sf_ttwj_sig_err  ;
       float  sf_ee            ;
       float  sf_ee_err        ;
       float  sf_mm            ;
       float  sf_mm_err        ;

       float  Rlsb_passfail     ;
       float  Rlsb_passfail_err ;

       // trigger efficiency corrections
       float eps_sb_mean ;
       float eps_sig_mean ;
       float eps_sb_ldp_mean ;
       float eps_sb_sl_e_mean ;
       float eps_sb_sl_mu_mean ;
       float eps_sig_sl_mean ;

       // trigger efficiency scale factors (with asymmetric errors)
       float epsSF_sb ;
       float epsSF_sb_errp ;
       float epsSF_sb_errm ;
       float epsSF_sig ;
       float epsSF_sig_errp ;
       float epsSF_sig_errm ;
       float epsSF_sb_ldp ;
       float epsSF_sb_ldp_errp ;
       float epsSF_sb_ldp_errm ;
       float epsSF_sb_sl_e ;
       float epsSF_sb_sl_e_errp ;
       float epsSF_sb_sl_e_errm ;
       float epsSF_sb_sl_mu ;
       float epsSF_sb_sl_mu_errp ;
       float epsSF_sb_sl_mu_errm ;
       float epsSF_sig_sl ;
       float epsSF_sig_sl_errp ;
       float epsSF_sig_sl_errm ;


       //--- read in description line.
       printf("\n\n") ;
       char c(0) ;
       while ( c!=10  ) { c = fgetc( infp ) ; printf("%c", c ) ; }
       printf("\n\n") ;


       char label[1000] ;

      //-----    The order here matters!

       fscanf( infp, "%s %d", label, &Nsig                  ) ;   printf( "%s %d\n", label, Nsig                  ) ;
       fscanf( infp, "%s %d", label, &Nsb                   ) ;   printf( "%s %d\n", label, Nsb                   ) ;
       fscanf( infp, "%s %d", label, &Nsig_sl               ) ;   printf( "%s %d\n", label, Nsig_sl               ) ;
       fscanf( infp, "%s %d", label, &Nsb_sl_e                ) ;   printf( "%s %d\n", label, Nsb_sl_e                ) ;
       //fscanf( infp, "%s %d", label, &Nsb_sl_mu                ) ;   printf( "%s %d\n", label, Nsb_sl_mu               ) ;
       fscanf( infp, "%s %d", label, &Nsig_ldp              ) ;   printf( "%s %d\n", label, Nsig_ldp              ) ;
       fscanf( infp, "%s %d", label, &Nsb_ldp               ) ;   printf( "%s %d\n", label, Nsb_ldp               ) ;
       fscanf( infp, "%s %d", label, &Nsb_ee                    ) ;   printf( "%s %d\n", label, Nsb_ee                    ) ;
       fscanf( infp, "%s %d", label, &Nsig_ee                   ) ;   printf( "%s %d\n", label, Nsig_ee                   ) ;
       fscanf( infp, "%s %d", label, &Nsb_mm                    ) ;   printf( "%s %d\n", label, Nsb_mm                    ) ;
       fscanf( infp, "%s %d", label, &Nsig_mm                   ) ;   printf( "%s %d\n", label, Nsig_mm                   ) ;
       fscanf( infp, "%s %g", label, &Rlsb_passfail         ) ;   printf( "%s %g\n", label, Rlsb_passfail         ) ;
       fscanf( infp, "%s %g", label, &Rlsb_passfail_err     ) ;   printf( "%s %g\n", label, Rlsb_passfail_err     ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sig_ldp      ) ;   printf( "%s %g\n", label, Nttbarmc_sig_ldp      ) ;
       fscanf( infp, "%s %g", label, &Nttbarmc_sb_ldp       ) ;   printf( "%s %g\n", label, Nttbarmc_sb_ldp       ) ;
       fscanf( infp, "%s %g", label, &NWJmc_sig_ldp         ) ;   printf( "%s %g\n", label, NWJmc_sig_ldp         ) ;
       fscanf( infp, "%s %g", label, &NWJmc_sb_ldp          ) ;   printf( "%s %g\n", label, NWJmc_sb_ldp          ) ;
       fscanf( infp, "%s %g", label, &NZnnmc_sig_ldp        ) ;   printf( "%s %g\n", label, NZnnmc_sig_ldp        ) ;
       fscanf( infp, "%s %g", label, &NZnnmc_sb_ldp         ) ;   printf( "%s %g\n", label, NZnnmc_sb_ldp         ) ;
       fscanf( infp, "%s %g", label, &NEwomc_sig_ldp        ) ;   printf( "%s %g\n", label, NEwomc_sig_ldp        ) ;
       fscanf( infp, "%s %g", label, &NEwomc_sb_ldp         ) ;   printf( "%s %g\n", label, NEwomc_sb_ldp         ) ;
       fscanf( infp, "%s %g", label, &NEwomc_sig        ) ;   printf( "%s %g\n", label, NEwomc_sig        ) ;
       fscanf( infp, "%s %g", label, &NEwomc_sb         ) ;   printf( "%s %g\n", label, NEwomc_sb         ) ;
       fscanf( infp, "%s %g", label, &DataLumi                  ) ;   printf( "%s %g\n", label, DataLumi                  ) ;
       fscanf( infp, "%s %g", label, &acc_ee_sig_mean              ) ;   printf( "%s %g\n", label, acc_ee_sig_mean               ) ;
       fscanf( infp, "%s %g", label, &acc_ee_sig_err               ) ;   printf( "%s %g\n", label, acc_ee_sig_err                ) ;
       fscanf( infp, "%s %g", label, &acc_ee_sb_mean               ) ;   printf( "%s %g\n", label, acc_ee_sb_mean                ) ;
       fscanf( infp, "%s %g", label, &acc_ee_sb_err                ) ;   printf( "%s %g\n", label, acc_ee_sb_err                 ) ;
       fscanf( infp, "%s %g", label, &acc_mm_sig_mean              ) ;   printf( "%s %g\n", label, acc_mm_sig_mean               ) ;
       fscanf( infp, "%s %g", label, &acc_mm_sig_err               ) ;   printf( "%s %g\n", label, acc_mm_sig_err                ) ;
       fscanf( infp, "%s %g", label, &acc_mm_sb_mean               ) ;   printf( "%s %g\n", label, acc_mm_sb_mean                ) ;
       fscanf( infp, "%s %g", label, &acc_mm_sb_err                ) ;   printf( "%s %g\n", label, acc_mm_sb_err                 ) ;
       fscanf( infp, "%s %g", label, &eff_ee_mean               ) ;   printf( "%s %g\n", label, eff_ee_mean               ) ;
       fscanf( infp, "%s %g", label, &eff_ee_err                ) ;   printf( "%s %g\n", label, eff_ee_err                ) ;
       fscanf( infp, "%s %g", label, &eff_mm_mean               ) ;   printf( "%s %g\n", label, eff_mm_mean               ) ;
       fscanf( infp, "%s %g", label, &eff_mm_err                ) ;   printf( "%s %g\n", label, eff_mm_err                ) ;
       fscanf( infp, "%s %g", label, &Ztoll_lumi                ) ;   printf( "%s %g\n", label, Ztoll_lumi                ) ;
       fscanf( infp, "%s %g", label, &knn_ee_sig_mean              ) ;   printf( "%s %g\n", label, knn_ee_sig_mean              ) ;
       fscanf( infp, "%s %g", label, &knn_ee_sig_err               ) ;   printf( "%s %g\n", label, knn_ee_sig_err               ) ;
       fscanf( infp, "%s %g", label, &knn_ee_sb_mean               ) ;   printf( "%s %g\n", label, knn_ee_sb_mean               ) ;
       fscanf( infp, "%s %g", label, &knn_ee_sb_err                ) ;   printf( "%s %g\n", label, knn_ee_sb_err                ) ;
       fscanf( infp, "%s %g", label, &knn_mm_sig_mean              ) ;   printf( "%s %g\n", label, knn_mm_sig_mean              ) ;
       fscanf( infp, "%s %g", label, &knn_mm_sig_err               ) ;   printf( "%s %g\n", label, knn_mm_sig_err               ) ;
       fscanf( infp, "%s %g", label, &knn_mm_sb_mean               ) ;   printf( "%s %g\n", label, knn_mm_sb_mean               ) ;
       fscanf( infp, "%s %g", label, &knn_mm_sb_err                ) ;   printf( "%s %g\n", label, knn_mm_sb_err                ) ;
       fscanf( infp, "%s %g", label, &fsig_ee_mean              ) ;   printf( "fsig_ee_mean    %s %g\n", label, fsig_ee_mean              ) ;
       fscanf( infp, "%s %g", label, &fsig_ee_err               ) ;   printf( "fsig_ee_err     %s %g\n", label, fsig_ee_err               ) ;
       fscanf( infp, "%s %g", label, &fsig_mm_mean              ) ;   printf( "fsig_mm_mean    %s %g\n", label, fsig_mm_mean              ) ;
       fscanf( infp, "%s %g", label, &fsig_mm_err               ) ;   printf( "fsig_mm_err     %s %g\n", label, fsig_mm_err               ) ;
       fscanf( infp, "%s %g", label, &sf_mc                     ) ;   printf( "sf_mc           %s %g\n", label, sf_mc                     ) ;
       fscanf( infp, "%s %g", label, &sf_mc_err                 ) ;   printf( "sf_mc_err       %s %g\n", label, sf_mc_err                 ) ;
       fscanf( infp, "%s %g", label, &sf_qcd_sb                 ) ;   printf( "sf_qcd_sb       %s %g\n", label, sf_qcd_sb                 ) ;
       fscanf( infp, "%s %g", label, &sf_qcd_sb_err             ) ;   printf( "sf_qcd_sb_err   %s %g\n", label, sf_qcd_sb_err             ) ;
       fscanf( infp, "%s %g", label, &sf_qcd_sig                ) ;   printf( "sf_qcd_sig      %s %g\n", label, sf_qcd_sig                ) ;
       fscanf( infp, "%s %g", label, &sf_qcd_sig_err            ) ;   printf( "sf_qcd_sig_err  %s %g\n", label, sf_qcd_sig_err            ) ;
       fscanf( infp, "%s %g", label, &sf_ttwj_sig               ) ;   printf( "sf_ttwj_sig     %s %g\n", label, sf_ttwj_sig               ) ;
       fscanf( infp, "%s %g", label, &sf_ttwj_sig_err           ) ;   printf( "sf_ttwj_sig_err %s %g\n", label, sf_ttwj_sig_err           ) ;
       fscanf( infp, "%s %g", label, &sf_ee                     ) ;   printf( "sf_ee           %s %g\n", label, sf_ee                     ) ;
       fscanf( infp, "%s %g", label, &sf_ee_err                 ) ;   printf( "sf_ee_err       %s %g\n", label, sf_ee_err                 ) ;
       fscanf( infp, "%s %g", label, &sf_mm                     ) ;   printf( "sf_mm           %s %g\n", label, sf_mm                     ) ;
       fscanf( infp, "%s %g", label, &sf_mm_err                 ) ;   printf( "sf_mm_err       %s %g\n", label, sf_mm_err                 ) ;
       fscanf( infp, "%s %g", label, &eps_sb_mean               ) ;   printf( "eps_sb_mean     %s %g\n", label, eps_sb_mean               ) ;
       fscanf( infp, "%s %g", label, &eps_sig_mean              ) ;   printf( "eps_sig_mean    %s %g\n", label, eps_sig_mean              ) ;
       fscanf( infp, "%s %g", label, &eps_sb_ldp_mean           ) ;   printf( "eps_sb_ldp_mean %s %g\n", label, eps_sb_ldp_mean           ) ;
       fscanf( infp, "%s %g", label, &eps_sb_sl_e_mean            ) ;   printf( "eps_sb_sl_e_mean  %s %g\n", label, eps_sb_sl_e_mean            ) ;
       //fscanf( infp, "%s %g", label, &eps_sb_sl_mu_mean            ) ;   printf( "eps_sb_sl_mu_mean  %s %g\n", label, eps_sb_sl_mu_mean            ) ;
       fscanf( infp, "%s %g", label, &eps_sig_sl_mean           ) ;   printf( "eps_sig_sl_mean %s %g\n", label, eps_sig_sl_mean           ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb                  ) ;   printf( "epsSF_sb          %s %g\n", label, epsSF_sb                ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_errp             ) ;   printf( "epsSF_sb_errp     %s %g\n", label, epsSF_sb_errp           ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_errm             ) ;   printf( "epsSF_sb_errm     %s %g\n", label, epsSF_sb_errm           ) ;
       fscanf( infp, "%s %g", label, &epsSF_sig                 ) ;   printf( "epsSF_sig         %s %g\n", label, epsSF_sig               ) ;
       fscanf( infp, "%s %g", label, &epsSF_sig_errp            ) ;   printf( "epsSF_sig_errp    %s %g\n", label, epsSF_sig_errp          ) ;
       fscanf( infp, "%s %g", label, &epsSF_sig_errm            ) ;   printf( "epsSF_sig_errm    %s %g\n", label, epsSF_sig_errm          ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_ldp              ) ;   printf( "epsSF_sb_ldp      %s %g\n", label, epsSF_sb_ldp            ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_ldp_errp         ) ;   printf( "epsSF_sb_ldp_errp %s %g\n", label, epsSF_sb_ldp_errp       ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_ldp_errm         ) ;   printf( "epsSF_sb_ldp_errm %s %g\n", label, epsSF_sb_ldp_errm       ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_sl_e               ) ;   printf( "epsSF_sb_sl_e       %s %g\n", label, epsSF_sb_sl_e             ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_sl_e_errp          ) ;   printf( "epsSF_sb_sl_e_errp  %s %g\n", label, epsSF_sb_sl_e_errp        ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_sl_e_errm          ) ;   printf( "epsSF_sb_sl_e_errm  %s %g\n", label, epsSF_sb_sl_e_errm        ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_sl_mu               ) ;   printf( "epsSF_sb_sl_mu       %s %g\n", label, epsSF_sb_sl_mu             ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_sl_mu_errp          ) ;   printf( "epsSF_sb_sl_mu_errp  %s %g\n", label, epsSF_sb_sl_mu_errp        ) ;
       fscanf( infp, "%s %g", label, &epsSF_sb_sl_mu_errm          ) ;   printf( "epsSF_sb_sl_mu_errm  %s %g\n", label, epsSF_sb_sl_mu_errm        ) ;
       fscanf( infp, "%s %g", label, &epsSF_sig_sl              ) ;   printf( "epsSF_sig_sl      %s %g\n", label, epsSF_sig_sl            ) ;
       fscanf( infp, "%s %g", label, &epsSF_sig_sl_errp         ) ;   printf( "epsSF_sig_sl_errp %s %g\n", label, epsSF_sig_sl_errp       ) ;
       fscanf( infp, "%s %g", label, &epsSF_sig_sl_errm         ) ;   printf( "epsSF_sig_sl_errm %s %g\n", label, epsSF_sig_sl_errm       ) ;

       printf("\n Done reading in %s\n\n", infile ) ;
       fclose( infp ) ;

       //More reading of files -- automated to read inputs from Josh

       map<TString,TString> cutHash;
       cutHash[TString("1BL")] = TString("ge1bLoose") ;
       cutHash[TString("1BT")] = TString("ge1bTight") ;
       cutHash[TString("2BL")] = TString("ge2bLoose") ;
       cutHash[TString("2BT")] = TString("ge2bTight") ;
       cutHash[TString("3B" )] = TString("ge3bLoose") ;

       TString dataInputFileName("/afs/cern.ch/user/j/joshmt/public/RA2bFall2011/RA2b_data_results_WideSB.0LSB.4684invpb.txt");

       TString selectionName;

       for(map<TString,TString>::iterator thisSelection = cutHash.begin(); thisSelection!=cutHash.end(); thisSelection++)
	 {
	   if(TString(infile).Contains(thisSelection->first)) selectionName = thisSelection->second;
	   if(TString(infile).Contains(thisSelection->first)) cout << infile << " contains " << thisSelection->first << endl;
	 }

       cout <<"The selection name for Josh's files is: " << selectionName << endl;

       TString  backgroundSystematicsFileName("/afs/cern.ch/user/j/joshmt/public/RA2bFall2011/backgroundSyst.");
       backgroundSystematicsFileName += selectionName ;
       backgroundSystematicsFileName += "WideSB.4684invpb.dat" ;

       map<string,double> dataInputs;
       map<string,double> backgroundSystematicsInputs;
       
       ifstream inFile;
       
       cout << "getting the file: " << dataInputFileName << endl;
       
       inFile.open(dataInputFileName.Data(),fstream::in);
       
       string fileLine;
       
       string index;
       double value;
       bool inHeader = true;
       
       while(!inFile.eof())
	 {
	   getline(inFile,fileLine);
	   TString thisLine(fileLine.c_str());
	   while(!thisLine.Contains("===") && !inFile.eof() && inHeader)
	     {
	       TStringToken nameAndNumber(thisLine," ");
	       nameAndNumber.NextToken();
	       index = nameAndNumber.Data();
	       nameAndNumber.NextToken();
	       value = nameAndNumber.Atof();
	       dataInputs[index] = value;
	       cout << index << " : " << value << endl;
	       getline(inFile,fileLine);
	       thisLine = fileLine.c_str();
	     }
	   if(inHeader && thisLine.Contains("===")) inHeader = false;
	   if(thisLine.Contains(selectionName) && !inFile.eof())
	     {
	       getline(inFile,fileLine);
	       thisLine = fileLine.c_str();
	       while(!thisLine.Contains("===") && !thisLine.Contains("luminosity") && !inFile.eof())
		 {
		   TStringToken nameAndNumber(thisLine," ");
		   nameAndNumber.NextToken();
		   index = nameAndNumber.Data();
		   nameAndNumber.NextToken();
		   value = nameAndNumber.Atof();
		   dataInputs[index] = value;
		   cout << index << " : " << value << endl;
		   getline(inFile,fileLine);
		   thisLine = fileLine.c_str();
		 }
	     }
	   if(thisLine.Contains("luminosity") && !inFile.eof())
	     {
	       TStringToken nameAndNumber(thisLine," ");
	       nameAndNumber.NextToken();
	       index = nameAndNumber.Data();
	       nameAndNumber.NextToken();
	       nameAndNumber.NextToken();
	       value = nameAndNumber.Atof();
	       dataInputs[index] = value;
	       cout << index << " : " << value << endl;
	     }
	 }

       eps_sb_mean	      = dataInputs["eff_SB_MHT"              ];
       epsSF_sb_errp 	      = dataInputs["eff_SB_MHT_err_plus"     ];
       epsSF_sb_errm 	      = dataInputs["eff_SB_MHT_err_minus"    ];
       eps_sb_ldp_mean	      = dataInputs["eff_SB_ldp_MHT"          ]; 
       epsSF_sb_ldp_errp      = dataInputs["eff_SB_ldp_MHT_err_plus" ]; 
       epsSF_sb_ldp_errm      = dataInputs["eff_SB_ldp_MHT_err_minus"]; 
       eps_sb_sl_e_mean	      = dataInputs["eff_SB_1e_MHT"           ]; 
       epsSF_sb_sl_e_errp     = dataInputs["eff_SB_1e_MHT_err_plus"  ]; 
       epsSF_sb_sl_e_errm     = dataInputs["eff_SB_1e_MHT_err_minus" ]; 
       eps_sb_sl_mu_mean      = dataInputs["eff_SB_1m_MHT"           ]; 
       epsSF_sb_sl_mu_errp    = dataInputs["eff_SB_1m_MHT_err_plus"  ]; 
       epsSF_sb_sl_mu_errm    = dataInputs["eff_SB_1m_MHT_err_minus" ]; 
       eps_sig_mean           = dataInputs["eff_SIG_MHT"             ]; 
       epsSF_sig_errp         = dataInputs["eff_SIG_MHT_err_plus"    ]; 
       epsSF_sig_errm         = dataInputs["eff_SIG_MHT_err_minus"   ]; 
       eps_sig_sl_mean	      = dataInputs["eff_SIG_sl_MHT"          ]; 
       epsSF_sig_sl_errp      = dataInputs["eff_SIG_sl_MHT_err_plus" ]; 
       epsSF_sig_sl_errm      = dataInputs["eff_SIG_sl_MHT_err_minus"];
 

       Nttbarmc_sig_ldp       = 0;
       Nttbarmc_sb_ldp        = 0;
       NWJmc_sig_ldp          = 0;
       NWJmc_sb_ldp           = 0;
       NZnnmc_sig_ldp         = 0;
       NZnnmc_sb_ldp          = 0;
       NEwomc_sig_ldp         = 0;
       NEwomc_sb_ldp          = 0;
       NEwomc_sig  	      = 0;
       NEwomc_sb              = 0;
       
       Nsig              = dataInputs["Nsig"                 ];
       Nsb               = dataInputs["Nsb"                  ];
       Nsig_sl           = dataInputs["Nsig_sl"              ];
       Nsb_sl_e          = dataInputs["Nsb_1e"               ];
       Nsb_sl_mu         = dataInputs["Nsb_1m"               ];
       Nsig_ldp          = dataInputs["Nsig_ldp"             ];
       Nsb_ldp           = dataInputs["Nsb_ldp"              ];
       Nttbarmc_sig_ldp  = dataInputs["Nttbarmc_sig_ldp"     ];
       Nttbarmc_sb_ldp   = dataInputs["Nttbarmc_sb_ldp"      ];
       Nttbarmc_sig_ldp += dataInputs["Nsingletopmc_sig_ldp" ];
       Nttbarmc_sb_ldp  += dataInputs["Nsingletopmc_sb_ldp"  ];
       Nttbarmc_sig_ldp += dataInputs["NWJmc_sig_ldp"        ];
       Nttbarmc_sb_ldp  += dataInputs["NWJmc_sb_ldp"         ];
       Nttbarmc_sig_ldp += dataInputs["NZnnmc_sig_ldp"       ];
       Nttbarmc_sb_ldp  += dataInputs["NZnnmc_sb_ldp"        ];
       Nttbarmc_sig_ldp += dataInputs["NZjmc_sig_ldp"        ];
       Nttbarmc_sb_ldp  += dataInputs["NZjmc_sb_ldp"         ];     
       Rlsb_passfail     = dataInputs["Rlsb_passfail"        ];
       Rlsb_passfail_err = dataInputs["Rlsb_passfail_err"    ];
 
       //By hand entered expected numbers!
       if(TString(infile).Contains(TString("expected")))
	  {
	    if( TString(infile).Contains("1BL") ) Nsig = 518.8;
	    if( TString(infile).Contains("1BT") ) Nsig = 9.4  ;
	    if( TString(infile).Contains("2BL") ) Nsig = 156.2;
	    if( TString(infile).Contains("2BT") ) Nsig = 29.5 ;
	    if( TString(infile).Contains("3B")  ) Nsig = 13.2 ;
	    cout << "Using expected value of Nsig = " << Nsig << endl;
	  }
       else cout << "File " << infile << " is measured input" << endl;

       cout << "getting the file: " << backgroundSystematicsFileName << endl;

       inFile.close();       
       
       inFile.open(backgroundSystematicsFileName.Data(),fstream::in);
       
       while(!inFile.eof())
	 {
	   getline(inFile,fileLine);
	   if(fileLine.size()>1)
	     {
	       TString thisLine = fileLine.c_str();
	       TStringToken nameAndNumber(thisLine," ");
	       nameAndNumber.NextToken();
	       index = nameAndNumber.Data();
	       nameAndNumber.NextToken();
	       value = nameAndNumber.Atof();
	       backgroundSystematicsInputs[index] = value;
	       cout << index << " : " << value << endl;
	     }
	 }
       
       inFile.close();
       
       sf_mc             = backgroundSystematicsInputs["sf_mc"           ];
       sf_mc_err         = backgroundSystematicsInputs["sf_mc_err"       ];
       sf_qcd_sb         = backgroundSystematicsInputs["sf_qcd_sb"       ];
       sf_qcd_sb_err     = backgroundSystematicsInputs["sf_qcd_sb_err"   ];
       sf_qcd_sig        = backgroundSystematicsInputs["sf_qcd_sig"      ];
       sf_qcd_sig_err    = backgroundSystematicsInputs["sf_qcd_sig_err"  ];
       sf_ttwj_sig       = backgroundSystematicsInputs["sf_ttwj_sig"     ];
       sf_ttwj_sig_err   = backgroundSystematicsInputs["sf_ttwj_sig_err" ];


    //---- calculations for determining initial values for floating parameters.

     //-- Znunu stuff

       float initialval_znn_sig_ee(2.) ;
       float initialval_znn_sig_mm(2.) ;
       float initialval_znn_sig(2.) ;
       float initialval_znn_sb_ee(2.) ;
       float initialval_znn_sb_mm(2.) ;
       float initialval_znn_sb(2.) ;


       initialval_znn_sig_ee = (Nsig_ee) * ( 5.95 * DataLumi * fsig_ee_mean * knn_ee_sig_mean ) / ( acc_ee_sig_mean * eff_ee_mean * Ztoll_lumi ) ;
       initialval_znn_sb_ee  = (Nsb_ee ) * ( 5.95 * DataLumi * fsig_ee_mean * knn_ee_sb_mean  ) / ( acc_ee_sb_mean  * eff_ee_mean * Ztoll_lumi ) ;

       initialval_znn_sig_mm = (Nsig_mm) * ( 5.95 * DataLumi * fsig_mm_mean * knn_mm_sig_mean ) / ( acc_mm_sig_mean * eff_mm_mean * Ztoll_lumi ) ;
       initialval_znn_sb_mm  = (Nsb_mm ) * ( 5.95 * DataLumi * fsig_mm_mean * knn_mm_sb_mean  ) / ( acc_mm_sb_mean  * eff_mm_mean * Ztoll_lumi ) ;


       //-- really dumb ave.
       initialval_znn_sig = 0.5 * ( initialval_znn_sig_ee + initialval_znn_sig_mm ) ;
       initialval_znn_sb  = 0.5 * ( initialval_znn_sb_ee  + initialval_znn_sb_mm ) ;


  /////--- QCD lsb stuff

  ///  double initialval_qcd_lsb_0b     = Nhtonlytrig_lsb_0b     ;
  ///  double initialval_qcd_lsb_0b_ldp = Nhtonlytrig_lsb_0b_ldp ;


     //--- QCD ldp stuff

       double initialval_qcd_sig_ldp = ( Nsig_ldp / eps_sig_mean ) - ( Nttbarmc_sig_ldp + NWJmc_sig_ldp + NZnnmc_sig_ldp + NEwomc_sig_ldp ) ;
       double initialval_qcd_sb_ldp  = ( Nsb_ldp  / eps_sb_ldp_mean ) - ( Nttbarmc_sb_ldp  + NWJmc_sb_ldp  + NZnnmc_sb_ldp  + NEwomc_sb_ldp  ) ;


     //--- QCD sig and sb stuff

   /// if ( Nhtonlytrig_lsb_0b_ldp <= 0 ) {
   ///    printf("\n\n\n *** Nhtonlytrig_lsb_0b_ldp has a crazy value (%d).  I quit.\n\n\n", Nhtonlytrig_lsb_0b_ldp ) ;
   ///    return false ;
   /// }
   /// double Rldp = initialval_qcd_lsb_0b / initialval_qcd_lsb_0b_ldp ;
       double initialval_qcd_sig = Rlsb_passfail * initialval_qcd_sig_ldp ;
       double initialval_qcd_sb  = Rlsb_passfail * initialval_qcd_sb_ldp  ;


     //--- ttwj SL

       double initialval_ttwj_sig_sl = Nsig_sl / eps_sig_sl_mean ;
       double initialval_ttwj_sb_sl_e  = Nsb_sl_e / eps_sb_sl_e_mean ;
       double initialval_ttwj_sb_sl_mu  = Nsb_sl_mu / eps_sb_sl_mu_mean ;


     //--- ttwj sig and sb

       if ( initialval_ttwj_sb_sl_e <= 0 || initialval_ttwj_sb_sl_mu <= 0) {
          printf("\n\n\n *** initialval_ttwj_sb_sl_e has a crazy value (%.1f).  I quit.\n\n\n", initialval_ttwj_sb_sl_e ) ;
          printf("\n\n\n *** initialval_ttwj_sb_sl_mu has a crazy value (%.1f).  I quit.\n\n\n", initialval_ttwj_sb_sl_mu ) ;
          return false ;
       }
       double initialval_ttwj_sb     = ( Nsb / eps_sb_mean ) - ( initialval_qcd_sb + initialval_znn_sb ) ;
       double initialval_ttwj_sig    = initialval_ttwj_sb * ( initialval_ttwj_sig_sl / (0.5*initialval_ttwj_sb_sl_e+0.5*initialval_ttwj_sb_sl_mu) ) ;



       printf("\n\n\n --------- Observables and floating parameter initial values. ------------\n\n") ;

       printf("          |  Nobs   ||  ttwj  |  QCD  |  Znn  |\n") ;
       printf(" SIG      | %5d   || %5.1f | %5.1f | %5.1f |\n", Nsig, initialval_ttwj_sig, initialval_qcd_sig, initialval_znn_sig ) ;
       printf(" SB       | %5d   || %5.1f | %5.1f | %5.1f |\n", Nsb , initialval_ttwj_sb , initialval_qcd_sb , initialval_znn_sb  ) ;
       printf(" SIG,LDP  | %5d   ||*%5.1f | %5.1f |*%5.1f |\n", Nsig_ldp, (Nttbarmc_sig_ldp+NWJmc_sig_ldp+NEwomc_sig_ldp), initialval_qcd_sig_ldp, NZnnmc_sig_ldp ) ;
       printf(" SB ,LDP  | %5d   ||*%5.1f | %5.1f |*%5.1f |\n", Nsb_ldp , (Nttbarmc_sb_ldp +NWJmc_sb_ldp +NEwomc_sb_ldp) , initialval_qcd_sb_ldp , NZnnmc_sb_ldp  ) ;
       printf("\n * means fixed MC value.\n\n\n") ;




     //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


       printf(" --- Defining observables.\n" ) ;


      rv_Nsig        = new RooRealVar( "Nsig"        , "Nsig"        , 0.0, 1000000. ) ;
      rv_Nsb         = new RooRealVar( "Nsb"         , "Nsb"         , 0.0, 1000000. ) ;

      rv_Nsig_sl     = new RooRealVar( "Nsig_sl"     , "Nsig_sl"     , 0.0, 1000000. ) ;
      rv_Nsb_sl_e      = new RooRealVar( "Nsb_sl_e"      , "Nsb_sl_e"      , 0.0, 1000000. ) ;
      rv_Nsb_sl_mu      = new RooRealVar( "Nsb_sl_mu"      , "Nsb_sl_mu"      , 0.0, 1000000. ) ;

      rv_Nsig_ldp    = new RooRealVar( "Nsig_ldp"    , "Nsig_ldp"    , 0.0, 1000000. ) ;
      rv_Nsb_ldp     = new RooRealVar( "Nsb_ldp"     , "Nsb_ldp"     , 0.0, 1000000. ) ;

 ///  rv_Nlsb_0b     = new RooRealVar( "Nlsb_0b"     , "Nlsb_0b"     , 0.0, 1000000. ) ;
 ///  rv_Nlsb_0b_ldp = new RooRealVar( "Nlsb_0b_ldp" , "Nlsb_0b_ldp" , 0.0, 1000000. ) ;


      rv_Nsb_ee      = new RooRealVar( "Nsb_ee"      ,"Nsb_ee"       , 0., 10000. ) ;
      rv_Nsig_ee     = new RooRealVar( "Nsig_ee"     ,"Nsig_ee"      , 0., 10000. ) ;

      rv_Nsb_mm      = new RooRealVar( "Nsb_mm"      ,"Nsb_mm"       , 0., 10000. ) ;
      rv_Nsig_mm     = new RooRealVar( "Nsig_mm"     ,"Nsig_mm"      , 0., 10000. ) ;






      rv_Nsig        -> setVal( Nsig ) ;
      rv_Nsb         -> setVal( Nsb ) ;

      rv_Nsig_sl     -> setVal( Nsig_sl ) ;
      rv_Nsb_sl_e      -> setVal( Nsb_sl_e ) ;
      rv_Nsb_sl_mu      -> setVal( Nsb_sl_mu ) ;

      rv_Nsig_ldp    -> setVal( Nsig_ldp ) ;
      rv_Nsb_ldp     -> setVal( Nsb_ldp ) ;

  /// rv_Nlsb_0b     -> setVal( Nhtonlytrig_lsb_0b ) ;
  /// rv_Nlsb_0b_ldp -> setVal( Nhtonlytrig_lsb_0b_ldp ) ;


      rv_Nsb_ee      -> setVal( Nsb_ee ) ;
      rv_Nsig_ee     -> setVal( Nsig_ee ) ;

      rv_Nsb_mm      -> setVal( Nsb_mm ) ;
      rv_Nsig_mm     -> setVal( Nsig_mm ) ;








    //++++++++ Parameters of the likelihood +++++++++++++++++++++++++++++++++++++++++

      printf(" --- Defining parameters.\n" ) ;

      RooArgSet globalObservables ("globalObservables");
      RooArgSet allNuisances ("allNuisances");
      RooArgSet allPoissonNuisances ("allPoissonNuisances");
      RooArgSet allNonPoissonNuisances ("allNonPoissonNuisances");
      //RooArgSet gammaNuisances ("gammaNuisances");
      RooArgSet normalNuisances ("normalNuisances");
      RooArgSet allNuisancePdfs ("allNuisancePdfs");
      RooArgSet gammaNuisancePdfs ("gammaNuisancePdfs");
      RooArgSet normalNuisancePdfs ("normalNuisancePdfs");
      RooArgSet betaNuisancePdfs ("betaNuisancePdfs");

    //____ Counts in SIG ______________________

      cout << " --- SIG" << endl;

      if ( useSigTtwjVar ) {
         rrv_mu_ttwj_sig = new RooRealVar( "mu_ttwj_sig"   , "mu_ttwj_sig"   , 1e-9, 10000. ) ;
         rv_mu_ttwj_sig = rrv_mu_ttwj_sig ;
	 allPoissonNuisances.add(*rv_mu_ttwj_sig);
         rrv_mu_ttwj_sig   -> setVal( initialval_ttwj_sig ) ;  //-- this is a starting value only.
      }
      if ( !useLdpVars ) {
         rrv_mu_qcd_sig  = new RooRealVar( "mu_qcd_sig"    , "mu_qcd_sig"    , 1e-9, 200. ) ;
         rv_mu_qcd_sig = rrv_mu_qcd_sig ;
	 allPoissonNuisances.add(*rv_mu_qcd_sig);
         rrv_mu_qcd_sig  -> setVal( initialval_qcd_sig ) ; //-- this is a starting value only.
      }


      //-- Note: Ewo is rfv

      rv_mu_znn_sig      = new RooRealVar( "mu_znn_sig"    , "mu_znn_sig"    , 1e-9, 300. ) ;
      allPoissonNuisances.add(*rv_mu_znn_sig);

      float maxSusySig = 4.0*Nsig ;
      rv_mu_susy_sig     = new RooRealVar( "mu_susy_sig"   , "mu_susy_sig"   , 1e-9, maxSusySig ) ;


      rv_mu_znn_sig   -> setVal( initialval_znn_sig ) ;  //-- this is a starting value only.
      rv_mu_susy_sig    -> setVal( 0. ) ;  //-- this is a starting value only.





    //____ Counts in SB  ______________________
      cout << " --- SB" << endl;

      if ( !useSigTtwjVar ) {
         rrv_mu_ttwj_sb  = new RooRealVar( "mu_ttwj_sb"    , "mu_ttwj_sb"    , 1e-9, 10000. ) ;
         rv_mu_ttwj_sb = rrv_mu_ttwj_sb ;
	 allPoissonNuisances.add(*rv_mu_ttwj_sb);
         rrv_mu_ttwj_sb   -> setVal( initialval_ttwj_sb ) ;  //-- this is a starting value only.
      }
      if ( !useLdpVars ) {
         rrv_mu_qcd_sb  = new RooRealVar( "mu_qcd_sb"    , "mu_qcd_sb"    , 1e-9, 500. ) ;
         rv_mu_qcd_sb = rrv_mu_qcd_sb ;
	 allPoissonNuisances.add(*rv_mu_qcd_sb);
         rrv_mu_qcd_sb  -> setVal( initialval_qcd_sb ) ; //-- this is a starting value only.
      }

      //-- Note: QCD is rfv
      //-- Note: Ewo is rfv
      //-- Note: SUSY is rfv


      rrv_mu_znn_sb       = new RooRealVar( "mu_znn_sb"     , "mu_znn_sb"     , 1e-9, 350. ) ;

      rrv_mu_znn_sb   -> setVal( initialval_znn_sb ) ;  //-- this is a starting value only.

      rv_mu_znn_sb = rrv_mu_znn_sb ;
      allPoissonNuisances.add(*rv_mu_znn_sb);

      //-- Note: Znn is rfv in Znn model 2.






    //____ Counts in SIG, SL  ______________________

      cout << " --- SIG, SL" << endl;

      rv_mu_ttwj_sig_sl  = new RooRealVar( "mu_ttwj_sig_sl"    , "mu_ttwj_sig_sl"    , 1e-9, 2500. ) ;
      allPoissonNuisances.add(*rv_mu_ttwj_sig_sl);

      //-- Note: QCD, Ewo, and Znn are assumed to be negligible and are not explicitly included.
      //-- Note: SUSY is rfv

      rv_mu_ttwj_sig_sl  -> setVal( initialval_ttwj_sig_sl ) ;  //-- this is a starting value only.







    //____ Counts in SB, SL  ______________________

      cout << " --- SB, SL" << endl;

      rv_mu_ttwj_sb_sl_e  = new RooRealVar( "mu_ttwj_sb_sl_e"    , "mu_ttwj_sb_sl_e"    , 1e-9, 3000. ) ;
      rv_mu_ttwj_sb_sl_mu  = new RooRealVar( "mu_ttwj_sb_sl_mu"    , "mu_ttwj_sb_sl_mu"    , 1e-9, 3000. ) ;
      allPoissonNuisances.add(*rv_mu_ttwj_sb_sl_e);
      allPoissonNuisances.add(*rv_mu_ttwj_sb_sl_mu);

      //-- Note: QCD, Ewo, and Znn are assumed to be negligible and are not explicitly included.
      //-- Note: SUSY is rfv

      rv_mu_ttwj_sb_sl_e  -> setVal( initialval_ttwj_sb_sl_e ) ;  //-- this is a starting value only.
      rv_mu_ttwj_sb_sl_mu  -> setVal( initialval_ttwj_sb_sl_mu ) ;  //-- this is a starting value only.







    //____ Counts in SIG, LDP  ______________________

      cout << " --- SIG, LDP" << endl;

      if ( useLdpVars ) {
         rrv_mu_qcd_sig_ldp  = new RooRealVar( "mu_qcd_sig_ldp"    , "mu_qcd_sig_ldp"    , 1e-9, 3500. ) ;
         rv_mu_qcd_sig_ldp = rrv_mu_qcd_sig_ldp ;
	 allPoissonNuisances.add(*rv_mu_qcd_sig_ldp);
         rrv_mu_qcd_sig_ldp  -> setVal( initialval_qcd_sig_ldp ) ; //-- this is a starting value only.
      }

      //-- Note: Ewo is assumed to be negligible and is not explicitly included.
      //-- Note: Znn is rfv (MC)
      //-- Note: ttwj is rfv (MC)
      //-- Note: SUSY is rfv








    //____ Counts in SB, LDP  ______________________

      cout << " --- SB, LDP" << endl;

      if ( useLdpVars ) {
         rrv_mu_qcd_sb_ldp  = new RooRealVar( "mu_qcd_sb_ldp"    , "mu_qcd_sb_ldp"    , 1e-9, 3000. ) ;
         rv_mu_qcd_sb_ldp = rrv_mu_qcd_sb_ldp ;
	 allPoissonNuisances.add(*rv_mu_qcd_sb_ldp);
         rrv_mu_qcd_sb_ldp  -> setVal( initialval_qcd_sb_ldp ) ; //-- this is a starting value only.
      }


      //-- Note: Ewo is assumed to be negligible and is not explicitly included.
      //-- Note: Znn is rfv (MC)
      //-- Note: ttwj is rfv (MC)
      //-- Note: SUSY is rfv








 // //____ Counts in LSB, 0b  ______________________

 //   rv_mu_qcd_lsb_0b  = new RooRealVar( "mu_qcd_lsb_0b"    ,  "mu_qcd_lsb_0b" ,  0.    ,  10000. ) ;

 //   //-- Note: The 0btag LSB is assumed to be 100% QCD.

 //   rv_mu_qcd_lsb_0b  -> setVal( initialval_qcd_lsb_0b ) ;  //-- this is a starting value only.






 // //____ Counts in LSB, 0b, LDP  ______________________

 //   rv_mu_qcd_lsb_0b_ldp  = new RooRealVar( "mu_qcd_lsb_0b_ldp"    ,  "mu_qcd_lsb_0b_ldp",   0. ,  10000. ) ;

 //   //-- Note: The 0btag LSB, LDP is assumed to be 100% QCD.

 //   rv_mu_qcd_lsb_0b_ldp  -> setVal( initialval_qcd_lsb_0b_ldp ) ;  //-- this is a starting value only.






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
      rv_mu_susymc_sig_sl_e   = new RooRealVar( "mu_susymc_sig_sl_e"  , "mu_susymc_sig_sl_e"  , 0.0, 100000. ) ;
      rv_mu_susymc_sig_sl_mu   = new RooRealVar( "mu_susymc_sig_sl_mu"  , "mu_susymc_sig_sl_mu"  , 0.0, 100000. ) ;
      rv_mu_susymc_sb_sl_e    = new RooRealVar( "mu_susymc_sb_sl_e"   , "mu_susymc_sb_sl_e"   , 0.0, 100000. ) ;
      rv_mu_susymc_sb_sl_mu    = new RooRealVar( "mu_susymc_sb_sl_mu"   , "mu_susymc_sb_sl_mu"   , 0.0, 100000. ) ;
      rv_mu_susymc_sig_ldp  = new RooRealVar( "mu_susymc_sig_ldp" , "mu_susymc_sig_ldp" , 0.0, 100000. ) ;
      rv_mu_susymc_sb_ldp   = new RooRealVar( "mu_susymc_sb_ldp"  , "mu_susymc_sb_ldp"  , 0.0, 100000. ) ;

      rv_mu_susymc_sig     -> setVal( 0.1 ) ;
      rv_mu_susymc_sb      -> setVal( 0. ) ;
      rv_mu_susymc_sig_sl_e  -> setVal( 0. ) ;
      rv_mu_susymc_sig_sl_mu  -> setVal( 0. ) ;
      rv_mu_susymc_sb_sl_e   -> setVal( 0. ) ;
      rv_mu_susymc_sb_sl_mu   -> setVal( 0. ) ;
      rv_mu_susymc_sig_ldp -> setVal( 0. ) ;
      rv_mu_susymc_sb_ldp  -> setVal( 0. ) ;

      rv_mu_susymc_sig     -> setConstant(kTRUE) ;
      rv_mu_susymc_sb      -> setConstant(kTRUE) ;
      rv_mu_susymc_sig_sl_e  -> setConstant(kTRUE) ;
      rv_mu_susymc_sig_sl_mu  -> setConstant(kTRUE) ;
      rv_mu_susymc_sb_sl_e   -> setConstant(kTRUE) ;
      rv_mu_susymc_sb_sl_mu   -> setConstant(kTRUE) ;
      rv_mu_susymc_sig_ldp -> setConstant(kTRUE) ;
      rv_mu_susymc_sb_ldp  -> setConstant(kTRUE) ;



     //-- SIG, LDP

      rv_mu_ttbarmc_sig_ldp   = new RooRealVar( "mu_ttbarmc_sig_ldp" ,"mu_ttbarmc_sig_ldp" , 1e-9, 1000. ) ;
      rv_mu_WJmc_sig_ldp      = new RooRealVar( "mu_WJmc_sig_ldp"    ,"mu_WJmc_sig_ldp"    , 1e-9, 1000. ) ;
      rv_mu_Znnmc_sig_ldp     = new RooRealVar( "mu_Znnmc_sig_ldp"   ,"mu_Znnmc_sig_ldp"   , 1e-9, 1000. ) ;
      rv_mu_Ewomc_sig_ldp     = new RooRealVar( "mu_Ewomc_sig_ldp"   ,"mu_Ewomc_sig_ldp"   , 1e-9, 1000. ) ;

      rv_mu_ttbarmc_sig_ldp  -> setVal( Nttbarmc_sig_ldp ) ;
      rv_mu_WJmc_sig_ldp     -> setVal( NWJmc_sig_ldp ) ;
      rv_mu_Znnmc_sig_ldp    -> setVal( NZnnmc_sig_ldp ) ;
      rv_mu_Ewomc_sig_ldp    -> setVal( NEwomc_sig_ldp ) ;

      rv_mu_ttbarmc_sig_ldp  -> setConstant( kTRUE ) ;
      rv_mu_WJmc_sig_ldp     -> setConstant( kTRUE ) ;
      rv_mu_Znnmc_sig_ldp    -> setConstant( kTRUE ) ;
      rv_mu_Ewomc_sig_ldp    -> setConstant( kTRUE ) ;


     //-- SB, LDP

      rv_mu_ttbarmc_sb_ldp   = new RooRealVar( "mu_ttbarmc_sb_ldp" ,"mu_ttbarmc_sb_ldp" , 1e-9, 1000. ) ;
      rv_mu_WJmc_sb_ldp      = new RooRealVar( "mu_WJmc_sb_ldp"    ,"mu_WJmc_sb_ldp"    , 1e-9, 1000. ) ;
      rv_mu_Znnmc_sb_ldp     = new RooRealVar( "mu_Znnmc_sb_ldp"   ,"mu_Znnmc_sb_ldp"   , 1e-9, 1000. ) ;
      rv_mu_Ewomc_sb_ldp     = new RooRealVar( "mu_Ewomc_sb_ldp"   ,"mu_Ewomc_sb_ldp"   , 1e-9, 1000. ) ;

      rv_mu_ttbarmc_sb_ldp  -> setVal( Nttbarmc_sb_ldp ) ;
      rv_mu_WJmc_sb_ldp     -> setVal( NWJmc_sb_ldp ) ;
      rv_mu_Znnmc_sb_ldp    -> setVal( NZnnmc_sb_ldp ) ;
      rv_mu_Ewomc_sb_ldp    -> setVal( NEwomc_sb_ldp ) ;

      rv_mu_ttbarmc_sb_ldp  -> setConstant( kTRUE ) ;
      rv_mu_WJmc_sb_ldp     -> setConstant( kTRUE ) ;
      rv_mu_Znnmc_sb_ldp    -> setConstant( kTRUE ) ;
      rv_mu_Ewomc_sb_ldp    -> setConstant( kTRUE ) ;


     //-- SIG

      rv_mu_Ewomc_sig     = new RooRealVar( "mu_Ewomc_sig"   ,"mu_Ewomc_sig"   , 1e-9, 1000. ) ;
      rv_mu_Ewomc_sig    -> setVal( NEwomc_sig ) ;
      rv_mu_Ewomc_sig    -> setConstant( kTRUE ) ;

     //-- SB

      rv_mu_Ewomc_sb     = new RooRealVar( "mu_Ewomc_sb"   ,"mu_Ewomc_sb"   , 1e-9, 1000. ) ;
      rv_mu_Ewomc_sb    -> setVal( NEwomc_sb ) ;
      rv_mu_Ewomc_sb    -> setConstant( kTRUE ) ;





    //+++++++ Non-Gaussian constraints ++++++++++++++++++++++++++++++++

      printf(" --- Defining constraints and constant parameters.\n" ) ;

    //_______ Efficiency scale factor.  Applied to SUSY and all MC inputs _______________

    //   August 23, 2011:  switching to correlated log-normal PDFs
    //
    //
    //  double pmin, pmax ;


    //--- mean parameters.
      rv_mean_eff_sf_sig     = new RooRealVar( "mean_eff_sf_sig"    , "mean_eff_sf_sig", 0., 10. ) ;
      rv_mean_eff_sf_sb      = new RooRealVar( "mean_eff_sf_sb"     , "mean_eff_sf_sb", 0., 10. ) ;
      rv_mean_eff_sf_sig_sl_e  = new RooRealVar( "mean_eff_sf_sig_sl_e" , "mean_eff_sf_sig_sl_e", 0., 10. ) ;
      rv_mean_eff_sf_sig_sl_mu  = new RooRealVar( "mean_eff_sf_sig_sl_mu" , "mean_eff_sf_sig_sl_mu", 0., 10. ) ;
      rv_mean_eff_sf_sb_sl_e   = new RooRealVar( "mean_eff_sf_sb_sl_e"  , "mean_eff_sf_sb_sl_e", 0., 10. ) ;
      rv_mean_eff_sf_sb_sl_mu   = new RooRealVar( "mean_eff_sf_sb_sl_mu"  , "mean_eff_sf_sb_sl_mu", 0., 10. ) ;
      rv_mean_eff_sf_sig_ldp = new RooRealVar( "mean_eff_sf_sig_ldp", "mean_eff_sf_sig_ldp", 0., 10. ) ;
      rv_mean_eff_sf_sb_ldp  = new RooRealVar( "mean_eff_sf_sb_ldp" , "mean_eff_sf_sb_ldp", 0., 10. ) ;

    //--- width parameters.
      rv_width_eff_sf_sig     = new RooRealVar( "width_eff_sf_sig"    , "width_eff_sf_sig", 0., 10. ) ;
      rv_width_eff_sf_sb      = new RooRealVar( "width_eff_sf_sb"     , "width_eff_sf_sb", 0., 10. ) ;
      rv_width_eff_sf_sig_sl_e  = new RooRealVar( "width_eff_sf_sig_sl_e" , "width_eff_sf_sig_sl_e", 0., 10. ) ;
      rv_width_eff_sf_sig_sl_mu  = new RooRealVar( "width_eff_sf_sig_sl_mu" , "width_eff_sf_sig_sl_mu", 0., 10. ) ;
      rv_width_eff_sf_sb_sl_e   = new RooRealVar( "width_eff_sf_sb_sl_e"  , "width_eff_sf_sb_sl_e", 0., 10. ) ;
      rv_width_eff_sf_sb_sl_mu   = new RooRealVar( "width_eff_sf_sb_sl_mu"  , "width_eff_sf_sb_sl_mu", 0., 10. ) ;
      rv_width_eff_sf_sig_ldp = new RooRealVar( "width_eff_sf_sig_ldp", "width_eff_sf_sig_ldp", 0., 10. ) ;
      rv_width_eff_sf_sb_ldp  = new RooRealVar( "width_eff_sf_sb_ldp" , "width_eff_sf_sb_ldp", 0., 10. ) ;


      rv_mean_eff_sf_sig     -> setVal( 1.00 ) ;
      rv_mean_eff_sf_sb      -> setVal( 1.00 ) ;
      rv_mean_eff_sf_sig_sl_e  -> setVal( 1.00 ) ;
      rv_mean_eff_sf_sig_sl_mu  -> setVal( 1.00 ) ;
      rv_mean_eff_sf_sb_sl_e   -> setVal( 1.00 ) ;
      rv_mean_eff_sf_sb_sl_mu   -> setVal( 1.00 ) ;
      rv_mean_eff_sf_sig_ldp -> setVal( 1.00 ) ;
      rv_mean_eff_sf_sb_ldp  -> setVal( 1.00 ) ;

      rv_mean_eff_sf_sig     -> setConstant( kTRUE ) ;
      rv_mean_eff_sf_sb      -> setConstant( kTRUE ) ;
      rv_mean_eff_sf_sig_sl_e  -> setConstant( kTRUE ) ;
      rv_mean_eff_sf_sig_sl_mu  -> setConstant( kTRUE ) ;
      rv_mean_eff_sf_sb_sl_e   -> setConstant( kTRUE ) ;
      rv_mean_eff_sf_sb_sl_mu   -> setConstant( kTRUE ) ;
      rv_mean_eff_sf_sig_ldp -> setConstant( kTRUE ) ;
      rv_mean_eff_sf_sb_ldp  -> setConstant( kTRUE ) ;


      //--- Initialize all width parameters to 15%.  They will be reset in the susy scan.
      rv_width_eff_sf_sig     -> setVal( 0.15 ) ;
      rv_width_eff_sf_sb      -> setVal( 0.15 ) ;
      rv_width_eff_sf_sig_sl_e  -> setVal( 0.15 ) ;
      rv_width_eff_sf_sig_sl_mu  -> setVal( 0.15 ) ;
      rv_width_eff_sf_sb_sl_e   -> setVal( 0.15 ) ;
      rv_width_eff_sf_sb_sl_mu   -> setVal( 0.15 ) ;
      rv_width_eff_sf_sig_ldp -> setVal( 0.15 ) ;
      rv_width_eff_sf_sb_ldp  -> setVal( 0.15 ) ;

      rv_width_eff_sf_sig     -> setConstant( kTRUE ) ;
      rv_width_eff_sf_sb      -> setConstant( kTRUE ) ;
      rv_width_eff_sf_sig_sl_e  -> setConstant( kTRUE ) ;
      rv_width_eff_sf_sig_sl_mu  -> setConstant( kTRUE ) ;
      rv_width_eff_sf_sb_sl_e   -> setConstant( kTRUE ) ;
      rv_width_eff_sf_sb_sl_mu   -> setConstant( kTRUE ) ;
      rv_width_eff_sf_sig_ldp -> setConstant( kTRUE ) ;
      rv_width_eff_sf_sb_ldp  -> setConstant( kTRUE ) ;


   //
   // Owen : Sept 7, 2011: need to set these here so that it gets into the workspace.
   //
   //++++++++++
      setSusyScanPoint( inputScanFile,  m0,  m12,  isT1bbbb,  t1bbbbXsec ) ;
   //++++++++++





    //--- Systematics and other nuisance parameters

      //  Gamma Distributed Variables


      //char formula[1024];

      double alpha,beta;
      double k,theta;

       // gammaModeTransform(Rlsb_passfail,Rlsb_passfail_err,k,theta);
       // //RooRealVar rrv_Rlsb_passfail ("Rlsb_passfail","Rlsb_passfail",Rlsb_passfail,0,1e5);
       // //RooRealVar Rlsb_passfail_k ("Rlsb_passfail_k", "Rlsb_passfail_k", k);
       // //RooRealVar Rlsb_passfail_theta ("Rlsb_passfail_theta", "Rlsb_passfail_theta", theta);
       // //RooGammaPdf pdf_Rlsb_passfail ("pdf_Rlsb_passfail" , "pdf_Rlsb_passfail", rrv_Rlsb_passfail, Rlsb_passfail_k, Rlsb_passfail_theta, RooConst(0));
       // 
       // //Rlsb_passfail_k.setConstant();
       // //Rlsb_passfail_theta.setConstant();
       // //allNonPoissonNuisances.add (rrv_Rlsb_passfail);
       // //if(constantNonPoisson) rrv_Rlsb_passfail.setConstant();
       // //gammaNuisancePdfs.add (pdf_Rlsb_passfail);
       // 
       // RooRealVar rrv_Rlsb_passfail ("Rlsb_passfail","Rlsb_passfail",Rlsb_passfail,0,1e5);
       // allNonPoissonNuisances.add (rrv_Rlsb_passfail);
       // RooRealVar Rlsb_passfail_invScale("Rlsb_passfail_invScale","Rlsb_passfail_invScale",1.0/theta);
       // Rlsb_passfail_invScale.setConstant();
       // RooProduct mu_comp_Rlsb_passfail("mu_comp_Rlsb_passfail","mu_comp_Rlsb_passfail",RooArgList(rrv_Rlsb_passfail,Rlsb_passfail_invScale));
       // RooRealVar N_comp_Rlsb_passfail ("N_comp_Rlsb_passfail", "N_comp_Rlsb_passfail", k-1,0,1e5);
       // observedParametersList.add( N_comp_Rlsb_passfail );
       // //RooRealVar mu_comp_Rlsb_passfail ("mu_comp_Rlsb_passfail", "mu_comp_Rlsb_passfail", k-1,1e-9,1e5);
       // //allPoissonNuisances.add(mu_comp_Rlsb_passfail);
       // RooPoisson pdf_Rlsb_passfail ("pdf_Rlsb_passfail" , "pdf_Rlsb_passfail", N_comp_Rlsb_passfail, mu_comp_Rlsb_passfail);
       // allNuisancePdfs.add(pdf_Rlsb_passfail);

      betaPrimeModeTransform(Rlsb_passfail,Rlsb_passfail_err,alpha,beta);

      RooRealVar N_pass_Rlsb_passfail ("N_pass_Rlsb_passfail", "N_pass_Rlsb_passfail", alpha-1,0,1e5);
      observedParametersList.add( N_pass_Rlsb_passfail );
      RooRealVar mu_pass_Rlsb_passfail ("mu_pass_Rlsb_passfail", "mu_pass_Rlsb_passfail", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_Rlsb_passfail);
      RooPoisson pdf_pass_Rlsb_passfail ("pdf_pass_Rlsb_passfail" , "pdf_pass_Rlsb_passfail", N_pass_Rlsb_passfail, mu_pass_Rlsb_passfail);
      allNuisancePdfs.add(pdf_pass_Rlsb_passfail);
      RooRealVar N_fail_Rlsb_passfail ("N_fail_Rlsb_passfail", "N_fail_Rlsb_passfail", beta-1,0,1e5);
      observedParametersList.add( N_fail_Rlsb_passfail );
      RooRealVar mu_fail_Rlsb_passfail ("mu_fail_Rlsb_passfail", "mu_fail_Rlsb_passfail", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_Rlsb_passfail);
      RooPoisson pdf_fail_Rlsb_passfail ("pdf_fail_Rlsb_passfail" , "pdf_fail_Rlsb_passfail", N_fail_Rlsb_passfail, mu_fail_Rlsb_passfail);
      allNuisancePdfs.add(pdf_fail_Rlsb_passfail);
      
      RooFormulaVar rrv_Rlsb_passfail ("Rlsb_passfail","@0/(@1)",RooArgList(mu_pass_Rlsb_passfail,mu_fail_Rlsb_passfail));

      //RooRealVar Rlsb_passfail_scale("Rlsb_passfail_scale","Rlsb_passfail_scale",theta);
      //Rlsb_passfail_scale.setConstant();
      //RooProduct rp_Rlsb_passfail ("Rlsb_passfail","Rlsb_passfail",RooArgList(mu_comp_Rlsb_passfail,Rlsb_passfail_scale));
      



    //--- Nov 24, 2011:  sf_ee and sf_mm are now derived from a common function.

      double sigma_avg = 0.5*(sf_ee_err + sf_mm_err);
      double mu_avg = 0.5*(sf_ee + sf_mm);

      double alpha_sf_ee,alpha_sf_mm,beta_sf_ee,beta_sf_mm;

      betaPrimeModeTransform( sf_ee       , sf_ee_err       , alpha_sf_ee       , beta_sf_ee       );
      betaPrimeModeTransform( sf_mm       , sf_mm_err       , alpha_sf_mm       , beta_sf_mm       );

      alpha = max(alpha_sf_ee , alpha_sf_mm);
      beta  = max( beta_sf_ee ,  beta_sf_mm);

      RooRealVar N_pass_scale_sf_ee       ("N_pass_scale_sf_ee"       , "N_pass_scale_sf_ee"       , (alpha_sf_ee-1) / (alpha-1) );
      RooRealVar N_pass_scale_sf_mm       ("N_pass_scale_sf_mm"       , "N_pass_scale_sf_mm"       , (alpha_sf_mm-1) / (alpha-1) );
      RooRealVar N_fail_scale_sf_ee       ("N_fail_scale_sf_ee"       , "N_fail_scale_sf_ee"       , ( beta_sf_ee-1) / ( beta-1) );
      RooRealVar N_fail_scale_sf_mm       ("N_fail_scale_sf_mm"       , "N_fail_scale_sf_mm"       , ( beta_sf_mm-1) / ( beta-1) );

      N_pass_scale_sf_ee.setConstant();
      N_pass_scale_sf_mm.setConstant();
      N_fail_scale_sf_ee.setConstant();
      N_fail_scale_sf_mm.setConstant();

      RooRealVar N_pass_sf_ll ("N_pass_sf_ll", "N_pass_sf_ll", alpha-1,0,1e5);
      observedParametersList.add( N_pass_sf_ll );
      RooRealVar mu_pass_sf_ll ("mu_pass_sf_ll", "mu_pass_sf_ll", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_sf_ll);
      RooPoisson pdf_pass_sf_ll ("pdf_pass_sf_ll" , "pdf_pass_sf_ll", N_pass_sf_ll, mu_pass_sf_ll);
      allNuisancePdfs.add(pdf_pass_sf_ll);
      RooRealVar N_fail_sf_ll ("N_fail_sf_ll", "N_fail_sf_ll", beta-1,0,1e5);
      observedParametersList.add( N_fail_sf_ll );
      RooRealVar mu_fail_sf_ll ("mu_fail_sf_ll", "mu_fail_sf_ll", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_sf_ll);
      RooPoisson pdf_fail_sf_ll ("pdf_fail_sf_ll" , "pdf_fail_sf_ll", N_fail_sf_ll, mu_fail_sf_ll);
      allNuisancePdfs.add(pdf_fail_sf_ll);

      RooProduct mu_pass_sf_ee       ("mu_pass_sf_ee"       , "mu_pass_sf_ee"       , RooArgSet(N_pass_scale_sf_ee       , mu_pass_sf_ll));
      RooProduct mu_fail_sf_ee       ("mu_fail_sf_ee"       , "mu_fail_sf_ee"       , RooArgSet(N_fail_scale_sf_ee       , mu_fail_sf_ll));

      RooProduct mu_pass_sf_mm       ("mu_pass_sf_mm"       , "mu_pass_sf_mm"       , RooArgSet(N_pass_scale_sf_mm       , mu_pass_sf_ll));
      RooProduct mu_fail_sf_mm       ("mu_fail_sf_mm"       , "mu_fail_sf_mm"       , RooArgSet(N_fail_scale_sf_mm       , mu_fail_sf_ll));
      
      cout << "sf_ll section -- calculation of mu_pass" << endl;
      cout << "sf_ll section -- sf_ee       has mu_pass of: " << mu_pass_sf_ee       .getVal() << endl;
      cout << "sf_ll section -- sf_mm       has mu_pass of: " << mu_pass_sf_mm       .getVal() << endl;

      cout << "sf_ll section -- calculation of mu_fail" << endl;
      cout << "sf_ll section -- sf_ee       has mu_fail of: " << mu_fail_sf_ee       .getVal() << endl;
      cout << "sf_ll section -- sf_mm       has mu_fail of: " << mu_fail_sf_mm       .getVal() << endl;

      RooFormulaVar fv_sf_ee       ("sf_ee"         , "@0/(@1)" , RooArgList( mu_pass_sf_ee       , mu_fail_sf_mm       ));
      RooFormulaVar fv_sf_mm       ("sf_mm"         , "@0/(@1)" , RooArgList( mu_pass_sf_mm       , mu_fail_sf_mm       ));

      cout << "sf_ll section -- calculation of efficiency" << endl;
      cout << "sf_ll section -- sf_ee       has efficiency of: " << fv_sf_ee       .getVal() << endl;
      cout << "sf_ll section -- sf_mm       has efficiency of: " << fv_sf_mm       .getVal() << endl;

      //gammaModeTransform(mu_avg,sigma_avg,k,theta);
      //
      ////RooRealVar rrv_sf_ll ("sf_ll","sf_ll",mu_avg,0,1e5);
      ////RooRealVar sf_ll_k ("sf_ll_k", "sf_ll_k", k);
      ////RooRealVar sf_ll_theta ("sf_ll_theta", "sf_ll_theta", theta);
      ////RooGammaPdf pdf_sf_ll ("pdf_sf_ll" , "pdf_sf_ll", rrv_sf_ll, sf_ll_k, sf_ll_theta, RooConst(0));
      ////sf_ll_k.setConstant();
      ////sf_ll_theta.setConstant();
      ////allNonPoissonNuisances.add (rrv_sf_ll);
      ////if(constantNonPoisson) rrv_sf_ll.setConstant();
      ////gammaNuisancePdfs.add (pdf_sf_ll);
      //
      //RooRealVar rrv_sf_ll ("sf_ll","sf_ll",mu_avg,0,1e5);
      //allNonPoissonNuisances.add (rrv_sf_ll);
      //RooRealVar sf_ll_invScale("sf_ll_invScale","sf_ll_invScale",1.0/theta);
      //sf_ll_invScale.setConstant();
      //RooProduct mu_comp_sf_ll("mu_comp_sf_ll","mu_comp_sf_ll",RooArgList(rrv_sf_ll,sf_ll_invScale));
      //RooRealVar N_comp_sf_ll ("N_comp_sf_ll", "N_comp_sf_ll", k-1,0,1e5);
      //observedParametersList.add( N_comp_sf_ll );
      ////RooRealVar mu_comp_sf_ll ("mu_comp_sf_ll", "mu_comp_sf_ll", k-1,1e-9,1e5);
      ////allPoissonNuisances.add(mu_comp_sf_ll);
      //RooPoisson pdf_sf_ll ("pdf_sf_ll" , "pdf_sf_ll", N_comp_sf_ll, mu_comp_sf_ll);
      //allNuisancePdfs.add(pdf_sf_ll);
      //
      ////RooRealVar sf_ll_scale("sf_ll_scale","sf_ll_scale",theta);
      ////sf_ll_scale.setConstant();
      ////RooProduct rp_sf_ll ("sf_ll","sf_ll",RooArgList(mu_comp_sf_ll,sf_ll_scale));
      //
      //gammaScalingFunctor alphaFinding(sigma_avg,1.);
      //
      //TF1 alphaFindingFunction("alphaFindingFunction",&alphaFinding,&gammaScalingFunctor::sigmaMeanIntersect,0,1,0,"alphaFindingFunction","alphaFindingFunction");
      //WrappedTF1 WrappedAlphaFindingFunction(alphaFindingFunction);
      //ROOT::Math::Roots::Brent alphaRoot;
      //alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*max(sf_mm_err,sf_ee_err)/sigma_avg);
      //alphaFinding.setDesiredSigma(sf_ee_err);
      //alphaRoot.Solve();
      //
      //RooRealVar sf_ee_alpha("sf_ee_alpha","sf_ee_alpha",alphaRoot.Root());
      //sf_ee_alpha.setConstant();
      //
      //alphaFinding.setDesiredSigma(sf_mm_err);
      //alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*max(sf_mm_err,sf_ee_err)/sigma_avg);
      //alphaRoot.Solve();
      //RooRealVar sf_mm_alpha("sf_mm_alpha","sf_mm_alpha",alphaRoot.Root());
      //sf_mm_alpha.setConstant();
      //
      //cout << "sf_ll section -- calculation of power law" << endl;
      //cout << "sf_ll section -- sf_ee has alpha of: " << sf_ee_alpha.getVal() << endl;
      //cout << "sf_ll section -- sf_mm has alpha of: " << sf_mm_alpha.getVal() << endl;
      //
      //RooFormulaVar fv_sf_ee ("sf_ee", "pow(@0,@1)" , RooArgList(rrv_sf_ll,sf_ee_alpha));
      //RooFormulaVar fv_sf_mm ("sf_mm", "pow(@0,@1)" , RooArgList(rrv_sf_ll,sf_mm_alpha));

        //  gammaModeTransform(sf_mc,sf_mc_err,k,theta);
        //  //RooRealVar rrv_sf_mc ("sf_mc","sf_mc",sf_mc,0,1e5);
        //  //RooRealVar sf_mc_k ("sf_mc_k", "sf_mc_k", k);
        //  //RooRealVar sf_mc_theta ("sf_mc_theta", "sf_mc_theta", theta);
        //  //RooGammaPdf pdf_sf_mc ("pdf_sf_mc" , "pdf_sf_mc", rrv_sf_mc, sf_mc_k, sf_mc_theta, RooConst(0));
        //  //sf_mc_k.setConstant();
        //  //sf_mc_theta.setConstant();
        //  //allNonPoissonNuisances.add (rrv_sf_mc);
        //  //if(constantNonPoisson) rrv_sf_mc.setConstant();
        //  //gammaNuisancePdfs.add (pdf_sf_mc);
        //  
        //  RooRealVar rrv_sf_mc ("sf_mc","sf_mc",sf_mc,0,1e5);
        //  allNonPoissonNuisances.add (rrv_sf_mc);
        //  RooRealVar sf_mc_invScale("sf_mc_invScale","sf_mc_invScale",1.0/theta);
        //  sf_mc_invScale.setConstant();
        //  RooProduct mu_comp_sf_mc("mu_comp_sf_mc","mu_comp_sf_mc",RooArgList(rrv_sf_mc,sf_mc_invScale));
        //  RooRealVar N_comp_sf_mc ("N_comp_sf_mc", "N_comp_sf_mc", k-1,0,1e5);
        //  observedParametersList.add( N_comp_sf_mc );
        //  //RooRealVar mu_comp_sf_mc ("mu_comp_sf_mc", "mu_comp_sf_mc", k-1,1e-9,1e5);
        //  //allPoissonNuisances.add(mu_comp_sf_mc);
        //  RooPoisson pdf_sf_mc ("pdf_sf_mc" , "pdf_sf_mc", N_comp_sf_mc, mu_comp_sf_mc);
        //  allNuisancePdfs.add(pdf_sf_mc);
        //  
        //  //RooRealVar sf_mc_scale("sf_mc_scale","sf_mc_scale",theta);
        //  //sf_mc_scale.setConstant();
        //  //RooProduct rp_sf_mc ("sf_mc","sf_mc",RooArgList(mu_comp_sf_mc,sf_mc_scale));

      betaPrimeModeTransform(sf_mc,sf_mc_err,alpha,beta);

      RooRealVar N_pass_sf_mc ("N_pass_sf_mc", "N_pass_sf_mc", alpha-1,0,1e5);
      observedParametersList.add( N_pass_sf_mc );
      RooRealVar mu_pass_sf_mc ("mu_pass_sf_mc", "mu_pass_sf_mc", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_sf_mc);
      RooPoisson pdf_pass_sf_mc ("pdf_pass_sf_mc" , "pdf_pass_sf_mc", N_pass_sf_mc, mu_pass_sf_mc);
      allNuisancePdfs.add(pdf_pass_sf_mc);
      RooRealVar N_fail_sf_mc ("N_fail_sf_mc", "N_fail_sf_mc", beta-1,0,1e5);
      observedParametersList.add( N_fail_sf_mc );
      RooRealVar mu_fail_sf_mc ("mu_fail_sf_mc", "mu_fail_sf_mc", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_sf_mc);
      RooPoisson pdf_fail_sf_mc ("pdf_fail_sf_mc" , "pdf_fail_sf_mc", N_fail_sf_mc, mu_fail_sf_mc);
      allNuisancePdfs.add(pdf_fail_sf_mc);
      
      RooFormulaVar rrv_sf_mc ("sf_mc","@0/(@1)",RooArgList(mu_pass_sf_mc,mu_fail_sf_mc));

        //  gammaModeTransform(sf_qcd_sb,sf_qcd_sb_err,k,theta);
        //  //RooRealVar rrv_sf_qcd_sb ("sf_qcd_sb","sf_qcd_sb",sf_qcd_sb,0,1e5);
        //  //RooRealVar sf_qcd_sb_k ("sf_qcd_sb_k", "sf_qcd_sb_k", k);
        //  //RooRealVar sf_qcd_sb_theta ("sf_qcd_sb_theta", "sf_qcd_sb_theta", theta);
        //  //RooGammaPdf pdf_sf_qcd_sb ("pdf_sf_qcd_sb" , "pdf_sf_qcd_sb", rrv_sf_qcd_sb, sf_qcd_sb_k, sf_qcd_sb_theta, RooConst(0));
        //  //sf_qcd_sb_k.setConstant();
        //  //sf_qcd_sb_theta.setConstant();
        //  //allNonPoissonNuisances.add (rrv_sf_qcd_sb);
        //  //if(constantNonPoisson) rrv_sf_qcd_sb.setConstant();
        //  //gammaNuisancePdfs.add (pdf_sf_qcd_sb);
        //  
        //  RooRealVar rrv_sf_qcd_sb ("sf_qcd_sb","sf_qcd_sb",sf_qcd_sb,0,1e5);
        //  allNonPoissonNuisances.add (rrv_sf_qcd_sb);
        //  RooRealVar sf_qcd_sb_invScale("sf_qcd_sb_invScale","sf_qcd_sb_invScale",1.0/theta);
        //  sf_qcd_sb_invScale.setConstant();
        //  RooProduct mu_comp_sf_qcd_sb("mu_comp_sf_qcd_sb","mu_comp_sf_qcd_sb",RooArgList(rrv_sf_qcd_sb,sf_qcd_sb_invScale));
        //  RooRealVar N_comp_sf_qcd_sb ("N_comp_sf_qcd_sb", "N_comp_sf_qcd_sb", k-1,0,1e5);
        //  observedParametersList.add( N_comp_sf_qcd_sb );
        //  //RooRealVar mu_comp_sf_qcd_sb ("mu_comp_sf_qcd_sb", "mu_comp_sf_qcd_sb", k-1,1e-9,1e5);
        //  //allPoissonNuisances.add(mu_comp_sf_qcd_sb);
        //  RooPoisson pdf_sf_qcd_sb ("pdf_sf_qcd_sb" , "pdf_sf_qcd_sb", N_comp_sf_qcd_sb, mu_comp_sf_qcd_sb);
        //  allNuisancePdfs.add(pdf_sf_qcd_sb);
        //  
        //  //RooRealVar sf_qcd_sb_scale("sf_qcd_sb_scale","sf_qcd_sb_scale",theta);
        //  //sf_qcd_sb_scale.setConstant();
        //  //RooProduct rp_sf_qcd_sb ("sf_qcd_sb","sf_qcd_sb",RooArgList(mu_comp_sf_qcd_sb,sf_qcd_sb_scale));

      betaPrimeModeTransform(sf_qcd_sb,sf_qcd_sb_err,alpha,beta);

      RooRealVar N_pass_sf_qcd_sb ("N_pass_sf_qcd_sb", "N_pass_sf_qcd_sb", alpha-1,0,1e5);
      observedParametersList.add( N_pass_sf_qcd_sb );
      RooRealVar mu_pass_sf_qcd_sb ("mu_pass_sf_qcd_sb", "mu_pass_sf_qcd_sb", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_sf_qcd_sb);
      RooPoisson pdf_pass_sf_qcd_sb ("pdf_pass_sf_qcd_sb" , "pdf_pass_sf_qcd_sb", N_pass_sf_qcd_sb, mu_pass_sf_qcd_sb);
      allNuisancePdfs.add(pdf_pass_sf_qcd_sb);
      RooRealVar N_fail_sf_qcd_sb ("N_fail_sf_qcd_sb", "N_fail_sf_qcd_sb", beta-1,0,1e5);
      observedParametersList.add( N_fail_sf_qcd_sb );
      RooRealVar mu_fail_sf_qcd_sb ("mu_fail_sf_qcd_sb", "mu_fail_sf_qcd_sb", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_sf_qcd_sb);
      RooPoisson pdf_fail_sf_qcd_sb ("pdf_fail_sf_qcd_sb" , "pdf_fail_sf_qcd_sb", N_fail_sf_qcd_sb, mu_fail_sf_qcd_sb);
      allNuisancePdfs.add(pdf_fail_sf_qcd_sb);
      
      RooFormulaVar rrv_sf_qcd_sb ("sf_qcd_sb","@0/(@1)",RooArgList(mu_pass_sf_qcd_sb,mu_fail_sf_qcd_sb));

        //  gammaModeTransform(sf_qcd_sig,sf_qcd_sig_err,k,theta);
        //  //RooRealVar rrv_sf_qcd_sig ("sf_qcd_sig","sf_qcd_sig",sf_qcd_sig,0,1e5);
        //  //RooRealVar sf_qcd_sig_k ("sf_qcd_sig_k", "sf_qcd_sig_k", k);
        //  //RooRealVar sf_qcd_sig_theta ("sf_qcd_sig_theta", "sf_qcd_sig_theta", theta);
        //  //RooGammaPdf pdf_sf_qcd_sig ("pdf_sf_qcd_sig" , "pdf_sf_qcd_sig", rrv_sf_qcd_sig, sf_qcd_sig_k, sf_qcd_sig_theta, RooConst(0));
        //  //sf_qcd_sig_k.setConstant();
        //  //sf_qcd_sig_theta.setConstant();
        //  //allNonPoissonNuisances.add (rrv_sf_qcd_sig);
        //  //if(constantNonPoisson) rrv_sf_qcd_sig.setConstant();
        //  //gammaNuisancePdfs.add (pdf_sf_qcd_sig);
        //  
        //  RooRealVar rrv_sf_qcd_sig ("sf_qcd_sig","sf_qcd_sig",sf_qcd_sig,0,1e5);
        //  allNonPoissonNuisances.add (rrv_sf_qcd_sig);
        //  RooRealVar sf_qcd_sig_invScale("sf_qcd_sig_invScale","sf_qcd_sig_invScale",1.0/theta);
        //  sf_qcd_sig_invScale.setConstant();
        //  RooProduct mu_comp_sf_qcd_sig("mu_comp_sf_qcd_sig","mu_comp_sf_qcd_sig",RooArgList(rrv_sf_qcd_sig,sf_qcd_sig_invScale));
        //  RooRealVar N_comp_sf_qcd_sig ("N_comp_sf_qcd_sig", "N_comp_sf_qcd_sig", k-1,0,1e5);
        //  observedParametersList.add( N_comp_sf_qcd_sig );
        //  //RooRealVar mu_comp_sf_qcd_sig ("mu_comp_sf_qcd_sig", "mu_comp_sf_qcd_sig", k-1,1e-9,1e5);
        //  //allPoissonNuisances.add(mu_comp_sf_qcd_sig);
        //  RooPoisson pdf_sf_qcd_sig ("pdf_sf_qcd_sig" , "pdf_sf_qcd_sig", N_comp_sf_qcd_sig, mu_comp_sf_qcd_sig);
        //  allNuisancePdfs.add(pdf_sf_qcd_sig);
        //  
        //  //RooRealVar sf_qcd_sig_scale("sf_qcd_sig_scale","sf_qcd_sig_scale",theta);
        //  //sf_qcd_sig_scale.setConstant();
        //  //RooProduct rp_sf_qcd_sig ("sf_qcd_sig","sf_qcd_sig",RooArgList(mu_comp_sf_qcd_sig,sf_qcd_sig_scale));

      betaPrimeModeTransform(sf_qcd_sig,sf_qcd_sig_err,alpha,beta);

      RooRealVar N_pass_sf_qcd_sig ("N_pass_sf_qcd_sig", "N_pass_sf_qcd_sig", alpha-1,0,1e5);
      observedParametersList.add( N_pass_sf_qcd_sig );
      RooRealVar mu_pass_sf_qcd_sig ("mu_pass_sf_qcd_sig", "mu_pass_sf_qcd_sig", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_sf_qcd_sig);
      RooPoisson pdf_pass_sf_qcd_sig ("pdf_pass_sf_qcd_sig" , "pdf_pass_sf_qcd_sig", N_pass_sf_qcd_sig, mu_pass_sf_qcd_sig);
      allNuisancePdfs.add(pdf_pass_sf_qcd_sig);
      RooRealVar N_fail_sf_qcd_sig ("N_fail_sf_qcd_sig", "N_fail_sf_qcd_sig", beta-1,0,1e5);
      observedParametersList.add( N_fail_sf_qcd_sig );
      RooRealVar mu_fail_sf_qcd_sig ("mu_fail_sf_qcd_sig", "mu_fail_sf_qcd_sig", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_sf_qcd_sig);
      RooPoisson pdf_fail_sf_qcd_sig ("pdf_fail_sf_qcd_sig" , "pdf_fail_sf_qcd_sig", N_fail_sf_qcd_sig, mu_fail_sf_qcd_sig);
      allNuisancePdfs.add(pdf_fail_sf_qcd_sig);
      
      RooFormulaVar rrv_sf_qcd_sig ("sf_qcd_sig","@0/(@1)",RooArgList(mu_pass_sf_qcd_sig,mu_fail_sf_qcd_sig));

        //  gammaModeTransform(sf_ttwj_sig,sf_ttwj_sig_err,k,theta);
        //  //RooRealVar rrv_sf_ttwj_sig ("sf_ttwj_sig","sf_ttwj_sig",sf_ttwj_sig,0,1e5);
        //  //RooRealVar sf_ttwj_sig_k ("sf_ttwj_sig_k", "sf_ttwj_sig_k", k);
        //  //RooRealVar sf_ttwj_sig_theta ("sf_ttwj_sig_theta", "sf_ttwj_sig_theta", theta);
        //  //RooGammaPdf pdf_sf_ttwj_sig ("pdf_sf_ttwj_sig" , "pdf_sf_ttwj_sig", rrv_sf_ttwj_sig, sf_ttwj_sig_k, sf_ttwj_sig_theta, RooConst(0));
        //  //sf_ttwj_sig_k.setConstant();
        //  //sf_ttwj_sig_theta.setConstant();
        //  //allNonPoissonNuisances.add (rrv_sf_ttwj_sig);
        //  //if(constantNonPoisson) rrv_sf_ttwj_sig.setConstant();
        //  //gammaNuisancePdfs.add (pdf_sf_ttwj_sig);
        //  
        //  RooRealVar rrv_sf_ttwj_sig ("sf_ttwj_sig","sf_ttwj_sig",sf_ttwj_sig,0,1e5);
        //  allNonPoissonNuisances.add (rrv_sf_ttwj_sig);
        //  RooRealVar sf_ttwj_sig_invScale("sf_ttwj_sig_invScale","sf_ttwj_sig_invScale",1.0/theta);
        //  sf_ttwj_sig_invScale.setConstant();
        //  RooProduct mu_comp_sf_ttwj_sig("mu_comp_sf_ttwj_sig","mu_comp_sf_ttwj_sig",RooArgList(rrv_sf_ttwj_sig,sf_ttwj_sig_invScale));
        //  RooRealVar N_comp_sf_ttwj_sig ("N_comp_sf_ttwj_sig", "N_comp_sf_ttwj_sig", k-1,0,1e5);
        //  observedParametersList.add( N_comp_sf_ttwj_sig );
        //  //RooRealVar mu_comp_sf_ttwj_sig ("mu_comp_sf_ttwj_sig", "mu_comp_sf_ttwj_sig", k-1,1e-9,1e5);
        //  //allPoissonNuisances.add(mu_comp_sf_ttwj_sig);
        //  RooPoisson pdf_sf_ttwj_sig ("pdf_sf_ttwj_sig" , "pdf_sf_ttwj_sig", N_comp_sf_ttwj_sig, mu_comp_sf_ttwj_sig);
        //  allNuisancePdfs.add(pdf_sf_ttwj_sig);
        //  
        //  //RooRealVar sf_ttwj_sig_scale("sf_ttwj_sig_scale","sf_ttwj_sig_scale",theta);
        //  //sf_ttwj_sig_scale.setConstant();
        //  //RooProduct rp_sf_ttwj_sig ("sf_ttwj_sig","sf_ttwj_sig",RooArgList(mu_comp_sf_ttwj_sig,sf_ttwj_sig_scale));

      betaPrimeModeTransform(sf_ttwj_sig,sf_ttwj_sig_err,alpha,beta);

      RooRealVar N_pass_sf_ttwj_sig ("N_pass_sf_ttwj_sig", "N_pass_sf_ttwj_sig", alpha-1,0,1e5);
      observedParametersList.add( N_pass_sf_ttwj_sig );
      RooRealVar mu_pass_sf_ttwj_sig ("mu_pass_sf_ttwj_sig", "mu_pass_sf_ttwj_sig", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_sf_ttwj_sig);
      RooPoisson pdf_pass_sf_ttwj_sig ("pdf_pass_sf_ttwj_sig" , "pdf_pass_sf_ttwj_sig", N_pass_sf_ttwj_sig, mu_pass_sf_ttwj_sig);
      allNuisancePdfs.add(pdf_pass_sf_ttwj_sig);
      RooRealVar N_fail_sf_ttwj_sig ("N_fail_sf_ttwj_sig", "N_fail_sf_ttwj_sig", beta-1,0,1e5);
      observedParametersList.add( N_fail_sf_ttwj_sig );
      RooRealVar mu_fail_sf_ttwj_sig ("mu_fail_sf_ttwj_sig", "mu_fail_sf_ttwj_sig", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_sf_ttwj_sig);
      RooPoisson pdf_fail_sf_ttwj_sig ("pdf_fail_sf_ttwj_sig" , "pdf_fail_sf_ttwj_sig", N_fail_sf_ttwj_sig, mu_fail_sf_ttwj_sig);
      allNuisancePdfs.add(pdf_fail_sf_ttwj_sig);
      
      RooFormulaVar rrv_sf_ttwj_sig ("sf_ttwj_sig","@0/(@1)",RooArgList(mu_pass_sf_ttwj_sig,mu_fail_sf_ttwj_sig));

      ////Truncated Gaussians for comparisons
      //
      //RooRealVar Rlsb_passfail_mu ("Rlsb_passfail_mu", "Rlsb_passfail_mu", Rlsb_passfail);
      //RooRealVar Rlsb_passfail_sigma ("Rlsb_passfail_sigma", "Rlsb_passfail_sigma", Rlsb_passfail_err);
      //RooGaussian pdf_normal_Rlsb_passfail ("pdf_Rlsb_passfail" , "pdf_Rlsb_passfail", rrv_Rlsb_passfail, Rlsb_passfail_mu, Rlsb_passfail_sigma);
      //Rlsb_passfail_mu.setConstant();
      //Rlsb_passfail_sigma.setConstant();
      //normalNuisancePdfs.add (pdf_normal_Rlsb_passfail);

    //--- Nov 24, 2011:  sf_ee and sf_mm are now derived from a common function.

      //sigma_avg = 1.;
      //mu_avg = 0.;
      //
      //RooRealVar rrv_normal_sf_ll ("sf_ll","sf_ll",mu_avg,-1e5,1e5);
      //RooRealVar sf_ll_mu ("sf_ll_mu", "sf_ll_mu", mu_avg);
      //RooRealVar sf_ll_sigma ("sf_ll_sigma", "sf_ll_sigma", sigma_avg);
      //RooGaussian pdf_normal_sf_ll ("pdf_sf_ll" , "pdf_sf_ll", rrv_normal_sf_ll, sf_ll_mu, sf_ll_sigma);
      //sf_ll_mu.setConstant();
      //sf_ll_sigma.setConstant();
      //normalNuisances.add (rrv_normal_sf_ll);
      //normalNuisancePdfs.add (pdf_normal_sf_ll);
      //
      //RooRealVar sf_ee_mu ("sf_ee_mu", "sf_ee_mu", sf_ee);
      //RooRealVar sf_mm_mu ("sf_mm_mu", "sf_mm_mu", sf_mm);
      //sf_ee_mu.setConstant();
      //sf_mm_mu.setConstant();
      //RooRealVar sf_ee_sigma ("sf_ee_sigma", "sf_ee_sigma", sf_ee_err);
      //RooRealVar sf_mm_sigma ("sf_mm_sigma", "sf_mm_sigma", sf_mm_err);
      //sf_ee_sigma.setConstant();
      //sf_mm_sigma.setConstant();
      //
      //RooFormulaVar fv_normal_sf_ee("sf_ee", "(@1+@0*@2>=0)*(@1+@0*@2)" , RooArgList(rrv_normal_sf_ll,sf_ee_mu,sf_ee_sigma));
      //RooFormulaVar fv_normal_sf_mm("sf_mm", "(@1+@0*@2>=0)*(@1+@0*@2)" , RooArgList(rrv_normal_sf_ll,sf_mm_mu,sf_mm_sigma));
      //
      //RooRealVar sf_mc_mu ("sf_mc_mu", "sf_mc_mu", sf_mc);
      //RooRealVar sf_mc_sigma ("sf_mc_sigma", "sf_mc_sigma", sf_mc_err);
      //RooGaussian pdf_normal_sf_mc ("pdf_sf_mc" , "pdf_sf_mc", rrv_sf_mc, sf_mc_mu, sf_mc_sigma);
      //sf_mc_mu.setConstant();
      //sf_mc_sigma.setConstant();
      //normalNuisancePdfs.add (pdf_normal_sf_mc);
      //
      //RooRealVar sf_qcd_sb_mu ("sf_qcd_sb_mu", "sf_qcd_sb_mu", sf_qcd_sb);
      //RooRealVar sf_qcd_sb_sigma ("sf_qcd_sb_sigma", "sf_qcd_sb_sigma", sf_qcd_sb_err);
      //RooGaussian pdf_normal_sf_qcd_sb ("pdf_sf_qcd_sb" , "pdf_sf_qcd_sb", rrv_sf_qcd_sb, sf_qcd_sb_mu, sf_qcd_sb_sigma);
      //sf_qcd_sb_mu.setConstant();
      //sf_qcd_sb_sigma.setConstant();
      //normalNuisancePdfs.add (pdf_normal_sf_qcd_sb);
      //
      //RooRealVar sf_qcd_sig_mu ("sf_qcd_sig_mu", "sf_qcd_sig_mu", sf_qcd_sig);
      //RooRealVar sf_qcd_sig_sigma ("sf_qcd_sig_sigma", "sf_qcd_sig_sigma", sf_qcd_sig_err);
      //RooGaussian pdf_normal_sf_qcd_sig ("pdf_sf_qcd_sig" , "pdf_sf_qcd_sig", rrv_sf_qcd_sig, sf_qcd_sig_mu, sf_qcd_sig_sigma);
      //sf_qcd_sig_mu.setConstant();
      //sf_qcd_sig_sigma.setConstant();
      //normalNuisancePdfs.add (pdf_normal_sf_qcd_sig);
      //
      //RooRealVar sf_ttwj_sig_mu ("sf_ttwj_sig_mu", "sf_ttwj_sig_mu", sf_ttwj_sig);
      //RooRealVar sf_ttwj_sig_sigma ("sf_ttwj_sig_sigma", "sf_ttwj_sig_sigma", sf_ttwj_sig_err);
      //RooGaussian pdf_normal_sf_ttwj_sig ("pdf_sf_ttwj_sig" , "pdf_sf_ttwj_sig", rrv_sf_ttwj_sig, sf_ttwj_sig_mu, sf_ttwj_sig_sigma);
      //sf_ttwj_sig_mu.setConstant();
      //sf_ttwj_sig_sigma.setConstant();
      //normalNuisancePdfs.add (pdf_normal_sf_ttwj_sig);


      //Beta Pdf Distributed Variables (efficiencies and acceptances) 

      betaModeTransform(acc_ee_sig_mean,acc_ee_sig_err,alpha,beta);
      //RooRealVar rrv_acc_ee_sig ("acc_ee_sig","acc_ee_sig",acc_ee_sig_mean,0,1);
      //RooRealVar acc_ee_sig_alpha ("acc_ee_sig_alpha", "acc_ee_sig_alpha", alpha);
      //RooRealVar acc_ee_sig_beta ("acc_ee_sig_beta", "acc_ee_sig_beta", beta);
      //RooBetaPdf pdf_acc_ee_sig ("pdf_acc_ee_sig" , "pdf_acc_ee_sig", rrv_acc_ee_sig, acc_ee_sig_alpha, acc_ee_sig_beta);
      //acc_ee_sig_alpha.setConstant();
      //acc_ee_sig_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_acc_ee_sig);
      //if(constantNonPoisson) rrv_acc_ee_sig.setConstant();
      //betaNuisancePdfs.add (pdf_acc_ee_sig);

      //RooRealVar N_pass_acc_ee_sig ("N_pass_acc_ee_sig", "N_pass_acc_ee_sig", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_acc_ee_sig );
      //RooRealVar N_fail_acc_ee_sig ("N_fail_acc_ee_sig", "N_fail_acc_ee_sig", beta-1,0,1e5);
      //observedParametersList.add( N_fail_acc_ee_sig );
      //RooRealVar rrv_acc_ee_sig ("acc_ee_sig","acc_ee_sig",acc_ee_sig_mean,0,1);
      //allNonPoissonNuisances.add (rrv_acc_ee_sig);
      //RooBetaPdfWithPoissonGenerator pdf_acc_ee_sig ("pdf_acc_ee_sig" , "pdf_acc_ee_sig", rrv_acc_ee_sig, N_pass_acc_ee_sig, N_fail_acc_ee_sig);
      //allNuisancePdfs.add(pdf_acc_ee_sig);

      RooRealVar N_pass_acc_ee_sig ("N_pass_acc_ee_sig", "N_pass_acc_ee_sig", alpha-1,0,1e5);
      observedParametersList.add( N_pass_acc_ee_sig );
      RooRealVar mu_pass_acc_ee_sig ("mu_pass_acc_ee_sig", "mu_pass_acc_ee_sig", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_acc_ee_sig);
      RooPoisson pdf_pass_acc_ee_sig ("pdf_pass_acc_ee_sig" , "pdf_pass_acc_ee_sig", N_pass_acc_ee_sig, mu_pass_acc_ee_sig);
      allNuisancePdfs.add(pdf_pass_acc_ee_sig);
      RooRealVar N_fail_acc_ee_sig ("N_fail_acc_ee_sig", "N_fail_acc_ee_sig", beta-1,0,1e5);
      observedParametersList.add( N_fail_acc_ee_sig );
      RooRealVar mu_fail_acc_ee_sig ("mu_fail_acc_ee_sig", "mu_fail_acc_ee_sig", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_acc_ee_sig);
      RooPoisson pdf_fail_acc_ee_sig ("pdf_fail_acc_ee_sig" , "pdf_fail_acc_ee_sig", N_fail_acc_ee_sig, mu_fail_acc_ee_sig);
      allNuisancePdfs.add(pdf_fail_acc_ee_sig);
      
      RooFormulaVar rrv_acc_ee_sig ("acc_ee_sig","@0/(@0+@1)",RooArgList(mu_pass_acc_ee_sig,mu_fail_acc_ee_sig));

      betaModeTransform(acc_ee_sb_mean,acc_ee_sb_err,alpha,beta);
      //RooRealVar N_pass_acc_ee_sb ("N_pass_acc_ee_sb", "N_pass_acc_ee_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_acc_ee_sb );
      //RooRealVar N_fail_acc_ee_sb ("N_fail_acc_ee_sb", "N_fail_acc_ee_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_acc_ee_sb );
      //RooRealVar rrv_acc_ee_sb ("acc_ee_sb","acc_ee_sb",acc_ee_sb_mean,0,1);
      //allNonPoissonNuisances.add (rrv_acc_ee_sb);
      //RooBetaPdfWithPoissonGenerator pdf_acc_ee_sb ("pdf_acc_ee_sb" , "pdf_acc_ee_sb", rrv_acc_ee_sb, N_pass_acc_ee_sb, N_fail_acc_ee_sb);
      //allNuisancePdfs.add(pdf_acc_ee_sb);

      //RooRealVar rrv_acc_ee_sb ("acc_ee_sb","acc_ee_sb",acc_ee_sb_mean,0,1);
      //RooRealVar acc_ee_sb_alpha ("acc_ee_sb_alpha", "acc_ee_sb_alpha", alpha);
      //RooRealVar acc_ee_sb_beta ("acc_ee_sb_beta", "acc_ee_sb_beta", beta);
      //RooBetaPdf pdf_acc_ee_sb ("pdf_acc_ee_sb" , "pdf_acc_ee_sb", rrv_acc_ee_sb, acc_ee_sb_alpha, acc_ee_sb_beta);
      //acc_ee_sb_alpha.setConstant();
      //acc_ee_sb_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_acc_ee_sb);
      //if(constantNonPoisson) rrv_acc_ee_sb.setConstant();
      //betaNuisancePdfs.add (pdf_acc_ee_sb);

      //RooRealVar N_pass_acc_ee_sb ("N_pass_acc_ee_sb", "N_pass_acc_ee_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_acc_ee_sb );
      //RooRealVar N_fail_acc_ee_sb ("N_fail_acc_ee_sb", "N_fail_acc_ee_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_acc_ee_sb );
      //RooAddition mu_acc_ee_sb ("mu_acc_ee_sb", "mu_acc_ee_sb", RooArgList(N_pass_acc_ee_sb,N_fail_acc_ee_sb));
      //RooRealVar rrv_acc_ee_sb ("acc_ee_sb","acc_ee_sb",acc_ee_sb_mean,0,1);
      //allNonPoissonNuisances.add (rrv_acc_ee_sb);
      //RooFormulaVar oneMinus_acc_ee_sb ("oneMinus_acc_ee_sb","1.0-@0",RooArgList(rrv_acc_ee_sb));
      //RooProduct mu_pass_acc_ee_sb ("mu_pass_acc_ee_sb", "mu_pass_acc_ee_sb",RooArgList(rrv_acc_ee_sb,mu_acc_ee_sb));
      //RooProduct mu_fail_acc_ee_sb ("mu_fail_acc_ee_sb", "mu_fail_acc_ee_sb",RooArgList(oneMinus_acc_ee_sb,mu_acc_ee_sb));
      //RooPoissonDummy pdf_pass_acc_ee_sb ("pdf_pass_acc_ee_sb" , "pdf_pass_acc_ee_sb", N_pass_acc_ee_sb, mu_pass_acc_ee_sb);
      //allNuisancePdfs.add(pdf_pass_acc_ee_sb);
      //RooPoissonDummy pdf_fail_acc_ee_sb ("pdf_fail_acc_ee_sb" , "pdf_fail_acc_ee_sb", N_fail_acc_ee_sb, mu_fail_acc_ee_sb);
      //allNuisancePdfs.add(pdf_fail_acc_ee_sb);

      RooRealVar N_pass_acc_ee_sb ("N_pass_acc_ee_sb", "N_pass_acc_ee_sb", alpha-1,0,1e5);
      observedParametersList.add( N_pass_acc_ee_sb );
      RooRealVar mu_pass_acc_ee_sb ("mu_pass_acc_ee_sb", "mu_pass_acc_ee_sb", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_acc_ee_sb);
      RooPoisson pdf_pass_acc_ee_sb ("pdf_pass_acc_ee_sb" , "pdf_pass_acc_ee_sb", N_pass_acc_ee_sb, mu_pass_acc_ee_sb);
      allNuisancePdfs.add(pdf_pass_acc_ee_sb);
      RooRealVar N_fail_acc_ee_sb ("N_fail_acc_ee_sb", "N_fail_acc_ee_sb", beta-1,0,1e5);
      observedParametersList.add( N_fail_acc_ee_sb );
      RooRealVar mu_fail_acc_ee_sb ("mu_fail_acc_ee_sb", "mu_fail_acc_ee_sb", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_acc_ee_sb);
      RooPoisson pdf_fail_acc_ee_sb ("pdf_fail_acc_ee_sb" , "pdf_fail_acc_ee_sb", N_fail_acc_ee_sb, mu_fail_acc_ee_sb);
      allNuisancePdfs.add(pdf_fail_acc_ee_sb);
      
      RooFormulaVar rrv_acc_ee_sb ("acc_ee_sb","@0/(@0+@1)",RooArgList(mu_pass_acc_ee_sb,mu_fail_acc_ee_sb));

      betaModeTransform(acc_mm_sig_mean,acc_mm_sig_err,alpha,beta);
      //RooRealVar N_pass_acc_mm_sig ("N_pass_acc_mm_sig", "N_pass_acc_mm_sig", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_acc_mm_sig );
      //RooRealVar N_fail_acc_mm_sig ("N_fail_acc_mm_sig", "N_fail_acc_mm_sig", beta-1,0,1e5);
      //observedParametersList.add( N_fail_acc_mm_sig );
      //RooRealVar rrv_acc_mm_sig ("acc_mm_sig","acc_mm_sig",acc_mm_sig_mean,0,1);
      //allNonPoissonNuisances.add (rrv_acc_mm_sig);
      //RooBetaPdfWithPoissonGenerator pdf_acc_mm_sig ("pdf_acc_mm_sig" , "pdf_acc_mm_sig", rrv_acc_mm_sig, N_pass_acc_mm_sig, N_fail_acc_mm_sig);
      //allNuisancePdfs.add(pdf_acc_mm_sig);

      //RooRealVar rrv_acc_mm_sig ("acc_mm_sig","acc_mm_sig",acc_mm_sig_mean,0,1);
      //RooRealVar acc_mm_sig_alpha ("acc_mm_sig_alpha", "acc_mm_sig_alpha", alpha);
      //RooRealVar acc_mm_sig_beta ("acc_mm_sig_beta", "acc_mm_sig_beta", beta);
      //RooBetaPdf pdf_acc_mm_sig ("pdf_acc_mm_sig" , "pdf_acc_mm_sig", rrv_acc_mm_sig, acc_mm_sig_alpha, acc_mm_sig_beta);
      //acc_mm_sig_alpha.setConstant();
      //acc_mm_sig_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_acc_mm_sig);
      //if(constantNonPoisson) rrv_acc_mm_sig.setConstant();
      //betaNuisancePdfs.add (pdf_acc_mm_sig);

      //RooRealVar N_pass_acc_mm_sig ("N_pass_acc_mm_sig", "N_pass_acc_mm_sig", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_acc_mm_sig );
      //RooRealVar N_fail_acc_mm_sig ("N_fail_acc_mm_sig", "N_fail_acc_mm_sig", beta-1,0,1e5);
      //observedParametersList.add( N_fail_acc_mm_sig );
      //RooAddition mu_acc_mm_sig ("mu_acc_mm_sig", "mu_acc_mm_sig", RooArgList(N_pass_acc_mm_sig,N_fail_acc_mm_sig));
      //RooRealVar rrv_acc_mm_sig ("acc_mm_sig","acc_mm_sig",acc_mm_sig_mean,0,1);
      //allNonPoissonNuisances.add (rrv_acc_mm_sig);
      //RooFormulaVar oneMinus_acc_mm_sig ("oneMinus_acc_mm_sig","1.0-@0",RooArgList(rrv_acc_mm_sig));
      //RooProduct mu_pass_acc_mm_sig ("mu_pass_acc_mm_sig", "mu_pass_acc_mm_sig",RooArgList(rrv_acc_mm_sig,mu_acc_mm_sig));
      //RooProduct mu_fail_acc_mm_sig ("mu_fail_acc_mm_sig", "mu_fail_acc_mm_sig",RooArgList(oneMinus_acc_mm_sig,mu_acc_mm_sig));
      //RooPoissonDummy pdf_pass_acc_mm_sig ("pdf_pass_acc_mm_sig" , "pdf_pass_acc_mm_sig", N_pass_acc_mm_sig, mu_pass_acc_mm_sig);
      //allNuisancePdfs.add(pdf_pass_acc_mm_sig);
      //RooPoissonDummy pdf_fail_acc_mm_sig ("pdf_fail_acc_mm_sig" , "pdf_fail_acc_mm_sig", N_fail_acc_mm_sig, mu_fail_acc_mm_sig);
      //allNuisancePdfs.add(pdf_fail_acc_mm_sig);

      RooRealVar N_pass_acc_mm_sig ("N_pass_acc_mm_sig", "N_pass_acc_mm_sig", alpha-1,0,1e5);
      observedParametersList.add( N_pass_acc_mm_sig );
      RooRealVar mu_pass_acc_mm_sig ("mu_pass_acc_mm_sig", "mu_pass_acc_mm_sig", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_acc_mm_sig);
      RooPoisson pdf_pass_acc_mm_sig ("pdf_pass_acc_mm_sig" , "pdf_pass_acc_mm_sig", N_pass_acc_mm_sig, mu_pass_acc_mm_sig);
      allNuisancePdfs.add(pdf_pass_acc_mm_sig);
      RooRealVar N_fail_acc_mm_sig ("N_fail_acc_mm_sig", "N_fail_acc_mm_sig", beta-1,0,1e5);
      observedParametersList.add( N_fail_acc_mm_sig );
      RooRealVar mu_fail_acc_mm_sig ("mu_fail_acc_mm_sig", "mu_fail_acc_mm_sig", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_acc_mm_sig);
      RooPoisson pdf_fail_acc_mm_sig ("pdf_fail_acc_mm_sig" , "pdf_fail_acc_mm_sig", N_fail_acc_mm_sig, mu_fail_acc_mm_sig);
      allNuisancePdfs.add(pdf_fail_acc_mm_sig);
      
      RooFormulaVar rrv_acc_mm_sig ("acc_mm_sig","@0/(@0+@1)",RooArgList(mu_pass_acc_mm_sig,mu_fail_acc_mm_sig));

      betaModeTransform(acc_mm_sb_mean,acc_mm_sb_err,alpha,beta);
      //RooRealVar N_pass_acc_mm_sb ("N_pass_acc_mm_sb", "N_pass_acc_mm_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_acc_mm_sb );
      //RooRealVar N_fail_acc_mm_sb ("N_fail_acc_mm_sb", "N_fail_acc_mm_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_acc_mm_sb );
      //RooRealVar rrv_acc_mm_sb ("acc_mm_sb","acc_mm_sb",acc_mm_sb_mean,0,1);
      //allNonPoissonNuisances.add (rrv_acc_mm_sb);
      //RooBetaPdfWithPoissonGenerator pdf_acc_mm_sb ("pdf_acc_mm_sb" , "pdf_acc_mm_sb", rrv_acc_mm_sb, N_pass_acc_mm_sb, N_fail_acc_mm_sb);
      //allNuisancePdfs.add(pdf_acc_mm_sb);

      //RooRealVar rrv_acc_mm_sb ("acc_mm_sb","acc_mm_sb",acc_mm_sb_mean,0,1);
      //RooRealVar acc_mm_sb_alpha ("acc_mm_sb_alpha", "acc_mm_sb_alpha", alpha);
      //RooRealVar acc_mm_sb_beta ("acc_mm_sb_beta", "acc_mm_sb_beta", beta);
      //RooBetaPdf pdf_acc_mm_sb ("pdf_acc_mm_sb" , "pdf_acc_mm_sb", rrv_acc_mm_sb, acc_mm_sb_alpha, acc_mm_sb_beta);
      //acc_mm_sb_alpha.setConstant();
      //acc_mm_sb_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_acc_mm_sb);
      //if(constantNonPoisson) rrv_acc_mm_sb.setConstant();
      //betaNuisancePdfs.add (pdf_acc_mm_sb);

      //RooRealVar N_pass_acc_mm_sb ("N_pass_acc_mm_sb", "N_pass_acc_mm_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_acc_mm_sb );
      //RooRealVar N_fail_acc_mm_sb ("N_fail_acc_mm_sb", "N_fail_acc_mm_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_acc_mm_sb );
      //RooAddition mu_acc_mm_sb ("mu_acc_mm_sb", "mu_acc_mm_sb", RooArgList(N_pass_acc_mm_sb,N_fail_acc_mm_sb));
      //RooRealVar rrv_acc_mm_sb ("acc_mm_sb","acc_mm_sb",acc_mm_sb_mean,0,1);
      //allNonPoissonNuisances.add (rrv_acc_mm_sb);
      //RooFormulaVar oneMinus_acc_mm_sb ("oneMinus_acc_mm_sb","1.0-@0",RooArgList(rrv_acc_mm_sb));
      //RooProduct mu_pass_acc_mm_sb ("mu_pass_acc_mm_sb", "mu_pass_acc_mm_sb",RooArgList(rrv_acc_mm_sb,mu_acc_mm_sb));
      //RooProduct mu_fail_acc_mm_sb ("mu_fail_acc_mm_sb", "mu_fail_acc_mm_sb",RooArgList(oneMinus_acc_mm_sb,mu_acc_mm_sb));
      //RooPoissonDummy pdf_pass_acc_mm_sb ("pdf_pass_acc_mm_sb" , "pdf_pass_acc_mm_sb", N_pass_acc_mm_sb, mu_pass_acc_mm_sb);
      //allNuisancePdfs.add(pdf_pass_acc_mm_sb);
      //RooPoissonDummy pdf_fail_acc_mm_sb ("pdf_fail_acc_mm_sb" , "pdf_fail_acc_mm_sb", N_fail_acc_mm_sb, mu_fail_acc_mm_sb);
      //allNuisancePdfs.add(pdf_fail_acc_mm_sb);

      RooRealVar N_pass_acc_mm_sb ("N_pass_acc_mm_sb", "N_pass_acc_mm_sb", alpha-1,0,1e5);
      observedParametersList.add( N_pass_acc_mm_sb );
      RooRealVar mu_pass_acc_mm_sb ("mu_pass_acc_mm_sb", "mu_pass_acc_mm_sb", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_acc_mm_sb);
      RooPoisson pdf_pass_acc_mm_sb ("pdf_pass_acc_mm_sb" , "pdf_pass_acc_mm_sb", N_pass_acc_mm_sb, mu_pass_acc_mm_sb);
      allNuisancePdfs.add(pdf_pass_acc_mm_sb);
      RooRealVar N_fail_acc_mm_sb ("N_fail_acc_mm_sb", "N_fail_acc_mm_sb", beta-1,0,1e5);
      observedParametersList.add( N_fail_acc_mm_sb );
      RooRealVar mu_fail_acc_mm_sb ("mu_fail_acc_mm_sb", "mu_fail_acc_mm_sb", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_acc_mm_sb);
      RooPoisson pdf_fail_acc_mm_sb ("pdf_fail_acc_mm_sb" , "pdf_fail_acc_mm_sb", N_fail_acc_mm_sb, mu_fail_acc_mm_sb);
      allNuisancePdfs.add(pdf_fail_acc_mm_sb);
      
      RooFormulaVar rrv_acc_mm_sb ("acc_mm_sb","@0/(@0+@1)",RooArgList(mu_pass_acc_mm_sb,mu_fail_acc_mm_sb));

      betaModeTransform(eff_ee_mean,eff_ee_err,alpha,beta);
      //RooRealVar N_pass_eff_ee ("N_pass_eff_ee", "N_pass_eff_ee", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_eff_ee );
      //RooRealVar N_fail_eff_ee ("N_fail_eff_ee", "N_fail_eff_ee", beta-1,0,1e5);
      //observedParametersList.add( N_fail_eff_ee );
      //RooRealVar rrv_eff_ee ("eff_ee","eff_ee",eff_ee_mean,0,1);
      //allNonPoissonNuisances.add (rrv_eff_ee);
      //RooBetaPdfWithPoissonGenerator pdf_eff_ee ("pdf_eff_ee" , "pdf_eff_ee", rrv_eff_ee, N_pass_eff_ee, N_fail_eff_ee);
      //allNuisancePdfs.add(pdf_eff_ee);

      //RooRealVar rrv_eff_ee ("eff_ee","eff_ee",eff_ee_mean,0,1);
      //RooRealVar eff_ee_alpha ("eff_ee_alpha", "eff_ee_alpha", alpha);
      //RooRealVar eff_ee_beta ("eff_ee_beta", "eff_ee_beta", beta);
      //RooBetaPdf pdf_eff_ee ("pdf_eff_ee" , "pdf_eff_ee", rrv_eff_ee, eff_ee_alpha, eff_ee_beta);
      //eff_ee_alpha.setConstant();
      //eff_ee_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_eff_ee);
      //if(constantNonPoisson) rrv_eff_ee.setConstant();
      //betaNuisancePdfs.add (pdf_eff_ee);

      //RooRealVar N_pass_eff_ee ("N_pass_eff_ee", "N_pass_eff_ee", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_eff_ee );
      //RooRealVar N_fail_eff_ee ("N_fail_eff_ee", "N_fail_eff_ee", beta-1,0,1e5);
      //observedParametersList.add( N_fail_eff_ee );
      //RooAddition mu_eff_ee ("mu_eff_ee", "mu_eff_ee", RooArgList(N_pass_eff_ee,N_fail_eff_ee));
      //RooRealVar rrv_eff_ee ("eff_ee","eff_ee",eff_ee_mean,0,1);
      //allNonPoissonNuisances.add (rrv_eff_ee);
      //RooFormulaVar oneMinus_eff_ee ("oneMinus_eff_ee","1.0-@0",RooArgList(rrv_eff_ee));
      //RooProduct mu_pass_eff_ee ("mu_pass_eff_ee", "mu_pass_eff_ee",RooArgList(rrv_eff_ee,mu_eff_ee));
      //RooProduct mu_fail_eff_ee ("mu_fail_eff_ee", "mu_fail_eff_ee",RooArgList(oneMinus_eff_ee,mu_eff_ee));
      //RooPoissonDummy pdf_pass_eff_ee ("pdf_pass_eff_ee" , "pdf_pass_eff_ee", N_pass_eff_ee, mu_pass_eff_ee);
      //allNuisancePdfs.add(pdf_pass_eff_ee);
      //RooPoissonDummy pdf_fail_eff_ee ("pdf_fail_eff_ee" , "pdf_fail_eff_ee", N_fail_eff_ee, mu_fail_eff_ee);
      //allNuisancePdfs.add(pdf_fail_eff_ee);

      RooRealVar N_pass_eff_ee ("N_pass_eff_ee", "N_pass_eff_ee", alpha-1,0,1e5);
      observedParametersList.add( N_pass_eff_ee );
      RooRealVar mu_pass_eff_ee ("mu_pass_eff_ee", "mu_pass_eff_ee", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_eff_ee);
      RooPoisson pdf_pass_eff_ee ("pdf_pass_eff_ee" , "pdf_pass_eff_ee", N_pass_eff_ee, mu_pass_eff_ee);
      allNuisancePdfs.add(pdf_pass_eff_ee);
      RooRealVar N_fail_eff_ee ("N_fail_eff_ee", "N_fail_eff_ee", beta-1,0,1e5);
      observedParametersList.add( N_fail_eff_ee );
      RooRealVar mu_fail_eff_ee ("mu_fail_eff_ee", "mu_fail_eff_ee", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_eff_ee);
      RooPoisson pdf_fail_eff_ee ("pdf_fail_eff_ee" , "pdf_fail_eff_ee", N_fail_eff_ee, mu_fail_eff_ee);
      allNuisancePdfs.add(pdf_fail_eff_ee);
      
      RooFormulaVar rrv_eff_ee ("eff_ee","@0/(@0+@1)",RooArgList(mu_pass_eff_ee,mu_fail_eff_ee));

      betaModeTransform(eff_mm_mean,eff_mm_err,alpha,beta);
      //RooRealVar N_pass_eff_mm ("N_pass_eff_mm", "N_pass_eff_mm", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_eff_mm );
      //RooRealVar N_fail_eff_mm ("N_fail_eff_mm", "N_fail_eff_mm", beta-1,0,1e5);
      //observedParametersList.add( N_fail_eff_mm );
      //RooRealVar rrv_eff_mm ("eff_mm","eff_mm",eff_mm_mean,0,1);
      //allNonPoissonNuisances.add (rrv_eff_mm);
      //RooBetaPdfWithPoissonGenerator pdf_eff_mm ("pdf_eff_mm" , "pdf_eff_mm", rrv_eff_mm, N_pass_eff_mm, N_fail_eff_mm);
      //allNuisancePdfs.add(pdf_eff_mm);

      //RooRealVar rrv_eff_mm ("eff_mm","eff_mm",eff_mm_mean,0,1);
      //RooRealVar eff_mm_alpha ("eff_mm_alpha", "eff_mm_alpha", alpha);
      //RooRealVar eff_mm_beta ("eff_mm_beta", "eff_mm_beta", beta);
      //RooBetaPdf pdf_eff_mm ("pdf_eff_mm" , "pdf_eff_mm", rrv_eff_mm, eff_mm_alpha, eff_mm_beta);
      //eff_mm_alpha.setConstant();
      //eff_mm_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_eff_mm);
      //if(constantNonPoisson) rrv_eff_mm.setConstant();
      //betaNuisancePdfs.add (pdf_eff_mm);

      //RooRealVar N_pass_eff_mm ("N_pass_eff_mm", "N_pass_eff_mm", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_eff_mm );
      //RooRealVar N_fail_eff_mm ("N_fail_eff_mm", "N_fail_eff_mm", beta-1,0,1e5);
      //observedParametersList.add( N_fail_eff_mm );
      //RooAddition mu_eff_mm ("mu_eff_mm", "mu_eff_mm", RooArgList(N_pass_eff_mm,N_fail_eff_mm));
      //RooRealVar rrv_eff_mm ("eff_mm","eff_mm",eff_mm_mean,0,1);
      //allNonPoissonNuisances.add (rrv_eff_mm);
      //RooFormulaVar oneMinus_eff_mm ("oneMinus_eff_mm","1.0-@0",RooArgList(rrv_eff_mm));
      //RooProduct mu_pass_eff_mm ("mu_pass_eff_mm", "mu_pass_eff_mm",RooArgList(rrv_eff_mm,mu_eff_mm));
      //RooProduct mu_fail_eff_mm ("mu_fail_eff_mm", "mu_fail_eff_mm",RooArgList(oneMinus_eff_mm,mu_eff_mm));
      //RooPoissonDummy pdf_pass_eff_mm ("pdf_pass_eff_mm" , "pdf_pass_eff_mm", N_pass_eff_mm, mu_pass_eff_mm);
      //allNuisancePdfs.add(pdf_pass_eff_mm);
      //RooPoissonDummy pdf_fail_eff_mm ("pdf_fail_eff_mm" , "pdf_fail_eff_mm", N_fail_eff_mm, mu_fail_eff_mm);
      //allNuisancePdfs.add(pdf_fail_eff_mm);

      RooRealVar N_pass_eff_mm ("N_pass_eff_mm", "N_pass_eff_mm", alpha-1,0,1e5);
      observedParametersList.add( N_pass_eff_mm );
      RooRealVar mu_pass_eff_mm ("mu_pass_eff_mm", "mu_pass_eff_mm", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_eff_mm);
      RooPoisson pdf_pass_eff_mm ("pdf_pass_eff_mm" , "pdf_pass_eff_mm", N_pass_eff_mm, mu_pass_eff_mm);
      allNuisancePdfs.add(pdf_pass_eff_mm);
      RooRealVar N_fail_eff_mm ("N_fail_eff_mm", "N_fail_eff_mm", beta-1,0,1e5);
      observedParametersList.add( N_fail_eff_mm );
      RooRealVar mu_fail_eff_mm ("mu_fail_eff_mm", "mu_fail_eff_mm", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_eff_mm);
      RooPoisson pdf_fail_eff_mm ("pdf_fail_eff_mm" , "pdf_fail_eff_mm", N_fail_eff_mm, mu_fail_eff_mm);
      allNuisancePdfs.add(pdf_fail_eff_mm);
      
      RooFormulaVar rrv_eff_mm ("eff_mm","@0/(@0+@1)",RooArgList(mu_pass_eff_mm,mu_fail_eff_mm));

      betaModeTransform(fsig_ee_mean,fsig_ee_err,alpha,beta);
      //RooRealVar N_pass_fsig_ee ("N_pass_fsig_ee", "N_pass_fsig_ee", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_fsig_ee );
      //RooRealVar N_fail_fsig_ee ("N_fail_fsig_ee", "N_fail_fsig_ee", beta-1,0,1e5);
      //observedParametersList.add( N_fail_fsig_ee );
      //RooRealVar rrv_fsig_ee ("fsig_ee","fsig_ee",fsig_ee_mean,0,1);
      //allNonPoissonNuisances.add (rrv_fsig_ee);
      //RooBetaPdfWithPoissonGenerator pdf_fsig_ee ("pdf_fsig_ee" , "pdf_fsig_ee", rrv_fsig_ee, N_pass_fsig_ee, N_fail_fsig_ee);
      //allNuisancePdfs.add(pdf_fsig_ee);

      //RooRealVar rrv_fsig_ee ("fsig_ee","fsig_ee",fsig_ee_mean,0,1);
      //RooRealVar fsig_ee_alpha ("fsig_ee_alpha", "fsig_ee_alpha", alpha);
      //RooRealVar fsig_ee_beta ("fsig_ee_beta", "fsig_ee_beta", beta);
      //RooBetaPdf pdf_fsig_ee ("pdf_fsig_ee" , "pdf_fsig_ee", rrv_fsig_ee, fsig_ee_alpha, fsig_ee_beta);
      //fsig_ee_alpha.setConstant();
      //fsig_ee_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_fsig_ee);
      //if(constantNonPoisson) rrv_fsig_ee.setConstant();
      //betaNuisancePdfs.add (pdf_fsig_ee);

      //RooRealVar N_pass_fsig_ee ("N_pass_fsig_ee", "N_pass_fsig_ee", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_fsig_ee );
      //RooRealVar N_fail_fsig_ee ("N_fail_fsig_ee", "N_fail_fsig_ee", beta-1,0,1e5);
      //observedParametersList.add( N_fail_fsig_ee );
      //RooAddition mu_fsig_ee ("mu_fsig_ee", "mu_fsig_ee", RooArgList(N_pass_fsig_ee,N_fail_fsig_ee));
      //RooRealVar rrv_fsig_ee ("fsig_ee","fsig_ee",fsig_ee_mean,0,1);
      //allNonPoissonNuisances.add (rrv_fsig_ee);
      //RooFormulaVar oneMinus_fsig_ee ("oneMinus_fsig_ee","1.0-@0",RooArgList(rrv_fsig_ee));
      //RooProduct mu_pass_fsig_ee ("mu_pass_fsig_ee", "mu_pass_fsig_ee",RooArgList(rrv_fsig_ee,mu_fsig_ee));
      //RooProduct mu_fail_fsig_ee ("mu_fail_fsig_ee", "mu_fail_fsig_ee",RooArgList(oneMinus_fsig_ee,mu_fsig_ee));
      //RooPoissonDummy pdf_pass_fsig_ee ("pdf_pass_fsig_ee" , "pdf_pass_fsig_ee", N_pass_fsig_ee, mu_pass_fsig_ee);
      //allNuisancePdfs.add(pdf_pass_fsig_ee);
      //RooPoissonDummy pdf_fail_fsig_ee ("pdf_fail_fsig_ee" , "pdf_fail_fsig_ee", N_fail_fsig_ee, mu_fail_fsig_ee);
      //allNuisancePdfs.add(pdf_fail_fsig_ee);

      RooRealVar N_pass_fsig_ee ("N_pass_fsig_ee", "N_pass_fsig_ee", alpha-1,0,1e5);
      observedParametersList.add( N_pass_fsig_ee );
      RooRealVar mu_pass_fsig_ee ("mu_pass_fsig_ee", "mu_pass_fsig_ee", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_fsig_ee);
      RooPoisson pdf_pass_fsig_ee ("pdf_pass_fsig_ee" , "pdf_pass_fsig_ee", N_pass_fsig_ee, mu_pass_fsig_ee);
      allNuisancePdfs.add(pdf_pass_fsig_ee);
      RooRealVar N_fail_fsig_ee ("N_fail_fsig_ee", "N_fail_fsig_ee", beta-1,0,1e5);
      observedParametersList.add( N_fail_fsig_ee );
      RooRealVar mu_fail_fsig_ee ("mu_fail_fsig_ee", "mu_fail_fsig_ee", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_fsig_ee);
      RooPoisson pdf_fail_fsig_ee ("pdf_fail_fsig_ee" , "pdf_fail_fsig_ee", N_fail_fsig_ee, mu_fail_fsig_ee);
      allNuisancePdfs.add(pdf_fail_fsig_ee);
      
      RooFormulaVar rrv_fsig_ee ("fsig_ee","@0/(@0+@1)",RooArgList(mu_pass_fsig_ee,mu_fail_fsig_ee));

      betaModeTransform(fsig_mm_mean,fsig_mm_err,alpha,beta);
      //RooRealVar N_pass_fsig_mm ("N_pass_fsig_mm", "N_pass_fsig_mm", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_fsig_mm );
      //RooRealVar N_fail_fsig_mm ("N_fail_fsig_mm", "N_fail_fsig_mm", beta-1,0,1e5);
      //observedParametersList.add( N_fail_fsig_mm );
      //RooRealVar rrv_fsig_mm ("fsig_mm","fsig_mm",fsig_mm_mean,0,1);
      //allNonPoissonNuisances.add (rrv_fsig_mm);
      //RooBetaPdfWithPoissonGenerator pdf_fsig_mm ("pdf_fsig_mm" , "pdf_fsig_mm", rrv_fsig_mm, N_pass_fsig_mm, N_fail_fsig_mm);
      //allNuisancePdfs.add(pdf_fsig_mm);

      //RooRealVar rrv_fsig_mm ("fsig_mm","fsig_mm",fsig_mm_mean,0,1);
      //RooRealVar fsig_mm_alpha ("fsig_mm_alpha", "fsig_mm_alpha", alpha);
      //RooRealVar fsig_mm_beta ("fsig_mm_beta", "fsig_mm_beta", beta);
      //RooBetaPdf pdf_fsig_mm ("pdf_fsig_mm" , "pdf_fsig_mm", rrv_fsig_mm, fsig_mm_alpha, fsig_mm_beta);
      //fsig_mm_alpha.setConstant();
      //fsig_mm_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_fsig_mm);
      //if(constantNonPoisson) rrv_fsig_mm.setConstant();
      //betaNuisancePdfs.add (pdf_fsig_mm);

      //RooRealVar N_pass_fsig_mm ("N_pass_fsig_mm", "N_pass_fsig_mm", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_fsig_mm );
      //RooRealVar N_fail_fsig_mm ("N_fail_fsig_mm", "N_fail_fsig_mm", beta-1,0,1e5);
      //observedParametersList.add( N_fail_fsig_mm );
      //RooAddition mu_fsig_mm ("mu_fsig_mm", "mu_fsig_mm", RooArgList(N_pass_fsig_mm,N_fail_fsig_mm));
      //RooRealVar rrv_fsig_mm ("fsig_mm","fsig_mm",fsig_mm_mean,0,1);
      //allNonPoissonNuisances.add (rrv_fsig_mm);
      //RooFormulaVar oneMinus_fsig_mm ("oneMinus_fsig_mm","1.0-@0",RooArgList(rrv_fsig_mm));
      //RooProduct mu_pass_fsig_mm ("mu_pass_fsig_mm", "mu_pass_fsig_mm",RooArgList(rrv_fsig_mm,mu_fsig_mm));
      //RooProduct mu_fail_fsig_mm ("mu_fail_fsig_mm", "mu_fail_fsig_mm",RooArgList(oneMinus_fsig_mm,mu_fsig_mm));
      //RooPoissonDummy pdf_pass_fsig_mm ("pdf_pass_fsig_mm" , "pdf_pass_fsig_mm", N_pass_fsig_mm, mu_pass_fsig_mm);
      //allNuisancePdfs.add(pdf_pass_fsig_mm);
      //RooPoissonDummy pdf_fail_fsig_mm ("pdf_fail_fsig_mm" , "pdf_fail_fsig_mm", N_fail_fsig_mm, mu_fail_fsig_mm);
      //allNuisancePdfs.add(pdf_fail_fsig_mm);

      RooRealVar N_pass_fsig_mm ("N_pass_fsig_mm", "N_pass_fsig_mm", alpha-1,0,1e5);
      observedParametersList.add( N_pass_fsig_mm );
      RooRealVar mu_pass_fsig_mm ("mu_pass_fsig_mm", "mu_pass_fsig_mm", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_fsig_mm);
      RooPoisson pdf_pass_fsig_mm ("pdf_pass_fsig_mm" , "pdf_pass_fsig_mm", N_pass_fsig_mm, mu_pass_fsig_mm);
      allNuisancePdfs.add(pdf_pass_fsig_mm);
      RooRealVar N_fail_fsig_mm ("N_fail_fsig_mm", "N_fail_fsig_mm", beta-1,0,1e5);
      observedParametersList.add( N_fail_fsig_mm );
      RooRealVar mu_fail_fsig_mm ("mu_fail_fsig_mm", "mu_fail_fsig_mm", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_fsig_mm);
      RooPoisson pdf_fail_fsig_mm ("pdf_fail_fsig_mm" , "pdf_fail_fsig_mm", N_fail_fsig_mm, mu_fail_fsig_mm);
      allNuisancePdfs.add(pdf_fail_fsig_mm);
      
      RooFormulaVar rrv_fsig_mm ("fsig_mm","@0/(@0+@1)",RooArgList(mu_pass_fsig_mm,mu_fail_fsig_mm));

      betaModeTransform(knn_ee_sig_mean,knn_ee_sig_err,alpha,beta);
      //RooRealVar N_pass_knn_ee_sig ("N_pass_knn_ee_sig", "N_pass_knn_ee_sig", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_knn_ee_sig );
      //RooRealVar N_fail_knn_ee_sig ("N_fail_knn_ee_sig", "N_fail_knn_ee_sig", beta-1,0,1e5);
      //observedParametersList.add( N_fail_knn_ee_sig );
      //RooRealVar rrv_knn_ee_sig ("knn_ee_sig","knn_ee_sig",knn_ee_sig_mean,0,1);
      //allNonPoissonNuisances.add (rrv_knn_ee_sig);
      //RooBetaPdfWithPoissonGenerator pdf_knn_ee_sig ("pdf_knn_ee_sig" , "pdf_knn_ee_sig", rrv_knn_ee_sig, N_pass_knn_ee_sig, N_fail_knn_ee_sig);
      //allNuisancePdfs.add(pdf_knn_ee_sig);

      //RooRealVar rrv_knn_ee_sig ("knn_ee_sig","knn_ee_sig",knn_ee_sig_mean,0,1);
      //RooRealVar knn_ee_sig_alpha ("knn_ee_sig_alpha", "knn_ee_sig_alpha", alpha);
      //RooRealVar knn_ee_sig_beta ("knn_ee_sig_beta", "knn_ee_sig_beta", beta);
      //RooBetaPdf pdf_knn_ee_sig ("pdf_knn_ee_sig" , "pdf_knn_ee_sig", rrv_knn_ee_sig, knn_ee_sig_alpha, knn_ee_sig_beta);
      //knn_ee_sig_alpha.setConstant();
      //knn_ee_sig_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_knn_ee_sig);
      //if(constantNonPoisson) rrv_knn_ee_sig.setConstant();
      //betaNuisancePdfs.add (pdf_knn_ee_sig);

      //RooRealVar N_pass_knn_ee_sig ("N_pass_knn_ee_sig", "N_pass_knn_ee_sig", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_knn_ee_sig );
      //RooRealVar N_fail_knn_ee_sig ("N_fail_knn_ee_sig", "N_fail_knn_ee_sig", beta-1,0,1e5);
      //observedParametersList.add( N_fail_knn_ee_sig );
      //RooAddition mu_knn_ee_sig ("mu_knn_ee_sig", "mu_knn_ee_sig", RooArgList(N_pass_knn_ee_sig,N_fail_knn_ee_sig));
      //RooRealVar rrv_knn_ee_sig ("knn_ee_sig","knn_ee_sig",knn_ee_sig_mean,0,1);
      //allNonPoissonNuisances.add (rrv_knn_ee_sig);
      //RooFormulaVar oneMinus_knn_ee_sig ("oneMinus_knn_ee_sig","1.0-@0",RooArgList(rrv_knn_ee_sig));
      //RooProduct mu_pass_knn_ee_sig ("mu_pass_knn_ee_sig", "mu_pass_knn_ee_sig",RooArgList(rrv_knn_ee_sig,mu_knn_ee_sig));
      //RooProduct mu_fail_knn_ee_sig ("mu_fail_knn_ee_sig", "mu_fail_knn_ee_sig",RooArgList(oneMinus_knn_ee_sig,mu_knn_ee_sig));
      //RooPoissonDummy pdf_pass_knn_ee_sig ("pdf_pass_knn_ee_sig" , "pdf_pass_knn_ee_sig", N_pass_knn_ee_sig, mu_pass_knn_ee_sig);
      //allNuisancePdfs.add(pdf_pass_knn_ee_sig);
      //RooPoissonDummy pdf_fail_knn_ee_sig ("pdf_fail_knn_ee_sig" , "pdf_fail_knn_ee_sig", N_fail_knn_ee_sig, mu_fail_knn_ee_sig);
      //allNuisancePdfs.add(pdf_fail_knn_ee_sig);

      RooRealVar N_pass_knn_ee_sig ("N_pass_knn_ee_sig", "N_pass_knn_ee_sig", alpha-1,0,1e5);
      observedParametersList.add( N_pass_knn_ee_sig );
      RooRealVar mu_pass_knn_ee_sig ("mu_pass_knn_ee_sig", "mu_pass_knn_ee_sig", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_knn_ee_sig);
      RooPoisson pdf_pass_knn_ee_sig ("pdf_pass_knn_ee_sig" , "pdf_pass_knn_ee_sig", N_pass_knn_ee_sig, mu_pass_knn_ee_sig);
      allNuisancePdfs.add(pdf_pass_knn_ee_sig);
      RooRealVar N_fail_knn_ee_sig ("N_fail_knn_ee_sig", "N_fail_knn_ee_sig", beta-1,0,1e5);
      observedParametersList.add( N_fail_knn_ee_sig );
      RooRealVar mu_fail_knn_ee_sig ("mu_fail_knn_ee_sig", "mu_fail_knn_ee_sig", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_knn_ee_sig);
      RooPoisson pdf_fail_knn_ee_sig ("pdf_fail_knn_ee_sig" , "pdf_fail_knn_ee_sig", N_fail_knn_ee_sig, mu_fail_knn_ee_sig);
      allNuisancePdfs.add(pdf_fail_knn_ee_sig);
      
      RooFormulaVar rrv_knn_ee_sig ("knn_ee_sig","@0/(@0+@1)",RooArgList(mu_pass_knn_ee_sig,mu_fail_knn_ee_sig));

      betaModeTransform(knn_ee_sb_mean,knn_ee_sb_err,alpha,beta);
      //RooRealVar N_pass_knn_ee_sb ("N_pass_knn_ee_sb", "N_pass_knn_ee_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_knn_ee_sb );
      //RooRealVar N_fail_knn_ee_sb ("N_fail_knn_ee_sb", "N_fail_knn_ee_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_knn_ee_sb );
      //RooRealVar rrv_knn_ee_sb ("knn_ee_sb","knn_ee_sb",knn_ee_sb_mean,0,1);
      //allNonPoissonNuisances.add (rrv_knn_ee_sb);
      //RooBetaPdfWithPoissonGenerator pdf_knn_ee_sb ("pdf_knn_ee_sb" , "pdf_knn_ee_sb", rrv_knn_ee_sb, N_pass_knn_ee_sb, N_fail_knn_ee_sb);
      //allNuisancePdfs.add(pdf_knn_ee_sb);

      //RooRealVar rrv_knn_ee_sb ("knn_ee_sb","knn_ee_sb",knn_ee_sb_mean,0,1);
      //RooRealVar knn_ee_sb_alpha ("knn_ee_sb_alpha", "knn_ee_sb_alpha", alpha);
      //RooRealVar knn_ee_sb_beta ("knn_ee_sb_beta", "knn_ee_sb_beta", beta);
      //RooBetaPdf pdf_knn_ee_sb ("pdf_knn_ee_sb" , "pdf_knn_ee_sb", rrv_knn_ee_sb, knn_ee_sb_alpha, knn_ee_sb_beta);
      //knn_ee_sb_alpha.setConstant();
      //knn_ee_sb_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_knn_ee_sb);
      //if(constantNonPoisson) rrv_knn_ee_sb.setConstant();
      //betaNuisancePdfs.add (pdf_knn_ee_sb);

      //RooRealVar N_pass_knn_ee_sb ("N_pass_knn_ee_sb", "N_pass_knn_ee_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_knn_ee_sb );
      //RooRealVar N_fail_knn_ee_sb ("N_fail_knn_ee_sb", "N_fail_knn_ee_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_knn_ee_sb );
      //RooAddition mu_knn_ee_sb ("mu_knn_ee_sb", "mu_knn_ee_sb", RooArgList(N_pass_knn_ee_sb,N_fail_knn_ee_sb));
      //RooRealVar rrv_knn_ee_sb ("knn_ee_sb","knn_ee_sb",knn_ee_sb_mean,0,1);
      //allNonPoissonNuisances.add (rrv_knn_ee_sb);
      //RooFormulaVar oneMinus_knn_ee_sb ("oneMinus_knn_ee_sb","1.0-@0",RooArgList(rrv_knn_ee_sb));
      //RooProduct mu_pass_knn_ee_sb ("mu_pass_knn_ee_sb", "mu_pass_knn_ee_sb",RooArgList(rrv_knn_ee_sb,mu_knn_ee_sb));
      //RooProduct mu_fail_knn_ee_sb ("mu_fail_knn_ee_sb", "mu_fail_knn_ee_sb",RooArgList(oneMinus_knn_ee_sb,mu_knn_ee_sb));
      //RooPoissonDummy pdf_pass_knn_ee_sb ("pdf_pass_knn_ee_sb" , "pdf_pass_knn_ee_sb", N_pass_knn_ee_sb, mu_pass_knn_ee_sb);
      //allNuisancePdfs.add(pdf_pass_knn_ee_sb);
      //RooPoissonDummy pdf_fail_knn_ee_sb ("pdf_fail_knn_ee_sb" , "pdf_fail_knn_ee_sb", N_fail_knn_ee_sb, mu_fail_knn_ee_sb);
      //allNuisancePdfs.add(pdf_fail_knn_ee_sb);

      RooRealVar N_pass_knn_ee_sb ("N_pass_knn_ee_sb", "N_pass_knn_ee_sb", alpha-1,0,1e5);
      observedParametersList.add( N_pass_knn_ee_sb );
      RooRealVar mu_pass_knn_ee_sb ("mu_pass_knn_ee_sb", "mu_pass_knn_ee_sb", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_knn_ee_sb);
      RooPoisson pdf_pass_knn_ee_sb ("pdf_pass_knn_ee_sb" , "pdf_pass_knn_ee_sb", N_pass_knn_ee_sb, mu_pass_knn_ee_sb);
      allNuisancePdfs.add(pdf_pass_knn_ee_sb);
      RooRealVar N_fail_knn_ee_sb ("N_fail_knn_ee_sb", "N_fail_knn_ee_sb", beta-1,0,1e5);
      observedParametersList.add( N_fail_knn_ee_sb );
      RooRealVar mu_fail_knn_ee_sb ("mu_fail_knn_ee_sb", "mu_fail_knn_ee_sb", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_knn_ee_sb);
      RooPoisson pdf_fail_knn_ee_sb ("pdf_fail_knn_ee_sb" , "pdf_fail_knn_ee_sb", N_fail_knn_ee_sb, mu_fail_knn_ee_sb);
      allNuisancePdfs.add(pdf_fail_knn_ee_sb);
      
      RooFormulaVar rrv_knn_ee_sb ("knn_ee_sb","@0/(@0+@1)",RooArgList(mu_pass_knn_ee_sb,mu_fail_knn_ee_sb));

      betaModeTransform(knn_mm_sig_mean,knn_mm_sig_err,alpha,beta);
      //RooRealVar N_pass_knn_mm_sig ("N_pass_knn_mm_sig", "N_pass_knn_mm_sig", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_knn_mm_sig );
      //RooRealVar N_fail_knn_mm_sig ("N_fail_knn_mm_sig", "N_fail_knn_mm_sig", beta-1,0,1e5);
      //observedParametersList.add( N_fail_knn_mm_sig );
      //RooRealVar rrv_knn_mm_sig ("knn_mm_sig","knn_mm_sig",knn_mm_sig_mean,0,1);
      //allNonPoissonNuisances.add (rrv_knn_mm_sig);
      //RooBetaPdfWithPoissonGenerator pdf_knn_mm_sig ("pdf_knn_mm_sig" , "pdf_knn_mm_sig", rrv_knn_mm_sig, N_pass_knn_mm_sig, N_fail_knn_mm_sig);
      //allNuisancePdfs.add(pdf_knn_mm_sig);

      //RooRealVar rrv_knn_mm_sig ("knn_mm_sig","knn_mm_sig",knn_mm_sig_mean,0,1);
      //RooRealVar knn_mm_sig_alpha ("knn_mm_sig_alpha", "knn_mm_sig_alpha", alpha);
      //RooRealVar knn_mm_sig_beta ("knn_mm_sig_beta", "knn_mm_sig_beta", beta);
      //RooBetaPdf pdf_knn_mm_sig ("pdf_knn_mm_sig" , "pdf_knn_mm_sig", rrv_knn_mm_sig, knn_mm_sig_alpha, knn_mm_sig_beta);
      //knn_mm_sig_alpha.setConstant();
      //knn_mm_sig_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_knn_mm_sig);
      //if(constantNonPoisson) rrv_knn_mm_sig.setConstant();
      //betaNuisancePdfs.add (pdf_knn_mm_sig);

      //RooRealVar N_pass_knn_mm_sig ("N_pass_knn_mm_sig", "N_pass_knn_mm_sig", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_knn_mm_sig );
      //RooRealVar N_fail_knn_mm_sig ("N_fail_knn_mm_sig", "N_fail_knn_mm_sig", beta-1,0,1e5);
      //observedParametersList.add( N_fail_knn_mm_sig );
      //RooAddition mu_knn_mm_sig ("mu_knn_mm_sig", "mu_knn_mm_sig", RooArgList(N_pass_knn_mm_sig,N_fail_knn_mm_sig));
      //RooRealVar rrv_knn_mm_sig ("knn_mm_sig","knn_mm_sig",knn_mm_sig_mean,0,1);
      //allNonPoissonNuisances.add (rrv_knn_mm_sig);
      //RooFormulaVar oneMinus_knn_mm_sig ("oneMinus_knn_mm_sig","1.0-@0",RooArgList(rrv_knn_mm_sig));
      //RooProduct mu_pass_knn_mm_sig ("mu_pass_knn_mm_sig", "mu_pass_knn_mm_sig",RooArgList(rrv_knn_mm_sig,mu_knn_mm_sig));
      //RooProduct mu_fail_knn_mm_sig ("mu_fail_knn_mm_sig", "mu_fail_knn_mm_sig",RooArgList(oneMinus_knn_mm_sig,mu_knn_mm_sig));
      //RooPoissonDummy pdf_pass_knn_mm_sig ("pdf_pass_knn_mm_sig" , "pdf_pass_knn_mm_sig", N_pass_knn_mm_sig, mu_pass_knn_mm_sig);
      //allNuisancePdfs.add(pdf_pass_knn_mm_sig);
      //RooPoissonDummy pdf_fail_knn_mm_sig ("pdf_fail_knn_mm_sig" , "pdf_fail_knn_mm_sig", N_fail_knn_mm_sig, mu_fail_knn_mm_sig);
      //allNuisancePdfs.add(pdf_fail_knn_mm_sig);

      RooRealVar N_pass_knn_mm_sig ("N_pass_knn_mm_sig", "N_pass_knn_mm_sig", alpha-1,0,1e5);
      observedParametersList.add( N_pass_knn_mm_sig );
      RooRealVar mu_pass_knn_mm_sig ("mu_pass_knn_mm_sig", "mu_pass_knn_mm_sig", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_knn_mm_sig);
      RooPoisson pdf_pass_knn_mm_sig ("pdf_pass_knn_mm_sig" , "pdf_pass_knn_mm_sig", N_pass_knn_mm_sig, mu_pass_knn_mm_sig);
      allNuisancePdfs.add(pdf_pass_knn_mm_sig);
      RooRealVar N_fail_knn_mm_sig ("N_fail_knn_mm_sig", "N_fail_knn_mm_sig", beta-1,0,1e5);
      observedParametersList.add( N_fail_knn_mm_sig );
      RooRealVar mu_fail_knn_mm_sig ("mu_fail_knn_mm_sig", "mu_fail_knn_mm_sig", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_knn_mm_sig);
      RooPoisson pdf_fail_knn_mm_sig ("pdf_fail_knn_mm_sig" , "pdf_fail_knn_mm_sig", N_fail_knn_mm_sig, mu_fail_knn_mm_sig);
      allNuisancePdfs.add(pdf_fail_knn_mm_sig);
      
      RooFormulaVar rrv_knn_mm_sig ("knn_mm_sig","@0/(@0+@1)",RooArgList(mu_pass_knn_mm_sig,mu_fail_knn_mm_sig));

      betaModeTransform(knn_mm_sb_mean,knn_mm_sb_err,alpha,beta);
      //RooRealVar N_pass_knn_mm_sb ("N_pass_knn_mm_sb", "N_pass_knn_mm_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_knn_mm_sb );
      //RooRealVar N_fail_knn_mm_sb ("N_fail_knn_mm_sb", "N_fail_knn_mm_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_knn_mm_sb );
      //RooRealVar rrv_knn_mm_sb ("knn_mm_sb","knn_mm_sb",knn_mm_sb_mean,0,1);
      //allNonPoissonNuisances.add (rrv_knn_mm_sb);
      //RooBetaPdfWithPoissonGenerator pdf_knn_mm_sb ("pdf_knn_mm_sb" , "pdf_knn_mm_sb", rrv_knn_mm_sb, N_pass_knn_mm_sb, N_fail_knn_mm_sb);
      //allNuisancePdfs.add(pdf_knn_mm_sb);

      //RooRealVar rrv_knn_mm_sb ("knn_mm_sb","knn_mm_sb",knn_mm_sb_mean,0,1);
      //RooRealVar knn_mm_sb_alpha ("knn_mm_sb_alpha", "knn_mm_sb_alpha", alpha);
      //RooRealVar knn_mm_sb_beta ("knn_mm_sb_beta", "knn_mm_sb_beta", beta);
      //RooBetaPdf pdf_knn_mm_sb ("pdf_knn_mm_sb" , "pdf_knn_mm_sb", rrv_knn_mm_sb, knn_mm_sb_alpha, knn_mm_sb_beta);
      //knn_mm_sb_alpha.setConstant();
      //knn_mm_sb_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_knn_mm_sb);
      //if(constantNonPoisson) rrv_knn_mm_sb.setConstant();
      //betaNuisancePdfs.add (pdf_knn_mm_sb);

      //RooRealVar N_pass_knn_mm_sb ("N_pass_knn_mm_sb", "N_pass_knn_mm_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_knn_mm_sb );
      //RooRealVar N_fail_knn_mm_sb ("N_fail_knn_mm_sb", "N_fail_knn_mm_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_knn_mm_sb );
      //RooAddition mu_knn_mm_sb ("mu_knn_mm_sb", "mu_knn_mm_sb", RooArgList(N_pass_knn_mm_sb,N_fail_knn_mm_sb));
      //RooRealVar rrv_knn_mm_sb ("knn_mm_sb","knn_mm_sb",knn_mm_sb_mean,0,1);
      //allNonPoissonNuisances.add (rrv_knn_mm_sb);
      //RooFormulaVar oneMinus_knn_mm_sb ("oneMinus_knn_mm_sb","1.0-@0",RooArgList(rrv_knn_mm_sb));
      //RooProduct mu_pass_knn_mm_sb ("mu_pass_knn_mm_sb", "mu_pass_knn_mm_sb",RooArgList(rrv_knn_mm_sb,mu_knn_mm_sb));
      //RooProduct mu_fail_knn_mm_sb ("mu_fail_knn_mm_sb", "mu_fail_knn_mm_sb",RooArgList(oneMinus_knn_mm_sb,mu_knn_mm_sb));
      //RooPoissonDummy pdf_pass_knn_mm_sb ("pdf_pass_knn_mm_sb" , "pdf_pass_knn_mm_sb", N_pass_knn_mm_sb, mu_pass_knn_mm_sb);
      //allNuisancePdfs.add(pdf_pass_knn_mm_sb);
      //RooPoissonDummy pdf_fail_knn_mm_sb ("pdf_fail_knn_mm_sb" , "pdf_fail_knn_mm_sb", N_fail_knn_mm_sb, mu_fail_knn_mm_sb);
      //allNuisancePdfs.add(pdf_fail_knn_mm_sb);

      RooRealVar N_pass_knn_mm_sb ("N_pass_knn_mm_sb", "N_pass_knn_mm_sb", alpha-1,0,1e5);
      observedParametersList.add( N_pass_knn_mm_sb );
      RooRealVar mu_pass_knn_mm_sb ("mu_pass_knn_mm_sb", "mu_pass_knn_mm_sb", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_knn_mm_sb);
      RooPoisson pdf_pass_knn_mm_sb ("pdf_pass_knn_mm_sb" , "pdf_pass_knn_mm_sb", N_pass_knn_mm_sb, mu_pass_knn_mm_sb);
      allNuisancePdfs.add(pdf_pass_knn_mm_sb);
      RooRealVar N_fail_knn_mm_sb ("N_fail_knn_mm_sb", "N_fail_knn_mm_sb", beta-1,0,1e5);
      observedParametersList.add( N_fail_knn_mm_sb );
      RooRealVar mu_fail_knn_mm_sb ("mu_fail_knn_mm_sb", "mu_fail_knn_mm_sb", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_knn_mm_sb);
      RooPoisson pdf_fail_knn_mm_sb ("pdf_fail_knn_mm_sb" , "pdf_fail_knn_mm_sb", N_fail_knn_mm_sb, mu_fail_knn_mm_sb);
      allNuisancePdfs.add(pdf_fail_knn_mm_sb);
      
      RooFormulaVar rrv_knn_mm_sb ("knn_mm_sb","@0/(@0+@1)",RooArgList(mu_pass_knn_mm_sb,mu_fail_knn_mm_sb));


    //-------  add trigger efficiencies as nuisance parameters with Beta Funtions

      betaModeTransform(eps_sb_mean-epsSF_sb_errm+0.5*(epsSF_sb_errm+epsSF_sb_errp),epsSF_sb_errm+epsSF_sb_errp,alpha,beta);
      //RooRealVar N_pass_epsSF_sb ("N_pass_epsSF_sb", "N_pass_epsSF_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sb );
      //RooRealVar N_fail_epsSF_sb ("N_fail_epsSF_sb", "N_fail_epsSF_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sb );
      //RooRealVar rrv_epsSF_sb ("epsSF_sb","epsSF_sb",eps_sb_mean-epsSF_sb_errm+0.5*(epsSF_sb_errm+epsSF_sb_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sb);
      //RooBetaPdfWithPoissonGenerator pdf_epsSF_sb ("pdf_epsSF_sb" , "pdf_epsSF_sb", rrv_epsSF_sb, N_pass_epsSF_sb, N_fail_epsSF_sb);
      //allNuisancePdfs.add(pdf_epsSF_sb);

      //RooRealVar rrv_epsSF_sb ("epsSF_sb","epsSF_sb",eps_sb_mean-epsSF_sb_errm+0.5*(epsSF_sb_errm+epsSF_sb_errp),0,1);
      //RooRealVar epsSF_sb_alpha ("epsSF_sb_alpha", "epsSF_sb_alpha", alpha);
      //RooRealVar epsSF_sb_beta ("epsSF_sb_beta", "epsSF_sb_beta", beta);
      //RooBetaPdf pdf_epsSF_sb ("pdf_epsSF_sb" , "pdf_epsSF_sb", rrv_epsSF_sb, epsSF_sb_alpha, epsSF_sb_beta);
      //epsSF_sb_alpha.setConstant();
      //epsSF_sb_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_epsSF_sb);
      //if(constantNonPoisson) rrv_epsSF_sb.setConstant();
      //betaNuisancePdfs.add (pdf_epsSF_sb);

      //RooRealVar N_pass_epsSF_sb ("N_pass_epsSF_sb", "N_pass_epsSF_sb", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sb );
      //RooRealVar N_fail_epsSF_sb ("N_fail_epsSF_sb", "N_fail_epsSF_sb", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sb );
      //RooAddition mu_epsSF_sb ("mu_epsSF_sb", "mu_epsSF_sb", RooArgList(N_pass_epsSF_sb,N_fail_epsSF_sb));
      //RooRealVar rrv_epsSF_sb ("epsSF_sb","epsSF_sb",eps_sb_mean-epsSF_sb_errm+0.5*(epsSF_sb_errm+epsSF_sb_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sb);
      //RooFormulaVar oneMinus_epsSF_sb ("oneMinus_epsSF_sb","1.0-@0",RooArgList(rrv_epsSF_sb));
      //RooProduct mu_pass_epsSF_sb ("mu_pass_epsSF_sb", "mu_pass_epsSF_sb",RooArgList(rrv_epsSF_sb,mu_epsSF_sb));
      //RooProduct mu_fail_epsSF_sb ("mu_fail_epsSF_sb", "mu_fail_epsSF_sb",RooArgList(oneMinus_epsSF_sb,mu_epsSF_sb));
      //RooPoissonDummy pdf_pass_epsSF_sb ("pdf_pass_epsSF_sb" , "pdf_pass_epsSF_sb", N_pass_epsSF_sb, mu_pass_epsSF_sb);
      //allNuisancePdfs.add(pdf_pass_epsSF_sb);
      //RooPoissonDummy pdf_fail_epsSF_sb ("pdf_fail_epsSF_sb" , "pdf_fail_epsSF_sb", N_fail_epsSF_sb, mu_fail_epsSF_sb);
      //allNuisancePdfs.add(pdf_fail_epsSF_sb);

      RooRealVar N_pass_epsSF_sb ("N_pass_epsSF_sb", "N_pass_epsSF_sb", alpha-1,0,1e5);
      observedParametersList.add( N_pass_epsSF_sb );
      RooRealVar mu_pass_epsSF_sb ("mu_pass_epsSF_sb", "mu_pass_epsSF_sb", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_epsSF_sb);
      RooPoisson pdf_pass_epsSF_sb ("pdf_pass_epsSF_sb" , "pdf_pass_epsSF_sb", N_pass_epsSF_sb, mu_pass_epsSF_sb);
      allNuisancePdfs.add(pdf_pass_epsSF_sb);
      RooRealVar N_fail_epsSF_sb ("N_fail_epsSF_sb", "N_fail_epsSF_sb", beta-1,0,1e5);
      observedParametersList.add( N_fail_epsSF_sb );
      RooRealVar mu_fail_epsSF_sb ("mu_fail_epsSF_sb", "mu_fail_epsSF_sb", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_epsSF_sb);
      RooPoisson pdf_fail_epsSF_sb ("pdf_fail_epsSF_sb" , "pdf_fail_epsSF_sb", N_fail_epsSF_sb, mu_fail_epsSF_sb);
      allNuisancePdfs.add(pdf_fail_epsSF_sb);
      
      RooFormulaVar rrv_epsSF_sb ("epsSF_sb","@0/(@0+@1)",RooArgList(mu_pass_epsSF_sb,mu_fail_epsSF_sb));

      betaModeTransform(eps_sig_mean-epsSF_sig_errm+0.5*(epsSF_sig_errm+epsSF_sig_errp),epsSF_sig_errm+epsSF_sig_errp,alpha,beta);
      //RooRealVar N_pass_epsSF_sig ("N_pass_epsSF_sig", "N_pass_epsSF_sig", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sig );
      //RooRealVar N_fail_epsSF_sig ("N_fail_epsSF_sig", "N_fail_epsSF_sig", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sig );
      //RooRealVar rrv_epsSF_sig ("epsSF_sig","epsSF_sig",eps_sig_mean-epsSF_sig_errm+0.5*(epsSF_sig_errm+epsSF_sig_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sig);
      //RooBetaPdfWithPoissonGenerator pdf_epsSF_sig ("pdf_epsSF_sig" , "pdf_epsSF_sig", rrv_epsSF_sig, N_pass_epsSF_sig, N_fail_epsSF_sig);
      //allNuisancePdfs.add(pdf_epsSF_sig);

      //RooRealVar rrv_epsSF_sig ("epsSF_sig","epsSF_sig",eps_sig_mean-epsSF_sig_errm+0.5*(epsSF_sig_errm+epsSF_sig_errp),0,1);
      //RooRealVar epsSF_sig_alpha ("epsSF_sig_alpha", "epsSF_sig_alpha", alpha);
      //RooRealVar epsSF_sig_beta ("epsSF_sig_beta", "epsSF_sig_beta", beta);
      //RooBetaPdf pdf_epsSF_sig ("pdf_epsSF_sig" , "pdf_epsSF_sig", rrv_epsSF_sig, epsSF_sig_alpha, epsSF_sig_beta);
      //epsSF_sig_alpha.setConstant();
      //epsSF_sig_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_epsSF_sig);
      //if(constantNonPoisson) rrv_epsSF_sig.setConstant();
      //betaNuisancePdfs.add (pdf_epsSF_sig);

      //RooRealVar N_pass_epsSF_sig ("N_pass_epsSF_sig", "N_pass_epsSF_sig", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sig );
      //RooRealVar N_fail_epsSF_sig ("N_fail_epsSF_sig", "N_fail_epsSF_sig", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sig );
      //RooAddition mu_epsSF_sig ("mu_epsSF_sig", "mu_epsSF_sig", RooArgList(N_pass_epsSF_sig,N_fail_epsSF_sig));
      //RooRealVar rrv_epsSF_sig ("epsSF_sig","epsSF_sig",eps_sig_mean-epsSF_sig_errm+0.5*(epsSF_sig_errm+epsSF_sig_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sig);
      //RooFormulaVar oneMinus_epsSF_sig ("oneMinus_epsSF_sig","1.0-@0",RooArgList(rrv_epsSF_sig));
      //RooProduct mu_pass_epsSF_sig ("mu_pass_epsSF_sig", "mu_pass_epsSF_sig",RooArgList(rrv_epsSF_sig,mu_epsSF_sig));
      //RooProduct mu_fail_epsSF_sig ("mu_fail_epsSF_sig", "mu_fail_epsSF_sig",RooArgList(oneMinus_epsSF_sig,mu_epsSF_sig));
      //RooPoissonDummy pdf_pass_epsSF_sig ("pdf_pass_epsSF_sig" , "pdf_pass_epsSF_sig", N_pass_epsSF_sig, mu_pass_epsSF_sig);
      //allNuisancePdfs.add(pdf_pass_epsSF_sig);
      //RooPoissonDummy pdf_fail_epsSF_sig ("pdf_fail_epsSF_sig" , "pdf_fail_epsSF_sig", N_fail_epsSF_sig, mu_fail_epsSF_sig);
      //allNuisancePdfs.add(pdf_fail_epsSF_sig);

      RooRealVar N_pass_epsSF_sig ("N_pass_epsSF_sig", "N_pass_epsSF_sig", alpha-1,0,1e5);
      observedParametersList.add( N_pass_epsSF_sig );
      RooRealVar mu_pass_epsSF_sig ("mu_pass_epsSF_sig", "mu_pass_epsSF_sig", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_epsSF_sig);
      RooPoisson pdf_pass_epsSF_sig ("pdf_pass_epsSF_sig" , "pdf_pass_epsSF_sig", N_pass_epsSF_sig, mu_pass_epsSF_sig);
      allNuisancePdfs.add(pdf_pass_epsSF_sig);
      RooRealVar N_fail_epsSF_sig ("N_fail_epsSF_sig", "N_fail_epsSF_sig", beta-1,0,1e5);
      observedParametersList.add( N_fail_epsSF_sig );
      RooRealVar mu_fail_epsSF_sig ("mu_fail_epsSF_sig", "mu_fail_epsSF_sig", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_epsSF_sig);
      RooPoisson pdf_fail_epsSF_sig ("pdf_fail_epsSF_sig" , "pdf_fail_epsSF_sig", N_fail_epsSF_sig, mu_fail_epsSF_sig);
      allNuisancePdfs.add(pdf_fail_epsSF_sig);
      
      RooFormulaVar rrv_epsSF_sig ("epsSF_sig","@0/(@0+@1)",RooArgList(mu_pass_epsSF_sig,mu_fail_epsSF_sig));

      betaModeTransform(eps_sb_ldp_mean-epsSF_sb_ldp_errm+0.5*(epsSF_sb_ldp_errm+epsSF_sb_ldp_errp),epsSF_sb_ldp_errm+epsSF_sb_ldp_errp,alpha,beta);
      //RooRealVar N_pass_epsSF_sb_ldp ("N_pass_epsSF_sb_ldp", "N_pass_epsSF_sb_ldp", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sb_ldp );
      //RooRealVar N_fail_epsSF_sb_ldp ("N_fail_epsSF_sb_ldp", "N_fail_epsSF_sb_ldp", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sb_ldp );
      //RooRealVar rrv_epsSF_sb_ldp ("epsSF_sb_ldp","epsSF_sb_ldp",eps_sb_ldp_mean-epsSF_sb_ldp_errm+0.5*(epsSF_sb_ldp_errm+epsSF_sb_ldp_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sb_ldp);
      //RooBetaPdfWithPoissonGenerator pdf_epsSF_sb_ldp ("pdf_epsSF_sb_ldp" , "pdf_epsSF_sb_ldp", rrv_epsSF_sb_ldp, N_pass_epsSF_sb_ldp, N_fail_epsSF_sb_ldp);
      //allNuisancePdfs.add(pdf_epsSF_sb_ldp);

      //RooRealVar rrv_epsSF_sb_ldp ("epsSF_sb_ldp","epsSF_sb_ldp",eps_sb_ldp_mean-epsSF_sb_ldp_errm+0.5*(epsSF_sb_ldp_errm+epsSF_sb_ldp_errp),0,1);
      //RooRealVar epsSF_sb_ldp_alpha ("epsSF_sb_ldp_alpha", "epsSF_sb_ldp_alpha", alpha);
      //RooRealVar epsSF_sb_ldp_beta ("epsSF_sb_ldp_beta", "epsSF_sb_ldp_beta", beta);
      //RooBetaPdf pdf_epsSF_sb_ldp ("pdf_epsSF_sb_ldp" , "pdf_epsSF_sb_ldp", rrv_epsSF_sb_ldp, epsSF_sb_ldp_alpha, epsSF_sb_ldp_beta);
      //epsSF_sb_ldp_alpha.setConstant();
      //epsSF_sb_ldp_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_epsSF_sb_ldp);
      //if(constantNonPoisson) rrv_epsSF_sb_ldp.setConstant();
      //betaNuisancePdfs.add (pdf_epsSF_sb_ldp);

      //RooRealVar N_pass_epsSF_sb_ldp ("N_pass_epsSF_sb_ldp", "N_pass_epsSF_sb_ldp", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sb_ldp );
      //RooRealVar N_fail_epsSF_sb_ldp ("N_fail_epsSF_sb_ldp", "N_fail_epsSF_sb_ldp", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sb_ldp );
      //RooAddition mu_epsSF_sb_ldp ("mu_epsSF_sb_ldp", "mu_epsSF_sb_ldp", RooArgList(N_pass_epsSF_sb_ldp,N_fail_epsSF_sb_ldp));
      //RooRealVar rrv_epsSF_sb_ldp ("epsSF_sb_ldp","epsSF_sb_ldp",eps_sig_mean-epsSF_sb_ldp_errm+0.5*(epsSF_sb_ldp_errm+epsSF_sb_ldp_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sb_ldp);
      //RooFormulaVar oneMinus_epsSF_sb_ldp ("oneMinus_epsSF_sb_ldp","1.0-@0",RooArgList(rrv_epsSF_sb_ldp));
      //RooProduct mu_pass_epsSF_sb_ldp ("mu_pass_epsSF_sb_ldp", "mu_pass_epsSF_sb_ldp",RooArgList(rrv_epsSF_sb_ldp,mu_epsSF_sb_ldp));
      //RooProduct mu_fail_epsSF_sb_ldp ("mu_fail_epsSF_sb_ldp", "mu_fail_epsSF_sb_ldp",RooArgList(oneMinus_epsSF_sb_ldp,mu_epsSF_sb_ldp));
      //RooPoissonDummy pdf_pass_epsSF_sb_ldp ("pdf_pass_epsSF_sb_ldp" , "pdf_pass_epsSF_sb_ldp", N_pass_epsSF_sb_ldp, mu_pass_epsSF_sb_ldp);
      //allNuisancePdfs.add(pdf_pass_epsSF_sb_ldp);
      //RooPoissonDummy pdf_fail_epsSF_sb_ldp ("pdf_fail_epsSF_sb_ldp" , "pdf_fail_epsSF_sb_ldp", N_fail_epsSF_sb_ldp, mu_fail_epsSF_sb_ldp);
      //allNuisancePdfs.add(pdf_fail_epsSF_sb_ldp);

      RooRealVar N_pass_epsSF_sb_ldp ("N_pass_epsSF_sb_ldp", "N_pass_epsSF_sb_ldp", alpha-1,0,1e5);
      observedParametersList.add( N_pass_epsSF_sb_ldp );
      RooRealVar mu_pass_epsSF_sb_ldp ("mu_pass_epsSF_sb_ldp", "mu_pass_epsSF_sb_ldp", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_epsSF_sb_ldp);
      RooPoisson pdf_pass_epsSF_sb_ldp ("pdf_pass_epsSF_sb_ldp" , "pdf_pass_epsSF_sb_ldp", N_pass_epsSF_sb_ldp, mu_pass_epsSF_sb_ldp);
      allNuisancePdfs.add(pdf_pass_epsSF_sb_ldp);
      RooRealVar N_fail_epsSF_sb_ldp ("N_fail_epsSF_sb_ldp", "N_fail_epsSF_sb_ldp", beta-1,0,1e5);
      observedParametersList.add( N_fail_epsSF_sb_ldp );
      RooRealVar mu_fail_epsSF_sb_ldp ("mu_fail_epsSF_sb_ldp", "mu_fail_epsSF_sb_ldp", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_epsSF_sb_ldp);
      RooPoisson pdf_fail_epsSF_sb_ldp ("pdf_fail_epsSF_sb_ldp" , "pdf_fail_epsSF_sb_ldp", N_fail_epsSF_sb_ldp, mu_fail_epsSF_sb_ldp);
      allNuisancePdfs.add(pdf_fail_epsSF_sb_ldp);
      
      RooFormulaVar rrv_epsSF_sb_ldp ("epsSF_sb_ldp","@0/(@0+@1)",RooArgList(mu_pass_epsSF_sb_ldp,mu_fail_epsSF_sb_ldp));

      betaModeTransform(eps_sb_sl_e_mean-epsSF_sb_sl_e_errm+0.5*(epsSF_sb_sl_e_errm+epsSF_sb_sl_e_errp),epsSF_sb_sl_e_errm+epsSF_sb_sl_e_errp,alpha,beta);
      //RooRealVar N_pass_epsSF_sb_sl_e ("N_pass_epsSF_sb_sl_e", "N_pass_epsSF_sb_sl_e", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sb_sl_e );
      //RooRealVar N_fail_epsSF_sb_sl_e ("N_fail_epsSF_sb_sl_e", "N_fail_epsSF_sb_sl_e", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sb_sl_e );
      //RooRealVar rrv_epsSF_sb_sl_e ("epsSF_sb_sl_e","epsSF_sb_sl_e",eps_sb_sl_e_mean-epsSF_sb_sl_e_errm+0.5*(epsSF_sb_sl_e_errm+epsSF_sb_sl_e_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sb_sl_e);
      //RooBetaPdfWithPoissonGenerator pdf_epsSF_sb_sl_e ("pdf_epsSF_sb_sl_e" , "pdf_epsSF_sb_sl_e", rrv_epsSF_sb_sl_e, N_pass_epsSF_sb_sl_e, N_fail_epsSF_sb_sl_e);
      //allNuisancePdfs.add(pdf_epsSF_sb_sl_e);

      //RooRealVar rrv_epsSF_sb_sl_e ("epsSF_sb_sl_e","epsSF_sb_sl_e",eps_sb_sl_e_mean-epsSF_sb_sl_e_errm+0.5*(epsSF_sb_sl_e_errm+epsSF_sb_sl_e_errp),0,1);
      //RooRealVar epsSF_sb_sl_e_alpha ("epsSF_sb_sl_e_alpha", "epsSF_sb_sl_e_alpha", alpha);
      //RooRealVar epsSF_sb_sl_e_beta ("epsSF_sb_sl_e_beta", "epsSF_sb_sl_e_beta", beta);
      //RooBetaPdf pdf_epsSF_sb_sl_e ("pdf_epsSF_sb_sl_e" , "pdf_epsSF_sb_sl_e", rrv_epsSF_sb_sl_e, epsSF_sb_sl_e_alpha, epsSF_sb_sl_e_beta);
      //epsSF_sb_sl_e_alpha.setConstant();
      //epsSF_sb_sl_e_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_epsSF_sb_sl_e);
      //if(constantNonPoisson) rrv_epsSF_sb_sl_e.setConstant();
      //betaNuisancePdfs.add (pdf_epsSF_sb_sl_e);

      //RooRealVar N_pass_epsSF_sb_sl_e ("N_pass_epsSF_sb_sl_e", "N_pass_epsSF_sb_sl_e", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sb_sl_e );
      //RooRealVar N_fail_epsSF_sb_sl_e ("N_fail_epsSF_sb_sl_e", "N_fail_epsSF_sb_sl_e", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sb_sl_e );
      //RooAddition mu_epsSF_sb_sl_e ("mu_epsSF_sb_sl_e", "mu_epsSF_sb_sl_e", RooArgList(N_pass_epsSF_sb_sl_e,N_fail_epsSF_sb_sl_e));
      //RooRealVar rrv_epsSF_sb_sl_e ("epsSF_sb_sl_e","epsSF_sb_sl_e",eps_sig_mean-epsSF_sb_sl_e_errm+0.5*(epsSF_sb_sl_e_errm+epsSF_sb_sl_e_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sb_sl_e);
      //RooFormulaVar oneMinus_epsSF_sb_sl_e ("oneMinus_epsSF_sb_sl_e","1.0-@0",RooArgList(rrv_epsSF_sb_sl_e));
      //RooProduct mu_pass_epsSF_sb_sl_e ("mu_pass_epsSF_sb_sl_e", "mu_pass_epsSF_sb_sl_e",RooArgList(rrv_epsSF_sb_sl_e,mu_epsSF_sb_sl_e));
      //RooProduct mu_fail_epsSF_sb_sl_e ("mu_fail_epsSF_sb_sl_e", "mu_fail_epsSF_sb_sl_e",RooArgList(oneMinus_epsSF_sb_sl_e,mu_epsSF_sb_sl_e));
      //RooPoissonDummy pdf_pass_epsSF_sb_sl_e ("pdf_pass_epsSF_sb_sl_e" , "pdf_pass_epsSF_sb_sl_e", N_pass_epsSF_sb_sl_e, mu_pass_epsSF_sb_sl_e);
      //allNuisancePdfs.add(pdf_pass_epsSF_sb_sl_e);
      //RooPoissonDummy pdf_fail_epsSF_sb_sl_e ("pdf_fail_epsSF_sb_sl_e" , "pdf_fail_epsSF_sb_sl_e", N_fail_epsSF_sb_sl_e, mu_fail_epsSF_sb_sl_e);
      //allNuisancePdfs.add(pdf_fail_epsSF_sb_sl_e);

      RooRealVar N_pass_epsSF_sb_sl_e ("N_pass_epsSF_sb_sl_e", "N_pass_epsSF_sb_sl_e", alpha-1,0,1e5);
      observedParametersList.add( N_pass_epsSF_sb_sl_e );
      RooRealVar mu_pass_epsSF_sb_sl_e ("mu_pass_epsSF_sb_sl_e", "mu_pass_epsSF_sb_sl_e", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_epsSF_sb_sl_e);
      RooPoisson pdf_pass_epsSF_sb_sl_e ("pdf_pass_epsSF_sb_sl_e" , "pdf_pass_epsSF_sb_sl_e", N_pass_epsSF_sb_sl_e, mu_pass_epsSF_sb_sl_e);
      allNuisancePdfs.add(pdf_pass_epsSF_sb_sl_e);
      RooRealVar N_fail_epsSF_sb_sl_e ("N_fail_epsSF_sb_sl_e", "N_fail_epsSF_sb_sl_e", beta-1,0,1e5);
      observedParametersList.add( N_fail_epsSF_sb_sl_e );
      RooRealVar mu_fail_epsSF_sb_sl_e ("mu_fail_epsSF_sb_sl_e", "mu_fail_epsSF_sb_sl_e", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_epsSF_sb_sl_e);
      RooPoisson pdf_fail_epsSF_sb_sl_e ("pdf_fail_epsSF_sb_sl_e" , "pdf_fail_epsSF_sb_sl_e", N_fail_epsSF_sb_sl_e, mu_fail_epsSF_sb_sl_e);
      allNuisancePdfs.add(pdf_fail_epsSF_sb_sl_e);
      
      RooFormulaVar rrv_epsSF_sb_sl_e ("epsSF_sb_sl_e","@0/(@0+@1)",RooArgList(mu_pass_epsSF_sb_sl_e,mu_fail_epsSF_sb_sl_e));

      betaModeTransform(eps_sb_sl_mu_mean-epsSF_sb_sl_mu_errm+0.5*(epsSF_sb_sl_mu_errm+epsSF_sb_sl_mu_errp),epsSF_sb_sl_mu_errm+epsSF_sb_sl_mu_errp,alpha,beta);
      //RooRealVar N_pass_epsSF_sb_sl_mu ("N_pass_epsSF_sb_sl_mu", "N_pass_epsSF_sb_sl_mu", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sb_sl_mu );
      //RooRealVar N_fail_epsSF_sb_sl_mu ("N_fail_epsSF_sb_sl_mu", "N_fail_epsSF_sb_sl_mu", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sb_sl_mu );
      //RooRealVar rrv_epsSF_sb_sl_mu ("epsSF_sb_sl_mu","epsSF_sb_sl_mu",eps_sb_sl_mu_mean-epsSF_sb_sl_mu_errm+0.5*(epsSF_sb_sl_mu_errm+epsSF_sb_sl_mu_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sb_sl_mu);
      //RooBetaPdfWithPoissonGenerator pdf_epsSF_sb_sl_mu ("pdf_epsSF_sb_sl_mu" , "pdf_epsSF_sb_sl_mu", rrv_epsSF_sb_sl_mu, N_pass_epsSF_sb_sl_mu, N_fail_epsSF_sb_sl_mu);
      //allNuisancePdfs.add(pdf_epsSF_sb_sl_mu);

      //RooRealVar rrv_epsSF_sb_sl_mu ("epsSF_sb_sl_mu","epsSF_sb_sl_mu",eps_sb_sl_mu_mean-epsSF_sb_sl_mu_errm+0.5*(epsSF_sb_sl_mu_errm+epsSF_sb_sl_mu_errp),0,1);
      //RooRealVar epsSF_sb_sl_mu_alpha ("epsSF_sb_sl_mu_alpha", "epsSF_sb_sl_mu_alpha", alpha);
      //RooRealVar epsSF_sb_sl_mu_beta ("epsSF_sb_sl_mu_beta", "epsSF_sb_sl_mu_beta", beta);
      //RooBetaPdf pdf_epsSF_sb_sl_mu ("pdf_epsSF_sb_sl_mu" , "pdf_epsSF_sb_sl_mu", rrv_epsSF_sb_sl_mu, epsSF_sb_sl_mu_alpha, epsSF_sb_sl_mu_beta);
      //epsSF_sb_sl_mu_alpha.setConstant();
      //epsSF_sb_sl_mu_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_epsSF_sb_sl_mu);
      //if(constantNonPoisson) rrv_epsSF_sb_sl_mu.setConstant();
      //betaNuisancePdfs.add (pdf_epsSF_sb_sl_mu);

      //RooRealVar N_pass_epsSF_sb_sl_mu ("N_pass_epsSF_sb_sl_mu", "N_pass_epsSF_sb_sl_mu", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sb_sl_mu );
      //RooRealVar N_fail_epsSF_sb_sl_mu ("N_fail_epsSF_sb_sl_mu", "N_fail_epsSF_sb_sl_mu", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sb_sl_mu );
      //RooAddition mu_epsSF_sb_sl_mu ("mu_epsSF_sb_sl_mu", "mu_epsSF_sb_sl_mu", RooArgList(N_pass_epsSF_sb_sl_mu,N_fail_epsSF_sb_sl_mu));
      //RooRealVar rrv_epsSF_sb_sl_mu ("epsSF_sb_sl_mu","epsSF_sb_sl_mu",eps_sig_mean-epsSF_sb_sl_mu_errm+0.5*(epsSF_sb_sl_mu_errm+epsSF_sb_sl_mu_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sb_sl_mu);
      //RooFormulaVar oneMinus_epsSF_sb_sl_mu ("oneMinus_epsSF_sb_sl_mu","1.0-@0",RooArgList(rrv_epsSF_sb_sl_mu));
      //RooProduct mu_pass_epsSF_sb_sl_mu ("mu_pass_epsSF_sb_sl_mu", "mu_pass_epsSF_sb_sl_mu",RooArgList(rrv_epsSF_sb_sl_mu,mu_epsSF_sb_sl_mu));
      //RooProduct mu_fail_epsSF_sb_sl_mu ("mu_fail_epsSF_sb_sl_mu", "mu_fail_epsSF_sb_sl_mu",RooArgList(oneMinus_epsSF_sb_sl_mu,mu_epsSF_sb_sl_mu));
      //RooPoissonDummy pdf_pass_epsSF_sb_sl_mu ("pdf_pass_epsSF_sb_sl_mu" , "pdf_pass_epsSF_sb_sl_mu", N_pass_epsSF_sb_sl_mu, mu_pass_epsSF_sb_sl_mu);
      //allNuisancePdfs.add(pdf_pass_epsSF_sb_sl_mu);
      //RooPoissonDummy pdf_fail_epsSF_sb_sl_mu ("pdf_fail_epsSF_sb_sl_mu" , "pdf_fail_epsSF_sb_sl_mu", N_fail_epsSF_sb_sl_mu, mu_fail_epsSF_sb_sl_mu);
      //allNuisancePdfs.add(pdf_fail_epsSF_sb_sl_mu);

      RooRealVar N_pass_epsSF_sb_sl_mu ("N_pass_epsSF_sb_sl_mu", "N_pass_epsSF_sb_sl_mu", alpha-1,0,1e5);
      observedParametersList.add( N_pass_epsSF_sb_sl_mu );
      RooRealVar mu_pass_epsSF_sb_sl_mu ("mu_pass_epsSF_sb_sl_mu", "mu_pass_epsSF_sb_sl_mu", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_epsSF_sb_sl_mu);
      RooPoisson pdf_pass_epsSF_sb_sl_mu ("pdf_pass_epsSF_sb_sl_mu" , "pdf_pass_epsSF_sb_sl_mu", N_pass_epsSF_sb_sl_mu, mu_pass_epsSF_sb_sl_mu);
      allNuisancePdfs.add(pdf_pass_epsSF_sb_sl_mu);
      RooRealVar N_fail_epsSF_sb_sl_mu ("N_fail_epsSF_sb_sl_mu", "N_fail_epsSF_sb_sl_mu", beta-1,0,1e5);
      observedParametersList.add( N_fail_epsSF_sb_sl_mu );
      RooRealVar mu_fail_epsSF_sb_sl_mu ("mu_fail_epsSF_sb_sl_mu", "mu_fail_epsSF_sb_sl_mu", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_epsSF_sb_sl_mu);
      RooPoisson pdf_fail_epsSF_sb_sl_mu ("pdf_fail_epsSF_sb_sl_mu" , "pdf_fail_epsSF_sb_sl_mu", N_fail_epsSF_sb_sl_mu, mu_fail_epsSF_sb_sl_mu);
      allNuisancePdfs.add(pdf_fail_epsSF_sb_sl_mu);
      
      RooFormulaVar rrv_epsSF_sb_sl_mu ("epsSF_sb_sl_mu","@0/(@0+@1)",RooArgList(mu_pass_epsSF_sb_sl_mu,mu_fail_epsSF_sb_sl_mu));

      betaModeTransform(eps_sig_sl_mean-epsSF_sig_sl_errm+0.5*(epsSF_sig_sl_errm+epsSF_sig_sl_errp),epsSF_sig_sl_errm+epsSF_sig_sl_errp,alpha,beta);
      //RooRealVar N_pass_epsSF_sig_sl ("N_pass_epsSF_sig_sl", "N_pass_epsSF_sig_sl", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sig_sl );
      //RooRealVar N_fail_epsSF_sig_sl ("N_fail_epsSF_sig_sl", "N_fail_epsSF_sig_sl", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sig_sl );
      //RooRealVar rrv_epsSF_sig_sl ("epsSF_sig_sl","epsSF_sig_sl",eps_sig_sl_mean-epsSF_sig_sl_errm+0.5*(epsSF_sig_sl_errm+epsSF_sig_sl_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sig_sl);
      //RooBetaPdfWithPoissonGenerator pdf_epsSF_sig_sl ("pdf_epsSF_sig_sl" , "pdf_epsSF_sig_sl", rrv_epsSF_sig_sl, N_pass_epsSF_sig_sl, N_fail_epsSF_sig_sl);
      //allNuisancePdfs.add(pdf_epsSF_sig_sl);

      //RooRealVar rrv_epsSF_sig_sl ("epsSF_sig_sl","epsSF_sig_sl",eps_sig_sl_mean-epsSF_sig_sl_errm+0.5*(epsSF_sig_sl_errm+epsSF_sig_sl_errp),0,1);
      //RooRealVar epsSF_sig_sl_alpha ("epsSF_sig_sl_alpha", "epsSF_sig_sl_alpha", alpha);
      //RooRealVar epsSF_sig_sl_beta ("epsSF_sig_sl_beta", "epsSF_sig_sl_beta", beta);
      //RooBetaPdf pdf_epsSF_sig_sl ("pdf_epsSF_sig_sl" , "pdf_epsSF_sig_sl", rrv_epsSF_sig_sl, epsSF_sig_sl_alpha, epsSF_sig_sl_beta);
      //epsSF_sig_sl_alpha.setConstant();
      //epsSF_sig_sl_beta.setConstant();
      //allNonPoissonNuisances.add (rrv_epsSF_sig_sl);
      //if(constantNonPoisson) rrv_epsSF_sig_sl.setConstant();
      //betaNuisancePdfs.add (pdf_epsSF_sig_sl);

      //RooRealVar N_pass_epsSF_sig_sl ("N_pass_epsSF_sig_sl", "N_pass_epsSF_sig_sl", alpha-1,0,1e5);
      //observedParametersList.add( N_pass_epsSF_sig_sl );
      //RooRealVar N_fail_epsSF_sig_sl ("N_fail_epsSF_sig_sl", "N_fail_epsSF_sig_sl", beta-1,0,1e5);
      //observedParametersList.add( N_fail_epsSF_sig_sl );
      //RooAddition mu_epsSF_sig_sl ("mu_epsSF_sig_sl", "mu_epsSF_sig_sl", RooArgList(N_pass_epsSF_sig_sl,N_fail_epsSF_sig_sl));
      //RooRealVar rrv_epsSF_sig_sl ("epsSF_sig_sl","epsSF_sig_sl",eps_sig_mean-epsSF_sig_sl_errm+0.5*(epsSF_sig_sl_errm+epsSF_sig_sl_errp),0,1);
      //allNonPoissonNuisances.add (rrv_epsSF_sig_sl);
      //RooFormulaVar oneMinus_epsSF_sig_sl ("oneMinus_epsSF_sig_sl","1.0-@0",RooArgList(rrv_epsSF_sig_sl));
      //RooProduct mu_pass_epsSF_sig_sl ("mu_pass_epsSF_sig_sl", "mu_pass_epsSF_sig_sl",RooArgList(rrv_epsSF_sig_sl,mu_epsSF_sig_sl));
      //RooProduct mu_fail_epsSF_sig_sl ("mu_fail_epsSF_sig_sl", "mu_fail_epsSF_sig_sl",RooArgList(oneMinus_epsSF_sig_sl,mu_epsSF_sig_sl));
      //RooPoissonDummy pdf_pass_epsSF_sig_sl ("pdf_pass_epsSF_sig_sl" , "pdf_pass_epsSF_sig_sl", N_pass_epsSF_sig_sl, mu_pass_epsSF_sig_sl);
      //allNuisancePdfs.add(pdf_pass_epsSF_sig_sl);
      //RooPoissonDummy pdf_fail_epsSF_sig_sl ("pdf_fail_epsSF_sig_sl" , "pdf_fail_epsSF_sig_sl", N_fail_epsSF_sig_sl, mu_fail_epsSF_sig_sl);
      //allNuisancePdfs.add(pdf_fail_epsSF_sig_sl);

      RooRealVar N_pass_epsSF_sig_sl ("N_pass_epsSF_sig_sl", "N_pass_epsSF_sig_sl", alpha-1,0,1e5);
      observedParametersList.add( N_pass_epsSF_sig_sl );
      RooRealVar mu_pass_epsSF_sig_sl ("mu_pass_epsSF_sig_sl", "mu_pass_epsSF_sig_sl", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_epsSF_sig_sl);
      RooPoisson pdf_pass_epsSF_sig_sl ("pdf_pass_epsSF_sig_sl" , "pdf_pass_epsSF_sig_sl", N_pass_epsSF_sig_sl, mu_pass_epsSF_sig_sl);
      allNuisancePdfs.add(pdf_pass_epsSF_sig_sl);
      RooRealVar N_fail_epsSF_sig_sl ("N_fail_epsSF_sig_sl", "N_fail_epsSF_sig_sl", beta-1,0,1e5);
      observedParametersList.add( N_fail_epsSF_sig_sl );
      RooRealVar mu_fail_epsSF_sig_sl ("mu_fail_epsSF_sig_sl", "mu_fail_epsSF_sig_sl", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_epsSF_sig_sl);
      RooPoisson pdf_fail_epsSF_sig_sl ("pdf_fail_epsSF_sig_sl" , "pdf_fail_epsSF_sig_sl", N_fail_epsSF_sig_sl, mu_fail_epsSF_sig_sl);
      allNuisancePdfs.add(pdf_fail_epsSF_sig_sl);
      
      RooFormulaVar rrv_epsSF_sig_sl ("epsSF_sig_sl","@0/(@0+@1)",RooArgList(mu_pass_epsSF_sig_sl,mu_fail_epsSF_sig_sl));


    //--------------------------------------

      // Underlying Gamma Distribution for signal systematics -- taken from average of all errors.

      double sigma_eff_sf_sig        = rv_width_eff_sf_sig       ->getVal();
      double sigma_eff_sf_sb         = rv_width_eff_sf_sb	 ->getVal();
      double sigma_eff_sf_sig_sl_e   = rv_width_eff_sf_sig_sl_e  ->getVal();
      double sigma_eff_sf_sig_sl_mu  = rv_width_eff_sf_sig_sl_mu ->getVal();
      double sigma_eff_sf_sb_sl_e    = rv_width_eff_sf_sb_sl_e   ->getVal();
      double sigma_eff_sf_sb_sl_mu   = rv_width_eff_sf_sb_sl_mu  ->getVal();
      double sigma_eff_sf_sig_ldp    = rv_width_eff_sf_sig_ldp   ->getVal();
      double sigma_eff_sf_sb_ldp     = rv_width_eff_sf_sb_ldp    ->getVal();

      //Trying with Beta Distributions for signal efficiency

      double mean_eff_sf_sig        = rv_mean_eff_sf_sig       ->getVal();
      double mean_eff_sf_sb         = rv_mean_eff_sf_sb	       ->getVal();
      double mean_eff_sf_sig_sl_e   = rv_mean_eff_sf_sig_sl_e  ->getVal();
      double mean_eff_sf_sig_sl_mu  = rv_mean_eff_sf_sig_sl_mu ->getVal();
      double mean_eff_sf_sb_sl_e    = rv_mean_eff_sf_sb_sl_e   ->getVal();
      double mean_eff_sf_sb_sl_mu   = rv_mean_eff_sf_sb_sl_mu  ->getVal();
      double mean_eff_sf_sig_ldp    = rv_mean_eff_sf_sig_ldp   ->getVal();
      double mean_eff_sf_sb_ldp     = rv_mean_eff_sf_sb_ldp    ->getVal();

      double alpha_eff_sf_sig       ;
      double alpha_eff_sf_sb        ;
      double alpha_eff_sf_sig_sl_e  ;
      double alpha_eff_sf_sig_sl_mu ;
      double alpha_eff_sf_sb_sl_e   ;
      double alpha_eff_sf_sb_sl_mu  ;
      double alpha_eff_sf_sig_ldp   ;
      double alpha_eff_sf_sb_ldp    ;

      double beta_eff_sf_sig       ;
      double beta_eff_sf_sb        ;
      double beta_eff_sf_sig_sl_e  ;
      double beta_eff_sf_sig_sl_mu ;
      double beta_eff_sf_sb_sl_e   ;
      double beta_eff_sf_sb_sl_mu  ;
      double beta_eff_sf_sig_ldp   ;
      double beta_eff_sf_sb_ldp    ;

      betaPrimeModeTransform( mean_eff_sf_sig       , mean_eff_sf_sig       * sigma_eff_sf_sig       , alpha_eff_sf_sig       , beta_eff_sf_sig       );
      betaPrimeModeTransform( mean_eff_sf_sb        , mean_eff_sf_sb        * sigma_eff_sf_sb        , alpha_eff_sf_sb        , beta_eff_sf_sb        );
      betaPrimeModeTransform( mean_eff_sf_sig_sl_e  , mean_eff_sf_sig_sl_e  * sigma_eff_sf_sig_sl_e  , alpha_eff_sf_sig_sl_e  , beta_eff_sf_sig_sl_e  );
      betaPrimeModeTransform( mean_eff_sf_sig_sl_mu , mean_eff_sf_sig_sl_mu * sigma_eff_sf_sig_sl_mu , alpha_eff_sf_sig_sl_mu , beta_eff_sf_sig_sl_mu );
      betaPrimeModeTransform( mean_eff_sf_sb_sl_e   , mean_eff_sf_sb_sl_e   * sigma_eff_sf_sb_sl_e   , alpha_eff_sf_sb_sl_e   , beta_eff_sf_sb_sl_e   );
      betaPrimeModeTransform( mean_eff_sf_sb_sl_mu  , mean_eff_sf_sb_sl_mu  * sigma_eff_sf_sb_sl_mu  , alpha_eff_sf_sb_sl_mu  , beta_eff_sf_sb_sl_mu  );
      betaPrimeModeTransform( mean_eff_sf_sig_ldp   , mean_eff_sf_sig_ldp   * sigma_eff_sf_sig_ldp   , alpha_eff_sf_sig_ldp   , beta_eff_sf_sig_ldp   );
      betaPrimeModeTransform( mean_eff_sf_sb_ldp    , mean_eff_sf_sb_ldp    * sigma_eff_sf_sb_ldp    , alpha_eff_sf_sb_ldp    , beta_eff_sf_sb_ldp    );

      alpha = max(alpha_eff_sf_sig,
		  max(alpha_eff_sf_sb,
		      max(alpha_eff_sf_sig_sl_e,
			  max(alpha_eff_sf_sig_sl_mu,
			      max(alpha_eff_sf_sb_sl_e,
				  max(alpha_eff_sf_sb_sl_mu,
				      max(alpha_eff_sf_sig_ldp,
					  alpha_eff_sf_sb_ldp)))))));
      
      beta = max(beta_eff_sf_sig,
		 max(beta_eff_sf_sb,
		     max(beta_eff_sf_sig_sl_e,
			 max(beta_eff_sf_sig_sl_mu,
			     max(beta_eff_sf_sb_sl_e,
				 max(beta_eff_sf_sb_sl_mu,
				     max(beta_eff_sf_sig_ldp,
					 beta_eff_sf_sb_ldp)))))));

      RooRealVar N_pass_scale_eff_sf_sig       ("N_pass_scale_eff_sf_sig"       , "N_pass_scale_eff_sf_sig"       , (alpha_eff_sf_sig      -1) / (alpha-1) );
      RooRealVar N_pass_scale_eff_sf_sb        ("N_pass_scale_eff_sf_sb"        , "N_pass_scale_eff_sf_sb"        , (alpha_eff_sf_sb       -1) / (alpha-1) );
      RooRealVar N_pass_scale_eff_sf_sig_sl_e  ("N_pass_scale_eff_sf_sig_sl_e"  , "N_pass_scale_eff_sf_sig_sl_e"  , (alpha_eff_sf_sig_sl_e -1) / (alpha-1) );
      RooRealVar N_pass_scale_eff_sf_sig_sl_mu ("N_pass_scale_eff_sf_sig_sl_mu" , "N_pass_scale_eff_sf_sig_sl_mu" , (alpha_eff_sf_sig_sl_mu-1) / (alpha-1) );
      RooRealVar N_pass_scale_eff_sf_sb_sl_e   ("N_pass_scale_eff_sf_sb_sl_e"   , "N_pass_scale_eff_sf_sb_sl_e"   , (alpha_eff_sf_sb_sl_e  -1) / (alpha-1) );
      RooRealVar N_pass_scale_eff_sf_sb_sl_mu  ("N_pass_scale_eff_sf_sb_sl_mu"  , "N_pass_scale_eff_sf_sb_sl_mu"  , (alpha_eff_sf_sb_sl_mu -1) / (alpha-1) );
      RooRealVar N_pass_scale_eff_sf_sig_ldp   ("N_pass_scale_eff_sf_sig_ldp"   , "N_pass_scale_eff_sf_sig_ldp"   , (alpha_eff_sf_sig_ldp  -1) / (alpha-1) );
      RooRealVar N_pass_scale_eff_sf_sb_ldp    ("N_pass_scale_eff_sf_sb_ldp"    , "N_pass_scale_eff_sf_sb_ldp"    , (alpha_eff_sf_sb_ldp   -1) / (alpha-1) );
      RooRealVar N_fail_scale_eff_sf_sig       ("N_fail_scale_eff_sf_sig"       , "N_fail_scale_eff_sf_sig"       , (beta_eff_sf_sig       -1) / (beta -1) );
      RooRealVar N_fail_scale_eff_sf_sb        ("N_fail_scale_eff_sf_sb"        , "N_fail_scale_eff_sf_sb"        , (beta_eff_sf_sb        -1) / (beta -1) );
      RooRealVar N_fail_scale_eff_sf_sig_sl_e  ("N_fail_scale_eff_sf_sig_sl_e"  , "N_fail_scale_eff_sf_sig_sl_e"  , (beta_eff_sf_sig_sl_e  -1) / (beta -1) );
      RooRealVar N_fail_scale_eff_sf_sig_sl_mu ("N_fail_scale_eff_sf_sig_sl_mu" , "N_fail_scale_eff_sf_sig_sl_mu" , (beta_eff_sf_sig_sl_mu -1) / (beta -1) );
      RooRealVar N_fail_scale_eff_sf_sb_sl_e   ("N_fail_scale_eff_sf_sb_sl_e"   , "N_fail_scale_eff_sf_sb_sl_e"   , (beta_eff_sf_sb_sl_e   -1) / (beta -1) );
      RooRealVar N_fail_scale_eff_sf_sb_sl_mu  ("N_fail_scale_eff_sf_sb_sl_mu"  , "N_fail_scale_eff_sf_sb_sl_mu"  , (beta_eff_sf_sb_sl_mu  -1) / (beta -1) );
      RooRealVar N_fail_scale_eff_sf_sig_ldp   ("N_fail_scale_eff_sf_sig_ldp"   , "N_fail_scale_eff_sf_sig_ldp"   , (beta_eff_sf_sig_ldp   -1) / (beta -1) );
      RooRealVar N_fail_scale_eff_sf_sb_ldp    ("N_fail_scale_eff_sf_sb_ldp"    , "N_fail_scale_eff_sf_sb_ldp"    , (beta_eff_sf_sb_ldp    -1) / (beta -1) );

      N_pass_scale_eff_sf_sig       .setConstant();
      N_pass_scale_eff_sf_sb        .setConstant();
      N_pass_scale_eff_sf_sig_sl_e  .setConstant();
      N_pass_scale_eff_sf_sig_sl_mu .setConstant();
      N_pass_scale_eff_sf_sb_sl_e   .setConstant();
      N_pass_scale_eff_sf_sb_sl_mu  .setConstant();
      N_pass_scale_eff_sf_sig_ldp   .setConstant();
      N_pass_scale_eff_sf_sb_ldp    .setConstant();                                    
      N_fail_scale_eff_sf_sig       .setConstant();
      N_fail_scale_eff_sf_sb        .setConstant();
      N_fail_scale_eff_sf_sig_sl_e  .setConstant();
      N_fail_scale_eff_sf_sig_sl_mu .setConstant();
      N_fail_scale_eff_sf_sb_sl_e   .setConstant();
      N_fail_scale_eff_sf_sb_sl_mu  .setConstant();
      N_fail_scale_eff_sf_sig_ldp   .setConstant();
      N_fail_scale_eff_sf_sb_ldp    .setConstant();

      RooRealVar N_pass_eff_sf ("N_pass_eff_sf", "N_pass_eff_sf", alpha-1,0,1e5);
      observedParametersList.add( N_pass_eff_sf );
      RooRealVar mu_pass_eff_sf ("mu_pass_eff_sf", "mu_pass_eff_sf", alpha-1,1e-9,1e5);
      allPoissonNuisances.add(mu_pass_eff_sf);
      RooPoisson pdf_pass_eff_sf ("pdf_pass_eff_sf" , "pdf_pass_eff_sf", N_pass_eff_sf, mu_pass_eff_sf);
      allNuisancePdfs.add(pdf_pass_eff_sf);
      RooRealVar N_fail_eff_sf ("N_fail_eff_sf", "N_fail_eff_sf", beta-1,0,1e5);
      observedParametersList.add( N_fail_eff_sf );
      RooRealVar mu_fail_eff_sf ("mu_fail_eff_sf", "mu_fail_eff_sf", beta-1,1e-9,1e5);
      allPoissonNuisances.add(mu_fail_eff_sf);
      RooPoisson pdf_fail_eff_sf ("pdf_fail_eff_sf" , "pdf_fail_eff_sf", N_fail_eff_sf, mu_fail_eff_sf);
      allNuisancePdfs.add(pdf_fail_eff_sf);

      RooProduct mu_pass_eff_sf_sig       ("mu_pass_eff_sf_sig"       , "mu_pass_eff_sf_sig"       , RooArgSet(N_pass_scale_eff_sf_sig       , mu_pass_eff_sf));
      RooProduct mu_pass_eff_sf_sb        ("mu_pass_eff_sf_sb"        , "mu_pass_eff_sf_sb"        , RooArgSet(N_pass_scale_eff_sf_sb        , mu_pass_eff_sf));
      RooProduct mu_pass_eff_sf_sig_sl_e  ("mu_pass_eff_sf_sig_sl_e"  , "mu_pass_eff_sf_sig_sl_e"  , RooArgSet(N_pass_scale_eff_sf_sig_sl_e  , mu_pass_eff_sf));
      RooProduct mu_pass_eff_sf_sig_sl_mu ("mu_pass_eff_sf_sig_sl_mu" , "mu_pass_eff_sf_sig_sl_mu" , RooArgSet(N_pass_scale_eff_sf_sig_sl_mu , mu_pass_eff_sf));
      RooProduct mu_pass_eff_sf_sb_sl_e   ("mu_pass_eff_sf_sb_sl_e"   , "mu_pass_eff_sf_sb_sl_e"   , RooArgSet(N_pass_scale_eff_sf_sb_sl_e   , mu_pass_eff_sf));
      RooProduct mu_pass_eff_sf_sb_sl_mu  ("mu_pass_eff_sf_sb_sl_mu"  , "mu_pass_eff_sf_sb_sl_mu"  , RooArgSet(N_pass_scale_eff_sf_sb_sl_mu  , mu_pass_eff_sf));
      RooProduct mu_pass_eff_sf_sig_ldp   ("mu_pass_eff_sf_sig_ldp"   , "mu_pass_eff_sf_sig_ldp"   , RooArgSet(N_pass_scale_eff_sf_sig_ldp   , mu_pass_eff_sf));
      RooProduct mu_pass_eff_sf_sb_ldp    ("mu_pass_eff_sf_sb_ldp"    , "mu_pass_eff_sf_sb_ldp"    , RooArgSet(N_pass_scale_eff_sf_sb_ldp    , mu_pass_eff_sf));
      RooProduct mu_fail_eff_sf_sig       ("mu_fail_eff_sf_sig"       , "mu_fail_eff_sf_sig"       , RooArgSet(N_fail_scale_eff_sf_sig       , mu_fail_eff_sf));
      RooProduct mu_fail_eff_sf_sb        ("mu_fail_eff_sf_sb"        , "mu_fail_eff_sf_sb"        , RooArgSet(N_fail_scale_eff_sf_sb        , mu_fail_eff_sf));
      RooProduct mu_fail_eff_sf_sig_sl_e  ("mu_fail_eff_sf_sig_sl_e"  , "mu_fail_eff_sf_sig_sl_e"  , RooArgSet(N_fail_scale_eff_sf_sig_sl_e  , mu_fail_eff_sf));
      RooProduct mu_fail_eff_sf_sig_sl_mu ("mu_fail_eff_sf_sig_sl_mu" , "mu_fail_eff_sf_sig_sl_mu" , RooArgSet(N_fail_scale_eff_sf_sig_sl_mu , mu_fail_eff_sf));
      RooProduct mu_fail_eff_sf_sb_sl_e   ("mu_fail_eff_sf_sb_sl_e"   , "mu_fail_eff_sf_sb_sl_e"   , RooArgSet(N_fail_scale_eff_sf_sb_sl_e   , mu_fail_eff_sf));
      RooProduct mu_fail_eff_sf_sb_sl_mu  ("mu_fail_eff_sf_sb_sl_mu"  , "mu_fail_eff_sf_sb_sl_mu"  , RooArgSet(N_fail_scale_eff_sf_sb_sl_mu  , mu_fail_eff_sf));
      RooProduct mu_fail_eff_sf_sig_ldp   ("mu_fail_eff_sf_sig_ldp"   , "mu_fail_eff_sf_sig_ldp"   , RooArgSet(N_fail_scale_eff_sf_sig_ldp   , mu_fail_eff_sf));
      RooProduct mu_fail_eff_sf_sb_ldp    ("mu_fail_eff_sf_sb_ldp"    , "mu_fail_eff_sf_sb_ldp"    , RooArgSet(N_fail_scale_eff_sf_sb_ldp    , mu_fail_eff_sf));
      
      cout << "eff_sf section -- calculation of mu_pass" << endl;
      cout << "eff_sf section -- eff_sf_sig       has mu_pass of: " << mu_pass_eff_sf_sig       .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb        has mu_pass of: " << mu_pass_eff_sf_sb        .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sig_sl_e  has mu_pass of: " << mu_pass_eff_sf_sig_sl_e  .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sig_sl_mu has mu_pass of: " << mu_pass_eff_sf_sig_sl_mu .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb_sl_e   has mu_pass of: " << mu_pass_eff_sf_sb_sl_e   .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb_sl_mu  has mu_pass of: " << mu_pass_eff_sf_sb_sl_mu  .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sig_ldp   has mu_pass of: " << mu_pass_eff_sf_sig_ldp   .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb_ldp    has mu_pass of: " << mu_pass_eff_sf_sb_ldp    .getVal() << endl;

      cout << "eff_sf section -- calculation of mu_fail" << endl;
      cout << "eff_sf section -- eff_sf_sig       has mu_fail of: " << mu_fail_eff_sf_sig       .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb        has mu_fail of: " << mu_fail_eff_sf_sb        .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sig_sl_e  has mu_fail of: " << mu_fail_eff_sf_sig_sl_e  .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sig_sl_mu has mu_fail of: " << mu_fail_eff_sf_sig_sl_mu .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb_sl_e   has mu_fail of: " << mu_fail_eff_sf_sb_sl_e   .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb_sl_mu  has mu_fail of: " << mu_fail_eff_sf_sb_sl_mu  .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sig_ldp   has mu_fail of: " << mu_fail_eff_sf_sig_ldp   .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb_ldp    has mu_fail of: " << mu_fail_eff_sf_sb_ldp    .getVal() << endl;

      //-- Parametric relations between correlated signal efficiency scale factors.

      RooFormulaVar fv_eff_sf_sig       ("eff_sf_sig"         , "@0/(@1)" , RooArgList( mu_pass_eff_sf_sig       , mu_fail_eff_sf_sig       ));
      RooFormulaVar fv_eff_sf_sb        ("eff_sf_sb"          , "@0/(@1)" , RooArgList( mu_pass_eff_sf_sb        , mu_fail_eff_sf_sb        ));
      RooFormulaVar fv_eff_sf_sig_sl_e  ("eff_sf_sig_sl_e"    , "@0/(@1)" , RooArgList( mu_pass_eff_sf_sig_sl_e  , mu_fail_eff_sf_sig_sl_e  ));
      RooFormulaVar fv_eff_sf_sig_sl_mu ("eff_sf_sig_sl_mu"   , "@0/(@1)" , RooArgList( mu_pass_eff_sf_sig_sl_mu , mu_fail_eff_sf_sig_sl_mu ));
      RooFormulaVar fv_eff_sf_sb_sl_e   ("eff_sf_sb_sl_e"     , "@0/(@1)" , RooArgList( mu_pass_eff_sf_sb_sl_e   , mu_fail_eff_sf_sb_sl_e   ));
      RooFormulaVar fv_eff_sf_sb_sl_mu  ("eff_sf_sb_sl_mu"    , "@0/(@1)" , RooArgList( mu_pass_eff_sf_sb_sl_mu  , mu_fail_eff_sf_sb_sl_mu  ));
      RooFormulaVar fv_eff_sf_sig_ldp   ("eff_sf_sig_ldp"     , "@0/(@1)" , RooArgList( mu_pass_eff_sf_sig_ldp   , mu_fail_eff_sf_sig_ldp   ));
      RooFormulaVar fv_eff_sf_sb_ldp    ("eff_sf_sb_ldp"      , "@0/(@1)" , RooArgList( mu_pass_eff_sf_sb_ldp    , mu_fail_eff_sf_sb_ldp    ));

      cout << "eff_sf section -- calculation of efficiency" << endl;
      cout << "eff_sf section -- eff_sf_sig       has efficiency of: " << fv_eff_sf_sig       .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb        has efficiency of: " << fv_eff_sf_sb        .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sig_sl_e  has efficiency of: " << fv_eff_sf_sig_sl_e  .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sig_sl_mu has efficiency of: " << fv_eff_sf_sig_sl_mu .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb_sl_e   has efficiency of: " << fv_eff_sf_sb_sl_e   .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb_sl_mu  has efficiency of: " << fv_eff_sf_sb_sl_mu  .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sig_ldp   has efficiency of: " << fv_eff_sf_sig_ldp   .getVal() << endl;
      cout << "eff_sf section -- eff_sf_sb_ldp    has efficiency of: " << fv_eff_sf_sb_ldp    .getVal() << endl;

      //End Beta Distributions

      //double sigma_max = max(sigma_eff_sf_sig,
      //			     max(sigma_eff_sf_sb,
      //				 max(sigma_eff_sf_sig_sl_e,
      //				     max(sigma_eff_sf_sig_sl_mu,
      //					 max(sigma_eff_sf_sb_sl_e,
      //					     max(sigma_eff_sf_sb_sl_mu,
      //						 max(sigma_eff_sf_sig_ldp,
      //						     sigma_eff_sf_sb_ldp)))))));
      //
      //sigma_avg =(sigma_eff_sf_sig + sigma_eff_sf_sb + sigma_eff_sf_sig_sl_e + sigma_eff_sf_sig_sl_mu + sigma_eff_sf_sb_sl_e + sigma_eff_sf_sb_sl_mu + sigma_eff_sf_sig_ldp + sigma_eff_sf_sb_ldp)/8.;
      //mu_avg = 1.;
      //
      //gammaModeTransform(mu_avg,sigma_avg,k,theta);

      //RooRealVar rrv_eff_sf ("eff_sf","eff_sf",mu_avg,0,1e5);
      //RooRealVar eff_sf_k ("eff_sf_k", "eff_sf_k", k);
      //RooRealVar eff_sf_theta ("eff_sf_theta", "eff_sf_theta", theta);
      //RooGammaPdf pdf_eff_sf ("pdf_eff_sf" , "pdf_eff_sf", rrv_eff_sf, eff_sf_k, eff_sf_theta, RooConst(0));
      //eff_sf_k.setConstant();
      //eff_sf_theta.setConstant();
      //if(!effInitFixed) allNonPoissonNuisances.add (rrv_eff_sf); 
      //if(constantSigEff) rrv_eff_sf.setConstant();
      //gammaNuisancePdfs.add (pdf_eff_sf);

      //RooRealVar rrv_eff_sf ("eff_sf","eff_sf",mu_avg,0,1e5);
      //allNonPoissonNuisances.add (rrv_eff_sf);
      //RooRealVar eff_sf_invScale("eff_sf_invScale","eff_sf_invScale",1.0/theta);
      //eff_sf_invScale.setConstant();
      //RooProduct mu_comp_eff_sf("mu_comp_eff_sf","mu_comp_eff_sf",RooArgList(rrv_eff_sf,eff_sf_invScale));
      //RooRealVar N_comp_eff_sf ("N_comp_eff_sf", "N_comp_eff_sf", k-1,0,1e5);
      //observedParametersList.add( N_comp_eff_sf );
      ////RooRealVar mu_comp_eff_sf ("mu_comp_eff_sf", "mu_comp_eff_sf", k-1,1e-9,1e5);
      ////allPoissonNuisances.add(mu_comp_eff_sf);
      //RooPoisson pdf_eff_sf ("pdf_eff_sf" , "pdf_eff_sf", N_comp_eff_sf, mu_comp_eff_sf);
      //allNuisancePdfs.add(pdf_eff_sf);
      //
      ////RooRealVar eff_sf_scale("eff_sf_scale","eff_sf_scale",theta);
      ////eff_sf_scale.setConstant();
      ////RooProduct rp_eff_sf ("eff_sf","eff_sf",RooArgList(mu_comp_eff_sf,eff_sf_scale));
      //
      //alphaFinding.setCurrentSigma(sigma_avg);
      //
      //if(sigma_eff_sf_sig > 0){
      //	alphaFinding.setDesiredSigma(sigma_eff_sf_sig);
      //	alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*sigma_max/sigma_avg);
      //	alphaRoot.Solve();
      //}
      //RooRealVar eff_sf_sig_alpha("eff_sf_sig_alpha","eff_sf_sig_alpha", (sigma_eff_sf_sig > 0) ? alphaRoot.Root() : 0 );
      //eff_sf_sig_alpha.setConstant();
      //
      //if(sigma_eff_sf_sb > 0){
      //	alphaFinding.setDesiredSigma(sigma_eff_sf_sb);
      //	alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*sigma_max/sigma_avg);
      //	alphaRoot.Solve();
      //}
      //RooRealVar eff_sf_sb_alpha("eff_sf_sb_alpha","eff_sf_sb_alpha", (sigma_eff_sf_sb > 0) ? alphaRoot.Root() : 0 );
      //eff_sf_sb_alpha.setConstant();
      //
      //if(sigma_eff_sf_sig_sl_e > 0){
      //	alphaFinding.setDesiredSigma(sigma_eff_sf_sig_sl_e);
      //	alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*sigma_max/sigma_avg);
      //	alphaRoot.Solve();
      //}
      //RooRealVar eff_sf_sig_sl_e_alpha("eff_sf_sig_sl_e_alpha","eff_sf_sig_sl_e_alpha", (sigma_eff_sf_sig_sl_e > 0) ? alphaRoot.Root() : 0 );
      //eff_sf_sig_sl_e_alpha.setConstant();
      //
      //if(sigma_eff_sf_sig_sl_mu > 0){
      //	alphaFinding.setDesiredSigma(sigma_eff_sf_sig_sl_mu);
      //	alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*sigma_max/sigma_avg);
      //	alphaRoot.Solve();
      //}
      //RooRealVar eff_sf_sig_sl_mu_alpha("eff_sf_sig_sl_mu_alpha","eff_sf_sig_sl_mu_alpha", (sigma_eff_sf_sig_sl_mu > 0) ? alphaRoot.Root() : 0 );
      //eff_sf_sig_sl_mu_alpha.setConstant();
      //
      //
      //if(sigma_eff_sf_sb_sl_e > 0){
      //	alphaFinding.setDesiredSigma(sigma_eff_sf_sb_sl_e);
      //	alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*sigma_max/sigma_avg);
      //	alphaRoot.Solve();
      //}
      //RooRealVar eff_sf_sb_sl_e_alpha("eff_sf_sb_sl_e_alpha","eff_sf_sb_sl_e_alpha", (sigma_eff_sf_sb_sl_e > 0) ? alphaRoot.Root() : 0 );
      //eff_sf_sb_sl_e_alpha.setConstant();
      //
      //if(sigma_eff_sf_sb_sl_mu > 0){
      //	alphaFinding.setDesiredSigma(sigma_eff_sf_sb_sl_mu);
      //	alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*sigma_max/sigma_avg);
      //	alphaRoot.Solve();
      //}
      //RooRealVar eff_sf_sb_sl_mu_alpha("eff_sf_sb_sl_mu_alpha","eff_sf_sb_sl_mu_alpha", (sigma_eff_sf_sb_sl_mu > 0) ? alphaRoot.Root() : 0 );
      //eff_sf_sb_sl_mu_alpha.setConstant();
      //
      //
      //if(sigma_eff_sf_sig_ldp > 0){
      //	alphaFinding.setDesiredSigma(sigma_eff_sf_sig_ldp);
      //	alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*sigma_max/sigma_avg);
      //	alphaRoot.Solve();
      //}
      //RooRealVar eff_sf_sig_ldp_alpha("eff_sf_sig_ldp_alpha","eff_sf_sig_ldp_alpha", (sigma_eff_sf_sig_ldp > 0) ? alphaRoot.Root() : 0 );
      //eff_sf_sig_ldp_alpha.setConstant();
      //
      //if(sigma_eff_sf_sb_ldp > 0){
      //	alphaFinding.setDesiredSigma(sigma_eff_sf_sb_ldp);
      //	alphaRoot.SetFunction(WrappedAlphaFindingFunction, 1e-6, 5*sigma_max/sigma_avg);
      //	alphaRoot.Solve();
      //}
      //RooRealVar eff_sf_sb_ldp_alpha("eff_sf_sb_ldp_alpha","eff_sf_sb_ldp_alpha", (sigma_eff_sf_sb_ldp > 0) ? alphaRoot.Root() : 0 );
      //eff_sf_sb_ldp_alpha.setConstant();
      //
      //cout << "eff_sf section -- calculation of power law" << endl;
      //cout << "eff_sf section -- eff_sf_sig       has alpha of: " << eff_sf_sig_alpha       .getVal() << endl;
      //cout << "eff_sf section -- eff_sf_sb        has alpha of: " << eff_sf_sb_alpha        .getVal() << endl;
      //cout << "eff_sf section -- eff_sf_sig_sl_e  has alpha of: " << eff_sf_sig_sl_e_alpha  .getVal() << endl;
      //cout << "eff_sf section -- eff_sf_sig_sl_mu has alpha of: " << eff_sf_sig_sl_mu_alpha .getVal() << endl;
      //cout << "eff_sf section -- eff_sf_sb_sl_e   has alpha of: " << eff_sf_sb_sl_e_alpha   .getVal() << endl;
      //cout << "eff_sf section -- eff_sf_sb_sl_mu  has alpha of: " << eff_sf_sb_sl_mu_alpha  .getVal() << endl;
      //cout << "eff_sf section -- eff_sf_sig_ldp   has alpha of: " << eff_sf_sig_ldp_alpha   .getVal() << endl;
      //cout << "eff_sf section -- eff_sf_sb_ldp    has alpha of: " << eff_sf_sb_ldp_alpha    .getVal() << endl;
      //
      ////-- Parametric relations between correlated signal efficiency scale factors.
      //
      //RooFormulaVar fv_eff_sf_sig       ("eff_sf_sig"         , "pow(@0,@1)" , RooArgList( rrv_eff_sf , eff_sf_sig_alpha      ));
      //RooFormulaVar fv_eff_sf_sb        ("eff_sf_sb"          , "pow(@0,@1)" , RooArgList( rrv_eff_sf , eff_sf_sb_alpha       ));
      //RooFormulaVar fv_eff_sf_sig_sl_e  ("eff_sf_sig_sl_e"    , "pow(@0,@1)" , RooArgList( rrv_eff_sf , eff_sf_sig_sl_e_alpha ));
      //RooFormulaVar fv_eff_sf_sig_sl_mu ("eff_sf_sig_sl_mu"   , "pow(@0,@1)" , RooArgList( rrv_eff_sf , eff_sf_sig_sl_mu_alpha));
      //RooFormulaVar fv_eff_sf_sb_sl_e   ("eff_sf_sb_sl_e"     , "pow(@0,@1)" , RooArgList( rrv_eff_sf , eff_sf_sb_sl_e_alpha  ));
      //RooFormulaVar fv_eff_sf_sb_sl_mu  ("eff_sf_sb_sl_mu"    , "pow(@0,@1)" , RooArgList( rrv_eff_sf , eff_sf_sb_sl_mu_alpha ));
      //RooFormulaVar fv_eff_sf_sig_ldp   ("eff_sf_sig_ldp"     , "pow(@0,@1)" , RooArgList( rrv_eff_sf , eff_sf_sig_ldp_alpha  ));
      //RooFormulaVar fv_eff_sf_sb_ldp    ("eff_sf_sb_ldp"      , "pow(@0,@1)" , RooArgList( rrv_eff_sf , eff_sf_sb_ldp_alpha   ));

      sigma_avg = 1.;
      mu_avg = 0.;

      //// Truncated Gaussians for signal systematics:
      //
      //RooRealVar rrv_normal_eff_sf ("eff_sf","eff_sf",mu_avg,-1e5,1e5);
      //RooRealVar eff_sf_mu ("eff_sf_mu", "eff_sf_mu", mu_avg);
      //RooRealVar eff_sf_sigma ("eff_sf_sigma", "eff_sf_sigma", sigma_avg);
      //RooGaussian pdf_normal_eff_sf ("pdf_eff_sf" , "pdf_eff_sf", rrv_eff_sf, eff_sf_mu, eff_sf_sigma);
      //eff_sf_mu.setConstant();
      //eff_sf_sigma.setConstant();
      //normalNuisancePdfs.add (pdf_eff_sf);
      //
      //RooRealVar eff_sf_all_mu ("eff_sf_all_mu", "eff_sf_all_mu", 1.0);
      //eff_sf_mu.setConstant();
      //
      //RooRealVar eff_sf_sig_sigma     ("eff_sf_sig_sigma"      , "eff_sf_sig_sigma"      , sigma_eff_sf_sig      );
      //RooRealVar eff_sf_sb_sigma      ("eff_sf_sb_sigma"       , "eff_sf_sb_sigma"       , sigma_eff_sf_sb       );
      //RooRealVar eff_sf_sig_sl_e_sigma  ("eff_sf_sig_sl_e_sigma"   , "eff_sf_sig_sl_e_sigma"   , sigma_eff_sf_sig_sl_e   );
      //RooRealVar eff_sf_sig_sl_mu_sigma  ("eff_sf_sig_sl_mu_sigma"   , "eff_sf_sig_sl_mu_sigma"   , sigma_eff_sf_sig_sl_mu   );
      //RooRealVar eff_sf_sb_sl_e_sigma ("eff_sf_sb_sl_e_sigma"  , "eff_sf_sb_sl_e_sigma"  , sigma_eff_sf_sb_sl_e  );
      //RooRealVar eff_sf_sb_sl_mu_sigma("eff_sf_sb_sl_mu_sigma" , "eff_sf_sb_sl_mu_sigma" , sigma_eff_sf_sb_sl_mu );
      //RooRealVar eff_sf_sig_ldp_sigma ("eff_sf_sig_ldp_sigma"  , "eff_sf_sig_ldp_sigma"  , sigma_eff_sf_sig_ldp  );
      //RooRealVar eff_sf_sb_ldp_sigma  ("eff_sf_sb_ldp_sigma"   , "eff_sf_sb_ldp_sigma"   , sigma_eff_sf_sb_ldp   );

      //RooFormulaVar fv_normal_eff_sf_sig     ("eff_sf_sig"    , "(@1+@0*@2>=0)*(@1+@0*@2)" , RooArgList(rrv_normal_eff_sf, eff_sf_all_mu, eff_sf_sig_sigma    ));
      //RooFormulaVar fv_normal_eff_sf_sb      ("eff_sf_sb"     , "(@1+@0*@2>=0)*(@1+@0*@2)" , RooArgList(rrv_normal_eff_sf, eff_sf_all_mu, eff_sf_sb_sigma     ));
      //RooFormulaVar fv_normal_eff_sf_sig_sl  ("eff_sf_sig_sl" , "(@1+@0*@2>=0)*(@1+@0*@2)" , RooArgList(rrv_normal_eff_sf, eff_sf_all_mu, eff_sf_sig_sl_sigma ));
      //RooFormulaVar fv_normal_eff_sf_sb_sl   ("eff_sf_sb_sl"  , "(@1+@0*@2>=0)*(@1+@0*@2)" , RooArgList(rrv_normal_eff_sf, eff_sf_all_mu, eff_sf_sb_sl_sigma  ));
      //RooFormulaVar fv_normal_eff_sf_sig_ldp ("eff_sf_sig_ldp", "(@1+@0*@2>=0)*(@1+@0*@2)" , RooArgList(rrv_normal_eff_sf, eff_sf_all_mu, eff_sf_sig_ldp_sigma));
      //RooFormulaVar fv_normal_eff_sf_sb_ldp  ("eff_sf_sb_ldp" , "(@1+@0*@2>=0)*(@1+@0*@2)" , RooArgList(rrv_normal_eff_sf, eff_sf_all_mu, eff_sf_sb_ldp_sigma ));




     //rv_eff_sf_sig = new RooFormulaVar("eff_sf_sig",
     //                                   "mean_eff_sf_sig * pow( exp( width_eff_sf_sig/mean_eff_sf_sig ), eff_sf_prim )",
     //                                   RooArgSet( *rv_mean_eff_sf_sig, *rv_width_eff_sf_sig, *rv_mean_eff_sf_sig, eff_sf_prim ) ) ;
     //
     //rv_eff_sf_sb = new RooFormulaVar("eff_sf_sb",
     //                                   "mean_eff_sf_sb * pow( exp( width_eff_sf_sb/mean_eff_sf_sb ), eff_sf_prim )",
     //                                   RooArgSet( *rv_mean_eff_sf_sb, *rv_width_eff_sf_sb, *rv_mean_eff_sf_sb, eff_sf_prim ) ) ;
     //
     //rv_eff_sf_sig_sl = new RooFormulaVar("eff_sf_sig_sl",
     //                                   "mean_eff_sf_sig_sl * pow( exp( width_eff_sf_sig_sl/mean_eff_sf_sig_sl ), eff_sf_prim )",
     //                                   RooArgSet( *rv_mean_eff_sf_sig_sl, *rv_width_eff_sf_sig_sl, *rv_mean_eff_sf_sig_sl, eff_sf_prim ) ) ;
     //
     //rv_eff_sf_sb_sl = new RooFormulaVar("eff_sf_sb_sl",
     //                                   "mean_eff_sf_sb_sl * pow( exp( width_eff_sf_sb_sl/mean_eff_sf_sb_sl ), eff_sf_prim )",
     //                                   RooArgSet( *rv_mean_eff_sf_sb_sl, *rv_width_eff_sf_sb_sl, *rv_mean_eff_sf_sb_sl, eff_sf_prim ) ) ;
     //
     //rv_eff_sf_sig_ldp = new RooFormulaVar("eff_sf_sig_ldp",
     //                                   "mean_eff_sf_sig_ldp * pow( exp( width_eff_sf_sig_ldp/mean_eff_sf_sig_ldp ), eff_sf_prim )",
     //                                   RooArgSet( *rv_mean_eff_sf_sig_ldp, *rv_width_eff_sf_sig_ldp, *rv_mean_eff_sf_sig_ldp, eff_sf_prim ) ) ;
     //
     //rv_eff_sf_sb_ldp = new RooFormulaVar("eff_sf_sb_ldp",
     //                                   "mean_eff_sf_sb_ldp * pow( exp( width_eff_sf_sb_ldp/mean_eff_sf_sb_ldp ), eff_sf_prim )",
     //                                   RooArgSet( *rv_mean_eff_sf_sb_ldp, *rv_width_eff_sf_sb_ldp, *rv_mean_eff_sf_sb_ldp, eff_sf_prim ) ) ;



/*       RooRealVar xxx_prim ("xxx_prim", "xxx_prim", 0, -5, 5); */
/*       RooRealVar xxx_nom ("xxx_nom", "xxx_nom", 0, -5, 5); */
/*       RooGaussian pdf_xxx ("pdf_xxx" , "pdf_xxx", xxx_prim, xxx_nom, RooConst(1)); */
/*       sprintf (formula, "%f*pow(%f,@0)", xxx, exp(xxx_err/xxx)); */
/*       RooFormulaVar fv_xxx ("xxx", formula, RooArgList(xxx_prim)); */
/*       globalObservables.add (xxx_nom); */
/*       allNuisances.add (xxx_prim); */
/*       allNuisancePdfs.add (pdf_xxx); */



    //-- Z to nunu stuff



      rv_znnoverll_bfratio = new RooRealVar("znnoverll_bfratio", "znnoverll_bfratio", 0.1, 10. ) ;
      rv_znnoverll_bfratio -> setVal( 5.95 ) ;
      rv_znnoverll_bfratio -> setConstant( kTRUE ) ;

      rv_dataoverll_lumiratio = new RooRealVar("dataoverll_lumiratio", "dataoverll_lumiratio", 0.1, 10.0 ) ;
      rv_dataoverll_lumiratio  -> setVal( DataLumi / Ztoll_lumi ) ;
      rv_dataoverll_lumiratio  -> setConstant( kTRUE ) ;




     //+++++++++++++++++ Relationships between parameters ++++++++++++++++++++++++++++++++++++++++++

       printf(" --- Defining relationships between parameters.\n" ) ;




    //-- ttwj

      if ( useSigTtwjVar ) {
         rfv_mu_ttwj_sb = new RooFormulaVar("mu_ttwj_sb",
					     "mu_ttwj_sig * (1.0/sf_ttwj_sig) * ((mu_ttwj_sb_sl_e+mu_ttwj_sb_sl_mu)/mu_ttwj_sig_sl)",
					     RooArgSet( *rv_mu_ttwj_sig, rrv_sf_ttwj_sig, *rv_mu_ttwj_sb_sl_e,*rv_mu_ttwj_sb_sl_mu, *rv_mu_ttwj_sig_sl) ) ;
         rv_mu_ttwj_sb = rfv_mu_ttwj_sb ;
      } else {
         rfv_mu_ttwj_sig = new RooFormulaVar("mu_ttwj_sig",
					      "mu_ttwj_sb * sf_ttwj_sig * (mu_ttwj_sig_sl/(mu_ttwj_sb_sl_e+mu_ttwj_sb_sl_mu))",
					      RooArgSet( *rv_mu_ttwj_sb, rrv_sf_ttwj_sig, *rv_mu_ttwj_sig_sl,*rv_mu_ttwj_sb_sl_e,*rv_mu_ttwj_sb_sl_mu) ) ;
         rv_mu_ttwj_sig = rfv_mu_ttwj_sig ;
      }




      rv_mu_ttwj_sig_ldp = new RooFormulaVar("mu_ttwj_sig_ldp",
                              "mu_ttbarmc_sig_ldp + mu_WJmc_sig_ldp",
                              RooArgSet( *rv_mu_ttbarmc_sig_ldp,
                                         *rv_mu_WJmc_sig_ldp ) ) ;


      rv_mu_ttwj_sb_ldp = new RooFormulaVar("mu_ttwj_sb_ldp",
                              "mu_ttbarmc_sb_ldp + mu_WJmc_sb_ldp",
                              RooArgSet( *rv_mu_ttbarmc_sb_ldp,
                                         *rv_mu_WJmc_sb_ldp ) ) ;






    //-- QCD

   //-------------------------------
   ///if ( useLdpVars ) {

   ///   rfv_mu_qcd_sig = new RooFormulaVar("mu_qcd_sig",
   ///                               "mu_qcd_sig_ldp * sf_qcd_sig * ( mu_qcd_lsb_0b / mu_qcd_lsb_0b_ldp )",
   ///                               RooArgSet( *rv_mu_qcd_sig_ldp, fv_sf_qcd_sig, *rv_mu_qcd_lsb_0b, *rv_mu_qcd_lsb_0b_ldp ) ) ;
   ///   rv_mu_qcd_sig = rfv_mu_qcd_sig ;

   ///   rfv_mu_qcd_sb = new RooFormulaVar("mu_qcd_sb",
   ///                               "mu_qcd_sb_ldp * sf_qcd_sb * ( mu_qcd_lsb_0b / mu_qcd_lsb_0b_ldp )",
   ///                               RooArgSet( *rv_mu_qcd_sb_ldp, fv_sf_qcd_sb, *rv_mu_qcd_lsb_0b, *rv_mu_qcd_lsb_0b_ldp ) ) ;
   ///   rv_mu_qcd_sb = rfv_mu_qcd_sb ;

   ///} else {

   ///   rfv_mu_qcd_sig_ldp = new RooFormulaVar("mu_qcd_sig_ldp",
   ///                               "mu_qcd_sig * (1.0/sf_qcd_sig) * ( mu_qcd_lsb_0b_ldp / mu_qcd_lsb_0b )",
   ///                               RooArgSet( *rv_mu_qcd_sig, fv_sf_qcd_sig, *rv_mu_qcd_lsb_0b_ldp, *rv_mu_qcd_lsb_0b ) ) ;
   ///   rv_mu_qcd_sig_ldp = rfv_mu_qcd_sig_ldp ;

   ///   rfv_mu_qcd_sb_ldp = new RooFormulaVar("mu_qcd_sb_ldp",
   ///                               "mu_qcd_sb * (1.0/sf_qcd_sb) * ( mu_qcd_lsb_0b_ldp / mu_qcd_lsb_0b )",
   ///                               RooArgSet( *rv_mu_qcd_sb, fv_sf_qcd_sb, *rv_mu_qcd_lsb_0b_ldp, *rv_mu_qcd_lsb_0b ) ) ;
   ///   rv_mu_qcd_sb_ldp = rfv_mu_qcd_sb_ldp ;

   ///}
   //-------------------------------

      if ( useLdpVars ) {

         rfv_mu_qcd_sig = new RooFormulaVar("mu_qcd_sig",
					     "mu_qcd_sig_ldp * sf_qcd_sig * Rlsb_passfail",
					     RooArgSet( *rv_mu_qcd_sig_ldp, rrv_sf_qcd_sig, rrv_Rlsb_passfail) ) ;
         rv_mu_qcd_sig = rfv_mu_qcd_sig ;

         rfv_mu_qcd_sb = new RooFormulaVar("mu_qcd_sb",
					    "mu_qcd_sb_ldp * sf_qcd_sb * Rlsb_passfail",
					    RooArgSet( *rv_mu_qcd_sb_ldp, rrv_sf_qcd_sb, rrv_Rlsb_passfail ) ) ;
         rv_mu_qcd_sb = rfv_mu_qcd_sb ;

      } else {

         rfv_mu_qcd_sig_ldp = new RooFormulaVar("mu_qcd_sig_ldp",
						 "mu_qcd_sig * (1.0/sf_qcd_sig) * ( 1.0 / Rlsb_passfail )",
						 RooArgSet( *rv_mu_qcd_sig, rrv_sf_qcd_sig, rrv_Rlsb_passfail ) ) ;
         rv_mu_qcd_sig_ldp = rfv_mu_qcd_sig_ldp ;

         rfv_mu_qcd_sb_ldp = new RooFormulaVar("mu_qcd_sb_ldp",
						"mu_qcd_sb * (1.0/sf_qcd_sb) * ( 1.0 / Rlsb_passfail ) ",
						RooArgSet( *rv_mu_qcd_sb, rrv_sf_qcd_sb, rrv_Rlsb_passfail ) ) ;
         rv_mu_qcd_sb_ldp = rfv_mu_qcd_sb_ldp ;

      }

   //-------------------------------



    //-- SUSY

      rv_mu_susy_sb = new RooFormulaVar("mu_susy_sb",
                                        "mu_susymc_sb * (mu_susy_sig/mu_susymc_sig)",
					 RooArgSet( *rv_mu_susymc_sb, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sig_sl_e = new RooFormulaVar("mu_susy_sig_sl_e",
					      "mu_susymc_sig_sl_e * (mu_susy_sig/mu_susymc_sig)",
					      RooArgSet( *rv_mu_susymc_sig_sl_e, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sig_sl_mu = new RooFormulaVar("mu_susy_sig_sl_mu",
					      "mu_susymc_sig_sl_mu * (mu_susy_sig/mu_susymc_sig)",
					      RooArgSet( *rv_mu_susymc_sig_sl_mu, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sb_sl_e = new RooFormulaVar("mu_susy_sb_sl_e",
					    "mu_susymc_sb_sl_e * (mu_susy_sig/mu_susymc_sig)",
					    RooArgSet( *rv_mu_susymc_sb_sl_e, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sb_sl_mu = new RooFormulaVar("mu_susy_sb_sl_mu",
					    "mu_susymc_sb_sl_mu * (mu_susy_sig/mu_susymc_sig)",
					    RooArgSet( *rv_mu_susymc_sb_sl_mu, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sig_ldp = new RooFormulaVar("mu_susy_sig_ldp",
					      "mu_susymc_sig_ldp * (mu_susy_sig/mu_susymc_sig)",
					      RooArgSet( *rv_mu_susymc_sig_ldp, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;

      rv_mu_susy_sb_ldp = new RooFormulaVar("mu_susy_sb_ldp",
					     "mu_susymc_sb_ldp * (mu_susy_sig/mu_susymc_sig)",
					     RooArgSet( *rv_mu_susymc_sb_ldp, *rv_mu_susy_sig, *rv_mu_susymc_sig ) ) ;





    //-- Z to nu nu

      rv_mu_znn_sig_ldp = new RooFormulaVar("mu_znn_sig_ldp",
                              "mu_Znnmc_sig_ldp",
                              RooArgSet( *rv_mu_Znnmc_sig_ldp ) ) ;

      rv_mu_znn_sb_ldp = new RooFormulaVar("mu_znn_sb_ldp",
                              "mu_Znnmc_sb_ldp",
                              RooArgSet( *rv_mu_Znnmc_sb_ldp ) ) ;


   //---------------------------------
   // if ( znnModel == 1 ) {

   //    rv_mu_zee_sb_ee = new RooFormulaVar("mu_zee_sb_ee",
   //                                    "mu_znn_sb * sf_ee * ( acc_ee * eff_ee ) / ( znnoverll_bfratio * dataoverll_lumiratio )",
   //                                    RooArgSet( *rv_mu_znn_sb, fv_sf_ee, fv_acc_ee, fv_eff_ee, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

   //    rv_mu_zee_sig_ee = new RooFormulaVar("mu_zee_sig_ee",
   //                                    "mu_znn_sig * sf_ee * ( acc_ee * eff_ee ) / ( znnoverll_bfratio * dataoverll_lumiratio )",
   //                                    RooArgSet( *rv_mu_znn_sig, fv_sf_ee, fv_acc_ee, fv_eff_ee, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

   //    rv_mu_zmm_sb_mm = new RooFormulaVar("mu_zmm_sb_mm",
   //                                    "mu_znn_sb * sf_mm * ( acc_mm * eff_mm ) / ( znnoverll_bfratio * dataoverll_lumiratio )",
   //                                    RooArgSet( *rv_mu_znn_sb, fv_sf_mm, fv_acc_mm, fv_eff_mm, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

   //    rv_mu_zmm_sig_mm = new RooFormulaVar("mu_zmm_sig_mm",
   //                                    "mu_znn_sig * sf_mm * ( acc_mm * eff_mm ) / ( znnoverll_bfratio * dataoverll_lumiratio )",
   //                                    RooArgSet( *rv_mu_znn_sig, fv_sf_mm, fv_acc_mm, fv_eff_mm, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

   // } else if ( znnModel == 2 ) {

   //    rfv_mu_znn_sb = new RooFormulaVar("mu_znn_sb",
   //                                     "mu_znn_sig * ( knn_sb / knn_sig )",
   //                                     RooArgSet( *rv_mu_znn_sig, fv_knn_sb, fv_knn_sig ) ) ;

   //    rv_mu_znn_sb = rfv_mu_znn_sb ;

   //    rv_mu_zee_sigsb_ee = new RooFormulaVar("mu_zee_sigsb_ee",
   //                                  "( mu_znn_sig / knn_sig ) * sf_ee * ( (acc_ee * eff_ee ) / ( znnoverll_bfratio * dataoverll_lumiratio ) )",
   //                                    RooArgSet( *rv_mu_znn_sig, fv_knn_sig, fv_sf_ee, fv_acc_ee, fv_eff_ee, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

   //    rv_mu_zmm_sigsb_mm = new RooFormulaVar("mu_zmm_sigsb_mm",
   //                                  "( mu_znn_sig / knn_sig ) * sf_mm * ( (acc_mm * eff_mm ) / ( znnoverll_bfratio * dataoverll_lumiratio ) )",
   //                                    RooArgSet( *rv_mu_znn_sig, fv_knn_sig, fv_sf_mm, fv_acc_mm, fv_eff_mm, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

   // }
   //---------------------------------



      rv_mu_zee_sig_ee = new RooFormulaVar("mu_zee_sig_ee",
                                    "( mu_znn_sig / knn_ee_sig ) * sf_ee * ( (acc_ee_sig * eff_ee ) / ( znnoverll_bfratio * dataoverll_lumiratio ) )",
                                      RooArgSet( *rv_mu_znn_sig, rrv_knn_ee_sig, fv_sf_ee, rrv_acc_ee_sig, rrv_eff_ee, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;


      rv_mu_zee_sb_ee = new RooFormulaVar("mu_zee_sb_ee",
                                    "( mu_znn_sb / knn_ee_sb ) * sf_ee * ( (acc_ee_sb * eff_ee ) / ( znnoverll_bfratio * dataoverll_lumiratio ) )",
                                      RooArgSet( *rv_mu_znn_sb, rrv_knn_ee_sb, fv_sf_ee, rrv_acc_ee_sb, rrv_eff_ee, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

      rv_mu_zmm_sig_mm = new RooFormulaVar("mu_zmm_sig_mm",
                                    "( mu_znn_sig / knn_mm_sig ) * sf_mm * ( (acc_mm_sig * eff_mm ) / ( znnoverll_bfratio * dataoverll_lumiratio ) )",
                                      RooArgSet( *rv_mu_znn_sig, rrv_knn_mm_sig, fv_sf_mm, rrv_acc_mm_sig, rrv_eff_mm, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;

      rv_mu_zmm_sb_mm = new RooFormulaVar("mu_zmm_sb_mm",
                                    "( mu_znn_sb / knn_mm_sb ) * sf_mm * ( (acc_mm_sb * eff_mm ) / ( znnoverll_bfratio * dataoverll_lumiratio ) )",
                                      RooArgSet( *rv_mu_znn_sb, rrv_knn_mm_sb, fv_sf_mm, rrv_acc_mm_sb, rrv_eff_mm, *rv_znnoverll_bfratio, *rv_dataoverll_lumiratio ) ) ;










    //-- EWO

      rv_mu_ewo_sig     = new RooFormulaVar("mu_ewo_sig"     , "mu_Ewomc_sig"     , RooArgSet( *rv_mu_Ewomc_sig     ) ) ;
      rv_mu_ewo_sb      = new RooFormulaVar("mu_ewo_sb"      , "mu_Ewomc_sb"      , RooArgSet( *rv_mu_Ewomc_sb      ) ) ;
      rv_mu_ewo_sig_ldp = new RooFormulaVar("mu_ewo_sig_ldp" , "mu_Ewomc_sig_ldp" , RooArgSet( *rv_mu_Ewomc_sig_ldp ) ) ;
      rv_mu_ewo_sb_ldp  = new RooFormulaVar("mu_ewo_sb_ldp"  , "mu_Ewomc_sb_ldp"  , RooArgSet( *rv_mu_Ewomc_sb_ldp  ) ) ;





    //+++++++++++++ Expected counts for observables in terms of parameters ++++++++++++++++++

       printf(" --- Defining expected counts in terms of parameters.\n" ) ;

      rv_n_sig         = new RooFormulaVar("n_sig",
					   "epsSF_sig*(mu_ttwj_sig + mu_qcd_sig + mu_znn_sig + eff_sf_sig*( mu_ewo_sig + mu_susy_sig))",
					   RooArgSet(rrv_epsSF_sig,*rv_mu_ttwj_sig, *rv_mu_qcd_sig, *rv_mu_znn_sig, fv_eff_sf_sig, *rv_mu_ewo_sig, *rv_mu_susy_sig ) ) ;
      
      rv_n_sb          = new RooFormulaVar("n_sb",
					   "epsSF_sb*(mu_ttwj_sb  + mu_qcd_sb  + mu_znn_sb  + eff_sf_sb*( mu_ewo_sb  + mu_susy_sb ))",
					   RooArgSet(rrv_epsSF_sb, *rv_mu_ttwj_sb , *rv_mu_qcd_sb , *rv_mu_znn_sb , fv_eff_sf_sb, *rv_mu_ewo_sb , *rv_mu_susy_sb  ) ) ;
      
      rv_n_sig_ldp     = new RooFormulaVar("n_sig_ldp",
					   "epsSF_sig*(mu_qcd_sig_ldp + sf_mc * (mu_ttwj_sig_ldp + mu_znn_sig_ldp + mu_ewo_sig_ldp) + eff_sf_sig_ldp*(mu_susy_sig_ldp))",
					   RooArgSet(rrv_epsSF_sig, *rv_mu_qcd_sig_ldp, fv_eff_sf_sig_ldp, rrv_sf_mc, *rv_mu_ttwj_sig_ldp, *rv_mu_znn_sig_ldp, *rv_mu_ewo_sig_ldp, *rv_mu_susy_sig_ldp ) ) ;

      rv_n_sb_ldp      = new RooFormulaVar("n_sb_ldp",
					   "epsSF_sb_ldp*(mu_qcd_sb_ldp + sf_mc * (mu_ttwj_sb_ldp + mu_znn_sb_ldp + mu_ewo_sb_ldp) + eff_sf_sb_ldp*(mu_susy_sb_ldp))",
					   RooArgSet(rrv_epsSF_sb_ldp, *rv_mu_qcd_sb_ldp, fv_eff_sf_sb_ldp, rrv_sf_mc, *rv_mu_ttwj_sb_ldp, *rv_mu_znn_sb_ldp, *rv_mu_ewo_sb_ldp, *rv_mu_susy_sb_ldp ) ) ;
      
      rv_n_sig_sl      = new RooFormulaVar("n_sig_sl",
					   "epsSF_sig_sl*(mu_ttwj_sig_sl + eff_sf_sig_sl_e*mu_susy_sig_sl_e + eff_sf_sig_sl_mu*mu_susy_sig_sl_mu)",
					   RooArgSet(rrv_epsSF_sig_sl, *rv_mu_ttwj_sig_sl, fv_eff_sf_sig_sl_e, *rv_mu_susy_sig_sl_e, fv_eff_sf_sig_sl_mu, *rv_mu_susy_sig_sl_mu ) ) ;

      rv_n_sb_sl_e       = new RooFormulaVar("n_sb_sl_e",
					     "epsSF_sb_sl_e*(mu_ttwj_sb_sl_e + eff_sf_sb_sl_e*mu_susy_sb_sl_e)",
					     RooArgSet(rrv_epsSF_sb_sl_e, *rv_mu_ttwj_sb_sl_e, fv_eff_sf_sb_sl_e, *rv_mu_susy_sb_sl_e ) ) ;

      rv_n_sb_sl_mu       = new RooFormulaVar("n_sb_sl_mu",
					      "epsSF_sb_sl_mu*(mu_ttwj_sb_sl_mu + eff_sf_sb_sl_mu*mu_susy_sb_sl_mu)",
					      RooArgSet(rrv_epsSF_sb_sl_mu, *rv_mu_ttwj_sb_sl_mu, fv_eff_sf_sb_sl_mu, *rv_mu_susy_sb_sl_mu ) ) ;

 ///  rv_n_lsb_0b      = new RooFormulaVar("n_lsb_0b",
 ///                                 "mu_qcd_lsb_0b",
 ///                                 RooArgSet( *rv_mu_qcd_lsb_0b ) ) ;

 ///  rv_n_lsb_0b_ldp  = new RooFormulaVar("n_lsb_0b_ldp",
 ///                                 "mu_qcd_lsb_0b_ldp",
 ///                                 RooArgSet( *rv_mu_qcd_lsb_0b_ldp ) ) ;


      rv_n_sig_ee      = new RooFormulaVar("n_sig_ee",
                                     "mu_zee_sig_ee / fsig_ee",
                                     RooArgSet( *rv_mu_zee_sig_ee, rrv_fsig_ee ) ) ;

      rv_n_sb_ee       = new RooFormulaVar("n_sb_ee",
                                     "mu_zee_sb_ee / fsig_ee",
                                     RooArgSet( *rv_mu_zee_sb_ee, rrv_fsig_ee ) ) ;

      rv_n_sig_mm      = new RooFormulaVar("n_sig_mm",
                                     "mu_zmm_sig_mm / fsig_mm",
                                     RooArgSet( *rv_mu_zmm_sig_mm, rrv_fsig_mm ) ) ;

      rv_n_sb_mm       = new RooFormulaVar("n_sb_mm",
                                     "mu_zmm_sb_mm / fsig_mm",
                                     RooArgSet( *rv_mu_zmm_sb_mm, rrv_fsig_mm ) ) ;


   //++++++++++++ PDFs for the likelihood +++++++++++++++++++++++++++++++++++++++++++++

      printf(" --- Defining PDFs of the likelihood.\n" ) ;

      pdf_Nsig        = new RooPoisson("pdf_Nsig"        , "Nsig Poisson PDF"        , *rv_Nsig        , *rv_n_sig ) ;
      pdf_Nsb         = new RooPoisson("pdf_Nsb"         , "Nsb Poisson PDF"         , *rv_Nsb         , *rv_n_sb ) ;
      pdf_Nsig_ldp    = new RooPoisson("pdf_Nsig_ldp"    , "Nsig_ldp Poisson PDF"    , *rv_Nsig_ldp    , *rv_n_sig_ldp ) ;
      pdf_Nsb_ldp     = new RooPoisson("pdf_Nsb_ldp"     , "Nsb_ldp Poisson PDF"     , *rv_Nsb_ldp     , *rv_n_sb_ldp ) ;
      pdf_Nsig_sl     = new RooPoisson("pdf_Nsig_sl"     , "Nsig_sl Poisson PDF"     , *rv_Nsig_sl     , *rv_n_sig_sl ) ;
      pdf_Nsb_sl_e      = new RooPoisson("pdf_Nsb_sl_e"      , "Nsb_sl_e Poisson PDF"      , *rv_Nsb_sl_e      , *rv_n_sb_sl_e ) ;
      pdf_Nsb_sl_mu      = new RooPoisson("pdf_Nsb_sl_mu"      , "Nsb_sl_mu Poisson PDF"      , *rv_Nsb_sl_mu      , *rv_n_sb_sl_mu ) ;
 ///  pdf_Nlsb_0b     = new RooPoisson("pdf_Nlsb_0b"     , "Nlsb_0b Poisson PDF"     , *rv_Nlsb_0b     , *rv_n_lsb_0b ) ;
 ///  pdf_Nlsb_0b_ldp = new RooPoisson("pdf_Nlsb_0b_ldp" , "Nlsb_0b_ldp Poisson PDF" , *rv_Nlsb_0b_ldp , *rv_n_lsb_0b_ldp ) ;

      pdf_Nsig_ee     = new RooPoisson("pdf_Nsig_ee"     , "Nsig_ee Poisson PDF"     , *rv_Nsig_ee     , *rv_n_sig_ee ) ;
      pdf_Nsb_ee      = new RooPoisson("pdf_Nsb_ee"      , "Nsb_ee Poisson PDF"      , *rv_Nsb_ee      , *rv_n_sb_ee ) ;
      pdf_Nsig_mm     = new RooPoisson("pdf_Nsig_mm"     , "Nsig_mm Poisson PDF"     , *rv_Nsig_mm     , *rv_n_sig_mm ) ;
      pdf_Nsb_mm      = new RooPoisson("pdf_Nsb_mm"      , "Nsb_mm Poisson PDF"      , *rv_Nsb_mm      , *rv_n_sb_mm ) ;


      {
         RooArgSet pdflist ;
         pdflist.add( *pdf_Nsig        ) ;
         pdflist.add( *pdf_Nsb         ) ;
         pdflist.add( *pdf_Nsig_ldp    ) ;
         pdflist.add( *pdf_Nsb_ldp     ) ;
         pdflist.add( *pdf_Nsig_sl     ) ;
         pdflist.add( *pdf_Nsb_sl_e      ) ;
         pdflist.add( *pdf_Nsb_sl_mu      ) ;
   ////  pdflist.add( *pdf_Nlsb_0b     ) ;
   ////  pdflist.add( *pdf_Nlsb_0b_ldp ) ;
         pdflist.add( *pdf_Nsig_ee     ) ;
         pdflist.add( *pdf_Nsb_ee      ) ;
         pdflist.add( *pdf_Nsig_mm     ) ;
         pdflist.add( *pdf_Nsb_mm      ) ;

	 //pdflist.add(gammaNuisancePdfs);
	 //pdflist.add(betaNuisancePdfs);
	 pdflist.add(allNuisancePdfs);

         likelihood = new RooProdPdf("likelihood", "ra2b likelihood", pdflist ) ;
	 likelihood->Print("v");

         RooArgSet pdflist_noNSig ;
         pdflist_noNSig.add( *pdf_Nsb         ) ;
         pdflist_noNSig.add( *pdf_Nsig_ldp    ) ;
         pdflist_noNSig.add( *pdf_Nsb_ldp     ) ;
         pdflist_noNSig.add( *pdf_Nsig_sl     ) ;
         pdflist_noNSig.add( *pdf_Nsb_sl_e      ) ;
         pdflist_noNSig.add( *pdf_Nsb_sl_mu      ) ;
   ////  pdflist_noNSig.add( *pdf_Nlsb_0b     ) ;
   ////  pdflist_noNSig.add( *pdf_Nlsb_0b_ldp ) ;
         pdflist_noNSig.add( *pdf_Nsig_ee     ) ;
         pdflist_noNSig.add( *pdf_Nsb_ee      ) ;
         pdflist_noNSig.add( *pdf_Nsig_mm     ) ;
         pdflist_noNSig.add( *pdf_Nsb_mm      ) ;

	 //pdflist_noNSig.add(gammaNuisancePdfs);
	 //pdflist_noNSig.add(betaNuisancePdfs);

         likelihood_noNSig = new RooProdPdf("likelihood_noNSig", "ra2b likelihood w/out NSig", pdflist_noNSig ) ;

      }


     //---- Define the list of observables.

       observedParametersList.add( *rv_Nsig        ) ;
       observedParametersList.add( *rv_Nsb         ) ;
       observedParametersList.add( *rv_Nsig_sl     ) ;
       observedParametersList.add( *rv_Nsb_sl_e      ) ;
       observedParametersList.add( *rv_Nsb_sl_mu      ) ;
       observedParametersList.add( *rv_Nsig_ldp    ) ;
       observedParametersList.add( *rv_Nsb_ldp     ) ;
  //// observedParametersList.add( *rv_Nlsb_0b     ) ;
  //// observedParametersList.add( *rv_Nlsb_0b_ldp ) ;
       observedParametersList.add( *rv_Nsb_ee      ) ;
       observedParametersList.add( *rv_Nsig_ee     ) ;
       observedParametersList.add( *rv_Nsb_mm      ) ;
       observedParametersList.add( *rv_Nsig_mm     ) ;



       dsObserved = new RooDataSet("ra2b_observed_rds", "RA2b observed data values",
                                      observedParametersList ) ;
       dsObserved->add( observedParametersList ) ;

       dsObserved->Print("v");


       RooWorkspace workspace ("ws") ;
       workspace.autoImportClassCode(true);

       workspace.import(*dsObserved);

       // parameters of interest
       RooArgSet poi(*rv_mu_susy_sig, "poi");
       // flat prior for POI
       RooUniform signal_prior ("signal_prior","signal_prior",*rv_mu_susy_sig);

       allNuisances.add(allPoissonNuisances);
       allNuisances.add(allNonPoissonNuisances);

       // signal+background model
       ModelConfig sbModel ("SbModel");
       sbModel.SetWorkspace(workspace);
       sbModel.SetPdf(*likelihood);
       sbModel.SetParametersOfInterest(poi);
       sbModel.SetPriorPdf(signal_prior);
       sbModel.SetNuisanceParameters(allNuisances);
       sbModel.SetObservables(observedParametersList);
       sbModel.SetGlobalObservables(globalObservables);

 
       // find global maximum with the signal+background model
       // with conditional MLEs for nuisance parameters
       // and save the parameter point snapshot in the Workspace
       //  - safer to keep a default name because some RooStats calculators
       //    will anticipate it
       RooAbsReal * pNll = sbModel.GetPdf()->createNLL(*dsObserved);
       RooAbsReal * pProfile = pNll->createProfile(RooArgSet());
       pProfile->getVal(); // this will do fit and set POI and nuisance parameters to fitted values
       RooArgSet * pPoiAndNuisance = new RooArgSet();
       pPoiAndNuisance->add(*sbModel.GetParametersOfInterest());
       if(sbModel.GetNuisanceParameters()) pPoiAndNuisance->add(*sbModel.GetNuisanceParameters());
       cout << "\nWill save these parameter points that correspond to the fit to data" << endl;
       pPoiAndNuisance->Print("v");
       sbModel.SetSnapshot(*pPoiAndNuisance);
       workspace.import (sbModel);

       delete pProfile;
       delete pNll;
       delete pPoiAndNuisance;


       // background-only model
       // use the same PDF as s+b, with xsec=0
       // POI value under the background hypothesis
       ModelConfig bModel (*(RooStats::ModelConfig *)workspace.obj("SbModel"));
       bModel.SetName("BModel");
       bModel.SetWorkspace(workspace);

       // Find a parameter point for generating pseudo-data
       // with the background-only data.
       // Save the parameter point snapshot in the Workspace
       pNll = bModel.GetPdf()->createNLL(*dsObserved);
       // bug discovered by Fedor on Sep 6th 2011:
       //pProfile = pNll->createProfile(poi);
       //((RooRealVar *)poi.first())->setVal(0.); // set signal = 0
       pProfile = pNll->createProfile(*bModel.GetParametersOfInterest());
       ((RooRealVar *)(bModel.GetParametersOfInterest()->first()))->setVal(0.); // set signal = 0
       pProfile->getVal(); // this will do fit and set nuisance parameters to profiled values
       pPoiAndNuisance = new RooArgSet();
       pPoiAndNuisance->add(*bModel.GetParametersOfInterest());
       if(bModel.GetNuisanceParameters()) pPoiAndNuisance->add(*bModel.GetNuisanceParameters());
       cout << "\nShould use these parameter points to generate pseudo data for bkg only" << endl;
       pPoiAndNuisance->Print("v");
       bModel.SetSnapshot(*pPoiAndNuisance);
       workspace.import (bModel);

       workspace.import(*likelihood_noNSig, RecycleConflictNodes());

       RooArgSet nuiscancePriors;
       nuiscancePriors.add(gammaNuisancePdfs);
       nuiscancePriors.add(betaNuisancePdfs);
       nuiscancePriors.add(allNuisancePdfs);

       RooProdPdf nuisancePrior("nuisancePrior", "nuisances we have priors for", nuiscancePriors ) ;
       workspace.import(nuisancePrior, RecycleConflictNodes());

       workspace.defineSet("allNonPoissonNuisances",allNonPoissonNuisances);

       delete pProfile;
       delete pNll;
       delete pPoiAndNuisance;


       workspace.Print() ;
       workspace.writeToFile(outfile);

       return true ;


    } // initialize.


   //===================================================================================================================================


    bool ra2bRoostatsClass7::setSusyScanPoint( const char* inputScanFile, double m0, double m12, bool isT1bbbb, double t1bbbbXsec ) {


       //--- Aug 15, 2011: updated to new format for AN, v3.


       printf("\n\n Opening SUSY scan input file : %s\n", inputScanFile ) ;

       FILE* infp ;
       if ( (infp=fopen( inputScanFile,"r"))==NULL ) {
          printf("\n\n *** Problem opening input file: %s.\n\n", inputScanFile ) ;
          return false ;
       }

       double deltaM0(0.) ;
       double deltaM12(0.) ;

       if ( !isT1bbbb ) {
          deltaM0 = 20 ;
          deltaM12 = 20 ;
       } else {
          deltaM0 = 25 ;
          deltaM12 = 25 ;
       }

       bool found(false) ;

       //--- Loop over the scan points.
       while ( !feof( infp ) ) {

          float pointM0 ;
          float pointM12 ;



   //+++ ORL: Aug 14, 2011 ++++++++++++++++++++++++++
          float n_sig_raw ;
          float n_sb_raw ;
          float n_sig_sl_e_raw ;
          float n_sig_sl_mu_raw ;
          float n_sb_sl_e_raw ;
          float n_sb_sl_mu_raw ;
          float n_sig_ldp_raw ;
          float n_sb_ldp_raw ;

          float n_sig_correction ;
          float n_sb_correction ;
          float n_sig_sl_e_correction ;
	  float n_sig_sl_mu_correction ;
          float n_sb_sl_e_correction ;
          float n_sb_sl_mu_correction ;
          float n_sig_ldp_correction ;
          float n_sb_ldp_correction ;

          float n_sig_error ;
          float n_sb_error ;
          float n_sig_sl_e_error ;
          float n_sig_sl_mu_error ;
          float n_sb_sl_e_error ;
          float n_sb_sl_mu_error ;
          float n_sig_ldp_error ;
          float n_sb_ldp_error ;

          int nGen ;

          fscanf( infp, "%f %f %d  %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
            &pointM0, &pointM12, &nGen,
            &n_sig_raw, &n_sb_raw, &n_sig_sl_mu_raw, &n_sb_sl_mu_raw, &n_sig_sl_e_raw, &n_sb_sl_e_raw, &n_sig_ldp_raw, &n_sb_ldp_raw,
            &n_sig_correction, &n_sb_correction, &n_sig_sl_mu_correction, &n_sb_sl_mu_correction, &n_sig_sl_e_correction, &n_sb_sl_e_correction, &n_sig_ldp_correction, &n_sb_ldp_correction,
            &n_sig_error, &n_sb_error,&n_sig_sl_mu_error, &n_sb_sl_mu_error,  &n_sig_sl_e_error, &n_sb_sl_e_error,&n_sig_ldp_error, &n_sb_ldp_error ) ;

          if ( feof(infp) ) break ;
          //if ( n_sig_raw < 0.00001 ) continue ;
   //--If you are asking for it, I'll assume it's good.  Josh is using 0 for ngen dummy in LM9.
   /////  if ( nGen != 10000 ) continue ; // get rid of bad scan points.

          if (    fabs( pointM0 - m0 ) <= deltaM0/2.
               && fabs( pointM12 - m12 ) <= deltaM12/2. ) {

              double nGenPerPoint = 10000 ; // for t1bbbb

             printf("\n\n") ;
             printf("  SUSY efficiency  systematic uncertainty,   n_sig_error     = %6.1f %%\n", n_sig_error     ) ;
             printf("  SUSY efficiency  systematic uncertainty,   n_sb_error      = %6.1f %%\n", n_sb_error      ) ;
             printf("  SUSY efficiency  systematic uncertainty,   n_sig_sl_e_error  = %6.1f %%\n", n_sig_sl_e_error  ) ;
             printf("  SUSY efficiency  systematic uncertainty,   n_sig_sl_mu_error  = %6.1f %%\n", n_sig_sl_mu_error  ) ;
             printf("  SUSY efficiency  systematic uncertainty,   n_sb_sl_e_error   = %6.1f %%\n", n_sb_sl_e_error   ) ;
             printf("  SUSY efficiency  systematic uncertainty,   n_sb_sl_mu_error   = %6.1f %%\n", n_sb_sl_mu_error   ) ;
             printf("  SUSY efficiency  systematic uncertainty,   n_sig_ldp_error = %6.1f %%\n", n_sig_ldp_error ) ;
             printf("  SUSY efficiency  systematic uncertainty,   n_sb_ldp_error  = %6.1f %%\n", n_sb_ldp_error  ) ;
             printf("\n\n") ;

      //--Include the stat error on the efficiency for t1bbbb.
             if ( isT1bbbb ) {

              //-- absolute raw eff
                 float n_sig_raw_eff     = n_sig_raw     / nGenPerPoint ;
                 float n_sb_raw_eff      = n_sb_raw      / nGenPerPoint ;
                 float n_sig_sl_e_raw_eff  = n_sig_sl_e_raw  / nGenPerPoint ;
                 float n_sig_sl_mu_raw_eff  = n_sig_sl_mu_raw  / nGenPerPoint ;
                 float n_sb_sl_e_raw_eff   = n_sb_sl_e_raw   / nGenPerPoint ;
                 float n_sb_sl_mu_raw_eff   = n_sb_sl_mu_raw   / nGenPerPoint ;
                 float n_sig_ldp_raw_eff = n_sig_ldp_raw / nGenPerPoint ;
                 float n_sb_ldp_raw_eff  = n_sb_ldp_raw  / nGenPerPoint ;


               //-- absolute stat err.
                 float n_sig_stat_error     =  sqrt(  n_sig_raw_eff    * ( 1.0 - n_sig_raw_eff     ) / nGenPerPoint ) ;
                 float n_sb_stat_error      =  sqrt(  n_sb_raw_eff     * ( 1.0 - n_sb_raw_eff      ) / nGenPerPoint ) ;
                 float n_sig_sl_e_stat_error  =  sqrt(  n_sig_sl_e_raw_eff * ( 1.0 - n_sig_sl_e_raw_eff  ) / nGenPerPoint ) ;
                 float n_sig_sl_mu_stat_error  =  sqrt(  n_sig_sl_mu_raw_eff * ( 1.0 - n_sig_sl_mu_raw_eff  ) / nGenPerPoint ) ;
                 float n_sb_sl_e_stat_error   =  sqrt(  n_sb_sl_e_raw_eff  * ( 1.0 - n_sb_sl_e_raw_eff   ) / nGenPerPoint ) ;
                 float n_sb_sl_mu_stat_error   =  sqrt(  n_sb_sl_mu_raw_eff  * ( 1.0 - n_sb_sl_mu_raw_eff   ) / nGenPerPoint ) ;
                 float n_sig_ldp_stat_error =  sqrt(  n_sig_ldp_raw_eff* ( 1.0 - n_sig_ldp_raw_eff ) / nGenPerPoint ) ;
                 float n_sb_ldp_stat_error  =  sqrt(  n_sb_ldp_raw_eff * ( 1.0 - n_sb_ldp_raw_eff  ) / nGenPerPoint ) ;

               //-- relative stat err in percent.
                 if ( n_sig_raw_eff     > 0 ) { n_sig_stat_error     = 100.* n_sig_stat_error     / n_sig_raw_eff     ; } else { n_sig_stat_error     = 0. ; }
                 if ( n_sb_raw_eff      > 0 ) { n_sb_stat_error      = 100.* n_sb_stat_error      / n_sb_raw_eff      ; } else { n_sb_stat_error      = 0. ; }
                 if ( n_sig_sl_e_raw_eff  > 0 ) { n_sig_sl_e_stat_error  = 100.* n_sig_sl_e_stat_error  / n_sig_sl_e_raw_eff  ; } else { n_sig_sl_e_stat_error  = 0. ; }
                 if ( n_sig_sl_mu_raw_eff  > 0 ) { n_sig_sl_mu_stat_error  = 100.* n_sig_sl_mu_stat_error  / n_sig_sl_mu_raw_eff  ; } else { n_sig_sl_mu_stat_error  = 0. ; }
                 if ( n_sb_sl_e_raw_eff   > 0 ) { n_sb_sl_e_stat_error   = 100.* n_sb_sl_e_stat_error   / n_sb_sl_e_raw_eff   ; } else { n_sb_sl_e_stat_error   = 0. ; }
                 if ( n_sb_sl_mu_raw_eff   > 0 ) { n_sb_sl_mu_stat_error   = 100.* n_sb_sl_mu_stat_error   / n_sb_sl_mu_raw_eff   ; } else { n_sb_sl_mu_stat_error   = 0. ; }
                 if ( n_sig_ldp_raw_eff > 0 ) { n_sig_ldp_stat_error = 100.* n_sig_ldp_stat_error / n_sig_ldp_raw_eff ; } else { n_sig_ldp_stat_error = 0. ; }
                 if ( n_sb_ldp_raw_eff  > 0 ) { n_sb_ldp_stat_error  = 100.* n_sb_ldp_stat_error  / n_sb_ldp_raw_eff  ; } else { n_sb_ldp_stat_error  = 0. ; }

                 printf("  SUSY efficiency  statistical uncertainty,   n_sig_stat_error     = %6.1f %%\n", n_sig_stat_error     ) ;
                 printf("  SUSY efficiency  statistical uncertainty,   n_sb_stat_error      = %6.1f %%\n", n_sb_stat_error      ) ;
                 printf("  SUSY efficiency  statistical uncertainty,   n_sig_sl_e_stat_error  = %6.1f %%\n", n_sig_sl_e_stat_error  ) ;
		 printf("  SUSY efficiency  statistical uncertainty,   n_sig_sl_mu_stat_error  = %6.1f %%\n", n_sig_sl_mu_stat_error  ) ;
                 printf("  SUSY efficiency  statistical uncertainty,   n_sb_sl_e_stat_error   = %6.1f %%\n", n_sb_sl_e_stat_error   ) ;
                 printf("  SUSY efficiency  statistical uncertainty,   n_sb_sl_mu_stat_error   = %6.1f %%\n", n_sb_sl_mu_stat_error   ) ;
                 printf("  SUSY efficiency  statistical uncertainty,   n_sig_ldp_stat_error = %6.1f %%\n", n_sig_ldp_stat_error ) ;
                 printf("  SUSY efficiency  statistical uncertainty,   n_sb_ldp_stat_error  = %6.1f %%\n", n_sb_ldp_stat_error  ) ;

               //-- total err in percent.
                 n_sig_error     = sqrt( pow( n_sig_error    , 2) + pow( n_sig_stat_error    , 2) ) ;
                 n_sb_error      = sqrt( pow( n_sb_error     , 2) + pow( n_sb_stat_error     , 2) ) ;
                 n_sig_sl_e_error  = sqrt( pow( n_sig_sl_e_error , 2) + pow( n_sig_sl_e_stat_error , 2) ) ;
                 n_sig_sl_mu_error  = sqrt( pow( n_sig_sl_mu_error , 2) + pow( n_sig_sl_mu_stat_error , 2) ) ;
                 n_sb_sl_e_error   = sqrt( pow( n_sb_sl_e_error  , 2) + pow( n_sb_sl_e_stat_error  , 2) ) ;
                 n_sb_sl_mu_error   = sqrt( pow( n_sb_sl_mu_error  , 2) + pow( n_sb_sl_mu_stat_error  , 2) ) ;
                 n_sig_ldp_error = sqrt( pow( n_sig_ldp_error, 2) + pow( n_sig_ldp_stat_error, 2) ) ;
                 n_sb_ldp_error  = sqrt( pow( n_sb_ldp_error , 2) + pow( n_sb_ldp_stat_error , 2) ) ;

                 printf("\n\n") ;
                 printf("  SUSY efficiency  total uncertainty,   n_sig_error     = %6.1f %%\n", n_sig_error     ) ;
                 printf("  SUSY efficiency  total uncertainty,   n_sb_error      = %6.1f %%\n", n_sb_error      ) ;
                 printf("  SUSY efficiency  total uncertainty,   n_sig_sl_e_error  = %6.1f %%\n", n_sig_sl_e_error  ) ;
		 printf("  SUSY efficiency  total uncertainty,   n_sig_sl_mu_error  = %6.1f %%\n", n_sig_sl_mu_error  ) ;
                 printf("  SUSY efficiency  total uncertainty,   n_sb_sl_e_error   = %6.1f %%\n", n_sb_sl_e_error   ) ;
                 printf("  SUSY efficiency  total uncertainty,   n_sb_sl_mu_error   = %6.1f %%\n", n_sb_sl_mu_error   ) ;
                 printf("  SUSY efficiency  total uncertainty,   n_sig_ldp_error = %6.1f %%\n", n_sig_ldp_error ) ;
                 printf("  SUSY efficiency  total uncertainty,   n_sb_ldp_error  = %6.1f %%\n", n_sb_ldp_error  ) ;
                 printf("\n\n") ;

             }

       //--- Not needed with log-normal
        ///  //-- enforce a maximum efficiency uncertainty (to avoid negative scale factors).
        ///  if ( n_sig_error     > 35. ) { n_sig_error     = 35. ; }
        ///  if ( n_sb_error      > 35. ) { n_sb_error      = 35. ; }
        ///  if ( n_sig_sl_error  > 35. ) { n_sig_sl_error  = 35. ; }
        ///  if ( n_sb_sl_error   > 35. ) { n_sb_sl_error   = 35. ; }
        ///  if ( n_sig_ldp_error > 35. ) { n_sig_ldp_error = 35. ; }
        ///  if ( n_sb_ldp_error  > 35. ) { n_sb_ldp_error  = 35. ; }

      //++++++++++++++++++++++++++++++++++++++++++++++++



             double setVal_n_sig(0.) ;
             double setVal_n_sb(0.) ;
             double setVal_n_sig_sl_e(0.) ;
             double setVal_n_sig_sl_mu(0.) ;
             double setVal_n_sb_sl_e(0.) ;
             double setVal_n_sb_sl_mu(0.) ;
             double setVal_n_sig_ldp(0.) ;
             double setVal_n_sb_ldp(0.) ;


             if ( !isT1bbbb ) {
                //-- tanb40
                setVal_n_sig     = n_sig_raw     * n_sig_correction     ;
                setVal_n_sb      = n_sb_raw      * n_sb_correction      ;
                setVal_n_sig_sl_e  = n_sig_sl_e_raw  * n_sig_sl_e_correction  ;
                setVal_n_sig_sl_mu  = n_sig_sl_mu_raw  * n_sig_sl_mu_correction  ;
                setVal_n_sb_sl_e   = n_sb_sl_e_raw   * n_sb_sl_e_correction   ;
                setVal_n_sb_sl_mu   = n_sb_sl_mu_raw   * n_sb_sl_mu_correction   ;
                setVal_n_sig_ldp = n_sig_ldp_raw * n_sig_ldp_correction ;
                setVal_n_sb_ldp  = n_sb_ldp_raw  * n_sb_ldp_correction  ;
             } else {
                //-- t1bbbb
                setVal_n_sig     = DataLumi * t1bbbbXsec * (( n_sig_raw     * n_sig_correction     )/ nGenPerPoint ) ;
                setVal_n_sb      = DataLumi * t1bbbbXsec * (( n_sb_raw      * n_sb_correction      )/ nGenPerPoint ) ;
                setVal_n_sig_sl_e  = DataLumi * t1bbbbXsec * (( n_sig_sl_e_raw  * n_sig_sl_e_correction  )/ nGenPerPoint ) ;
                setVal_n_sig_sl_mu  = DataLumi * t1bbbbXsec * (( n_sig_sl_mu_raw  * n_sig_sl_mu_correction  )/ nGenPerPoint ) ;
                setVal_n_sb_sl_e   = DataLumi * t1bbbbXsec * (( n_sb_sl_e_raw   * n_sb_sl_e_correction   )/ nGenPerPoint ) ;
                setVal_n_sb_sl_mu   = DataLumi * t1bbbbXsec * (( n_sb_sl_mu_raw   * n_sb_sl_mu_correction   )/ nGenPerPoint ) ;
                setVal_n_sig_ldp = DataLumi * t1bbbbXsec * (( n_sig_ldp_raw * n_sig_ldp_correction )/ nGenPerPoint ) ;
                setVal_n_sb_ldp  = DataLumi * t1bbbbXsec * (( n_sb_ldp_raw  * n_sb_ldp_correction  )/ nGenPerPoint ) ;
             }

             //if ( !isT1bbbb ) {
             //   //-- tanb40
             //   setVal_n_sig     = n_sig_raw        ;
             //   setVal_n_sb      = n_sb_raw           ;
             //   setVal_n_sig_sl_e  = n_sig_sl_e_raw    ;
             //   setVal_n_sig_sl_mu  = n_sig_sl_mu_raw    ;
             //   setVal_n_sb_sl_e   = n_sb_sl_e_raw    ;
             //   setVal_n_sb_sl_mu   = n_sb_sl_mu_raw    ;
             //   setVal_n_sig_ldp = n_sig_ldp_raw  ;
             //   setVal_n_sb_ldp  = n_sb_ldp_raw   ;
             //} else {
             //   //-- t1bbbb
             //   setVal_n_sig     = DataLumi * t1bbbbXsec * (( n_sig_raw    )/ nGenPerPoint ) ;
             //   setVal_n_sb      = DataLumi * t1bbbbXsec * (( n_sb_raw     )/ nGenPerPoint ) ;
             //   setVal_n_sig_sl_e  = DataLumi * t1bbbbXsec * (( n_sig_sl_e_raw  )/ nGenPerPoint ) ;
             //   setVal_n_sig_sl_mu  = DataLumi * t1bbbbXsec * (( n_sig_sl_mu_raw   )/ nGenPerPoint ) ;
             //   setVal_n_sb_sl_e   = DataLumi * t1bbbbXsec * (( n_sb_sl_e_raw      )/ nGenPerPoint ) ;
             //   setVal_n_sb_sl_mu   = DataLumi * t1bbbbXsec * (( n_sb_sl_mu_raw    )/ nGenPerPoint ) ;
             //   setVal_n_sig_ldp = DataLumi * t1bbbbXsec * (( n_sig_ldp_raw )/ nGenPerPoint ) ;
             //   setVal_n_sb_ldp  = DataLumi * t1bbbbXsec * (( n_sb_ldp_raw    )/ nGenPerPoint ) ;
             //}

	     //Setup better error definitions with Gamma functions

             rv_mu_susymc_sig       -> setVal( setVal_n_sig      ) ;
             rv_mu_susymc_sb        -> setVal( setVal_n_sb       ) ;
             rv_mu_susymc_sig_sl_e    -> setVal( setVal_n_sig_sl_e   ) ;
             rv_mu_susymc_sig_sl_mu    -> setVal( setVal_n_sig_sl_mu   ) ;
             rv_mu_susymc_sb_sl_e     -> setVal( setVal_n_sb_sl_e    ) ;
             rv_mu_susymc_sb_sl_mu     -> setVal( setVal_n_sb_sl_mu    ) ;
             rv_mu_susymc_sig_ldp   -> setVal( setVal_n_sig_ldp  ) ;
             rv_mu_susymc_sb_ldp    -> setVal( setVal_n_sb_ldp   ) ;

             rv_width_eff_sf_sig     -> setVal( n_sig_error     / 100. ) ;
             rv_width_eff_sf_sb      -> setVal( n_sb_error      / 100. ) ;
             rv_width_eff_sf_sig_sl_e  -> setVal( n_sig_sl_e_error  / 100. ) ;
             rv_width_eff_sf_sig_sl_mu  -> setVal( n_sig_sl_mu_error  / 100. ) ;
             rv_width_eff_sf_sb_sl_e   -> setVal( n_sb_sl_e_error   / 100. ) ;
             rv_width_eff_sf_sb_sl_mu   -> setVal( n_sb_sl_mu_error   / 100. ) ;
             rv_width_eff_sf_sig_ldp -> setVal( n_sig_ldp_error / 100. ) ;
             rv_width_eff_sf_sb_ldp  -> setVal( n_sb_ldp_error  / 100. ) ;

             //rv_mean_eff_sf_sig     -> setVal( n_sig_correction      ) ;
             //rv_mean_eff_sf_sb      -> setVal( n_sb_correction       ) ;
             //rv_mean_eff_sf_sig_sl_e  -> setVal( n_sig_sl_e_correction   ) ;
             //rv_mean_eff_sf_sig_sl_mu  -> setVal( n_sig_sl_mu_correction   ) ;
             //rv_mean_eff_sf_sb_sl_e   -> setVal( n_sb_sl_e_correction    ) ;
             //rv_mean_eff_sf_sb_sl_mu   -> setVal( n_sb_sl_mu_correction    ) ;
             //rv_mean_eff_sf_sig_ldp -> setVal( n_sig_ldp_correction  ) ;
             //rv_mean_eff_sf_sb_ldp  -> setVal( n_sb_ldp_correction   ) ;



             if ( !isT1bbbb ) {
                printf("\n\n Found point m0 = %4.0f,  m1/2 = %4.0f,  Npred = %7.1f\n\n\n", pointM0, pointM12, setVal_n_sig ) ;
             } else {
                printf("\n\n Found point mGluino = %4.0f,  mLSP = %4.0f,  Npred = %7.1f\n\n\n", pointM0, pointM12, setVal_n_sig ) ;
             }


             printf("\n\n") ;
             printf(" Setting susy N_sig     to  %7.1f\n", setVal_n_sig       ) ;
             printf(" Setting susy N_sb      to  %7.1f\n", setVal_n_sb        ) ;
             printf(" Setting susy N_sig_sl_e  to  %7.1f\n", setVal_n_sig_sl_e    ) ;
             printf(" Setting susy N_sig_sl_mu  to  %7.1f\n", setVal_n_sig_sl_mu    ) ;
             printf(" Setting susy N_sb_sl_e   to  %7.1f\n", setVal_n_sb_sl_e     ) ;
             printf(" Setting susy N_sb_sl_mu   to  %7.1f\n", setVal_n_sb_sl_mu     ) ;
             printf(" Setting susy N_sig_ldp to  %7.1f\n", setVal_n_sig_ldp   ) ;
             printf(" Setting susy N_sb_ldp  to  %7.1f\n", setVal_n_sb_ldp    ) ;
             printf("\n\n") ;

             found = true ;

             break ;

          } // point match?

       } // not eof ?

       fclose( infp ) ;

       if ( found ) {
          return true ;
       } else {
          printf("\n\n *** Point not found in scan.\n\n" ) ;
          return false ;
       }

    } // setSusyScanPoint.

