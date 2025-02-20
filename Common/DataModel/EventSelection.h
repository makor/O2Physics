// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#ifndef COMMON_DATAMODEL_EVENTSELECTION_H_
#define COMMON_DATAMODEL_EVENTSELECTION_H_

#include "Framework/AnalysisDataModel.h"
#include "Common/CCDB/TriggerAliases.h"
#include "Common/CCDB/EventSelectionParams.h"

namespace o2::aod
{

// Bits in eventCuts bitmask in Run2BCInfos table
// Must be consistent with EventSelectionCut enum in the Run2 converter
enum Run2EventCuts {
  kINELgtZERO = 0,
  kPileupInMultBins,
  kConsistencySPDandTrackVertices,
  kTrackletsVsClusters,
  kNonZeroNContribs,
  kIncompleteDAQ,
  kPileUpMV,
  kTPCPileUp,
  kTimeRangeCut,
  kEMCALEDCut,
  kAliEventCutsAccepted,
  kIsPileupFromSPD,
  kIsV0PFPileup,
  kIsTPCHVdip,
  kIsTPCLaserWarmUp,
  kTRDHCO, // Offline TRD cosmic trigger decision
  kTRDHJT, // Offline TRD jet trigger decision
  kTRDHSE, // Offline TRD single electron trigger decision
  kTRDHQU, // Offline TRD quarkonium trigger decision
  kTRDHEE  // Offline TRD single-electron-in-EMCAL-acceptance trigger decision
};

// collision-joinable event selection decisions
namespace evsel
{
DECLARE_SOA_BITMAP_COLUMN(Alias, alias, 32);
DECLARE_SOA_BITMAP_COLUMN(Selection, selection, 64);
DECLARE_SOA_COLUMN(BBV0A, bbV0A, bool);                                     //! Beam-beam time in V0A
DECLARE_SOA_COLUMN(BBV0C, bbV0C, bool);                                     //! Beam-beam time in V0C
DECLARE_SOA_COLUMN(BGV0A, bgV0A, bool);                                     //! Beam-gas time in V0A
DECLARE_SOA_COLUMN(BGV0C, bgV0C, bool);                                     //! Beam-gas time in V0C
DECLARE_SOA_COLUMN(BBFDA, bbFDA, bool);                                     //! Beam-beam time in FDA
DECLARE_SOA_COLUMN(BBFDC, bbFDC, bool);                                     //! Beam-beam time in FDC
DECLARE_SOA_COLUMN(BGFDA, bgFDA, bool);                                     //! Beam-gas time in FDA
DECLARE_SOA_COLUMN(BGFDC, bgFDC, bool);                                     //! Beam-gas time in FDC
DECLARE_SOA_COLUMN(MultRingV0A, multRingV0A, float[5]);                     //! V0A multiplicity per ring (4 rings in run2, 5 rings in run3)
DECLARE_SOA_COLUMN(MultRingV0C, multRingV0C, float[4]);                     //! V0C multiplicity per ring (4 rings in run2)
DECLARE_SOA_COLUMN(SpdClusters, spdClusters, uint32_t);                     //! Number of SPD clusters in two layers
DECLARE_SOA_COLUMN(NTracklets, nTracklets, int);                            //! Tracklet multiplicity
DECLARE_SOA_COLUMN(Sel7, sel7, bool);                                       //! Event selection decision based on V0A & V0C
DECLARE_SOA_COLUMN(Sel8, sel8, bool);                                       //! Event selection decision based on TVX
DECLARE_SOA_INDEX_COLUMN_FULL(FoundBC, foundBC, int, BCs, "_foundBC");      //! BC entry index in BCs table (-1 if doesn't exist)
DECLARE_SOA_INDEX_COLUMN_FULL(FoundFT0, foundFT0, int, FT0s, "_foundFT0");  //! FT0 entry index in FT0s table (-1 if doesn't exist)
DECLARE_SOA_INDEX_COLUMN_FULL(FoundFV0, foundFV0, int, FV0As, "_foundFV0"); //! FV0 entry index in FV0As table (-1 if doesn't exist)
DECLARE_SOA_INDEX_COLUMN_FULL(FoundFDD, foundFDD, int, FDDs, "_foundFDD");  //! FDD entry index in FDDs table (-1 if doesn't exist)
DECLARE_SOA_INDEX_COLUMN_FULL(FoundZDC, foundZDC, int, Zdcs, "_foundZDC");  //! ZDC entry index in ZDCs table (-1 if doesn't exist)
} // namespace evsel
DECLARE_SOA_TABLE(EvSels, "AOD", "EVSEL", //!
                  evsel::Alias, evsel::Selection,
                  evsel::BBV0A, evsel::BBV0C, evsel::BGV0A, evsel::BGV0C,
                  evsel::BBFDA, evsel::BBFDC, evsel::BGFDA, evsel::BGFDC,
                  evsel::MultRingV0A, evsel::MultRingV0C, evsel::SpdClusters, evsel::NTracklets,
                  evsel::Sel7, evsel::Sel8, evsel::FoundBCId, evsel::FoundFT0Id, evsel::FoundFV0Id, evsel::FoundFDDId, evsel::FoundZDCId);
using EvSel = EvSels::iterator;

DECLARE_SOA_TABLE(BcSels, "AOD", "BCSEL", //!
                  evsel::Alias, evsel::Selection,
                  evsel::BBV0A, evsel::BBV0C, evsel::BGV0A, evsel::BGV0C,
                  evsel::BBFDA, evsel::BBFDC, evsel::BGFDA, evsel::BGFDC,
                  evsel::MultRingV0A, evsel::MultRingV0C, evsel::SpdClusters, evsel::FoundFT0Id, evsel::FoundFV0Id, evsel::FoundFDDId, evsel::FoundZDCId);
using BcSel = BcSels::iterator;
} // namespace o2::aod

#endif // COMMON_DATAMODEL_EVENTSELECTION_H_
