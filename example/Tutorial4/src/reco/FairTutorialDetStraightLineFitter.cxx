#include "FairTutorialDetStraightLineFitter.h"
#include "FairTutorialDetHit.h"

#include "FairLogger.h"

#include "FairTrackParam.h"

#include "TF1.h"
#include "TGraphErrors.h"
#include "TMatrixFSym.h"

// ---- Default constructor -------------------------------------------
FairTutorialDetStraightLineFitter::FairTutorialDetStraightLineFitter()
  :FairTask("FairTutorialDetStraightLineFitter"),
   fHits(NULL),
   fTracks(NULL),
   fVersion(1)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of FairTutorialDetStraightLineFitter");
}

// ---- Destructor ----------------------------------------------------
FairTutorialDetStraightLineFitter::~FairTutorialDetStraightLineFitter()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of FairTutorialDetStraightLineFitter");
  if (fTracks) {
    fTracks->Delete();
    delete fTracks;
  }
}

// ----  Initialisation  ----------------------------------------------
void FairTutorialDetStraightLineFitter::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of FairTutorialDetStraightLineFitter");
  // Load all necessary parameter containers from the runtime data base
  /*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  <FairTutorialDetStraightLineFitterDataMember> = (<ClassPointer>*)
    (rtdb->getContainer("<ContainerName>"));
  */
}

// ---- Init ----------------------------------------------------------
InitStatus FairTutorialDetStraightLineFitter::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of FairTutorialDetStraightLineFitter");

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  // Get a pointer to the previous already existing data level

  fHits = (TClonesArray*) ioman->GetObject("TutorialDetHit");
  if ( ! fHits ) {
    LOG(ERROR)<<"No InputDataLevelName array!\n"
              << "FairTutorialDetStraightLineFitter will be inactive"<<FairLogger::endl;
    return kERROR;
  }

  // Create the TClonesArray for the output data and register
  // it in the IO manager
  fTracks = new TClonesArray("FairTrackParam", 100);
  ioman->Register("TutorialDetTrack","TutorialDet",fTracks,kTRUE);


  // Do whatever else is needed at the initilization stage
  // Create histograms to be filled
  // initialize variables

  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus FairTutorialDetStraightLineFitter::ReInit()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of FairTutorialDetStraightLineFitter");
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void FairTutorialDetStraightLineFitter::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of FairTutorialDetStraightLineFitter");

  // Declare some variables
  FairTutorialDetHit* hit = NULL;
  Int_t detID   = 0;        // Detector ID
  Int_t trackID = 0;        // Track index
  Double_t x, y, z;         // Position
  Double_t dx = 0.1;        // Position error
  Double_t tof = 0.;        // Time of flight
  TVector3 pos, dpos;       // Position and error vectors

  // Loop over TofPoints
  Int_t nHits = fHits->GetEntriesFast();
  Float_t* ZPos = new Float_t[nHits];
  Float_t* XPos = new Float_t[nHits];
  Float_t* XPosErr = new Float_t[nHits];
  Float_t* YPos = new Float_t[nHits];
  Float_t* YPosErr = new Float_t[nHits];

  if(40 == nHits) {
    for (Int_t iHit=0; iHit<nHits; iHit++) {
      hit = (FairTutorialDetHit*) fHits->At(iHit);
      if ( ! hit) { continue; }

      XPos[iHit] = hit->GetX();
      YPos[iHit] = hit->GetY();
      ZPos[iHit] = hit->GetZ();

      XPosErr[iHit] = hit->GetDx();
      YPosErr[iHit] = hit->GetDy();
    }
    TF1* f1 = new TF1("f1", "[0]*x + [1]");
    TGraphErrors* LineGraph;

    LineGraph = new TGraphErrors(nHits, ZPos, XPos, 0, XPosErr);
    LineGraph->Fit("f1", "Q");
    Double_t SlopeX = f1->GetParameter(0);
    Double_t OffX = f1->GetParameter(1);
    Double_t Chi2X = f1->GetChisquare();
    Double_t SlopeY;
    Double_t OffY;
    Double_t Chi2Y;

    if ( 1 == fVersion ) {
      LineGraph = new TGraphErrors(nHits, ZPos, YPos, 0, YPosErr);
      LineGraph->Fit("f1", "Q");
      SlopeY = f1->GetParameter(0);
      OffY = f1->GetParameter(1);
      Chi2Y = f1->GetChisquare();

      LOG(INFO)<<XPos[0]<<","<<XPos[nHits-1]<<","<<YPos[0]<<","<<YPos[nHits-1]<<","<<ZPos[0]<<","<<ZPos[nHits-1]<<FairLogger::endl;
      Double_t XSlope = (XPos[nHits-1]-XPos[0])/(ZPos[nHits-1]-ZPos[0]);
      Double_t YSlope = (YPos[nHits-1]-YPos[0])/(ZPos[nHits-1]-ZPos[0]);

      LOG(INFO)<<"Slope(x,y): "<<SlopeX<<" ,"<<SlopeY<<FairLogger::endl;
      LOG(INFO)<<"Slope1(x,y): "<<XSlope<<" ,"<<YSlope<<FairLogger::endl;
      LOG(INFO)<<"Offset(x,y): "<<OffX<<" ,"<<OffY<<FairLogger::endl;
      LOG(INFO)<<"Chi2(x,y): "<<Chi2X<<" ,"<<Chi2Y<<FairLogger::endl;

    }

    FairTrackParam* track = new FairTrackParam();
    track->SetX(OffX);
    track->SetTx(SlopeX);
    track->SetZ(0.);
    if ( 1 == fVersion ) {
      track->SetY(OffY);
      track->SetTy(SlopeY);
    }
    new ((*fTracks)[0]) FairTrackParam(*track);
//  const TMatrixFSym matrix;
//  Double_t Z = 0.;
//  new ((*fTracks)[0]) FairTrackParam(OffX, OffY, Z, SlopeX, SlopeY, matrix);

  }
}

// ---- Finish --------------------------------------------------------
void FairTutorialDetStraightLineFitter::Finish()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Finish of FairTutorialDetStraightLineFitter");
}

ClassImp(FairTutorialDetStraightLineFitter)