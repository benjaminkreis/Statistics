


   void run_ws_cls_hybrid1() {

      gROOT->LoadMacro("ws_cls_hybrid1.c+") ;

      TFile* outf = new TFile("cls-expected-2BT.root","recreate") ;

      gDirectory->pwd() ;

      bool isBgonlyStudy ;
      double poiVal ;
      int nToys ;
      bool makeTtree ;

      makeTtree = true ;

      nToys = 1000 ;

    //-------

      for ( int poii=0; poii<10; poii++ ) {

         poiVal = 5. + 5*poii ;

         isBgonlyStudy = false ;
         ws_cls_hybrid1( "output-files/expected-ws-lm9-2BT.root", isBgonlyStudy, poiVal, nToys, makeTtree ) ;

         isBgonlyStudy = true ;
         ws_cls_hybrid1( "output-files/expected-ws-lm9-2BT.root", isBgonlyStudy, poiVal, nToys, makeTtree ) ;

      }



      outf->ls() ;


      outf->Close() ;

   }



