
void make_newmcinp_lm9_ws_3B() {

  gROOT->LoadMacro("ra2bRoostatsClass7.c+") ;

  ra2bRoostatsClass7 ra2b(1,0) ;

  ra2b.initialize("paper-2011-files/input-files/likelihood-input-from-mc-HT400-SIGMET250-nb3.txt",
                  "paper-2011-files/input-files/signalSyst.LM9.ge3bLoose.dat", 0., 0., false, 1.0 ) ;

  gSystem->Exec("mkdir -p output-files") ;
  gSystem->Exec("mv output-files/ws-newmcinp-lm9-3B.root output-files/ws-newmcinp-lm9-3B.root-old") ;
  gSystem->Exec("mv ws.root output-files/ws-newmcinp-lm9-3B.root") ;

}



