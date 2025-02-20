// Copyright 2019-2022 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file femtoUniversePairTaskTrackTrack.cxx
/// \brief Tasks that reads the track tables used for the pairing and builds pairs of two tracks
/// \author Andi Mathis, TU München, andreas.mathis@ph.tum.de
/// \author Zuzanna Chochulska, WUT Warsaw, zuzanna.chochulska.stud@pw.edu.pl

#include <vector>
#include "Framework/AnalysisTask.h"
#include "Framework/runDataProcessing.h"
#include "Framework/HistogramRegistry.h"
#include "Framework/ASoAHelpers.h"
#include "Framework/RunningWorkflowInfo.h"
#include "Framework/StepTHn.h"

#include "PWGCF/FemtoUniverse/DataModel/FemtoDerived.h"
#include "PWGCF/FemtoUniverse/Core/FemtoUniverseParticleHisto.h"
#include "PWGCF/FemtoUniverse/Core/FemtoUniverseEventHisto.h"
#include "PWGCF/FemtoUniverse/Core/FemtoUniversePairCleaner.h"
#include "PWGCF/FemtoUniverse/Core/FemtoUniverseContainer.h"
#include "PWGCF/FemtoUniverse/Core/FemtoUniverseDetaDphiStar.h"
#include "PWGCF/FemtoUniverse/Core/FemtoUtils.h"

using namespace o2;
using namespace o2::soa;
using namespace o2::framework;
using namespace o2::framework::expressions;
using namespace o2::analysis::femtoUniverse;

namespace
{
static constexpr int nTrack = 1;
static constexpr int nV0Children = 2;
static constexpr int nCuts = 5;
static const std::vector<std::string> TrackName{"Track"};
static const std::vector<std::string> V0ChildrenName{"PosChild", "NegChild"};
static const std::vector<std::string> cutNames{"MaxPt", "PIDthr", "nSigmaTPC", "nSigmaTPCTOF", "MaxP"};
static const float cutsTableTrack[nTrack][nCuts]{{4.05f, 0.75f, 3.f, 3.f, 100.f}};
static const float cutsTableV0Children[nV0Children][nCuts]{
  {90.f, 99.f, 5.f, 5.f, 100.f},
  {90.f, 99.f, 5.f, 5.f, 100.f}};
} // namespace

struct femtoUniversePairTaskTrackV0 {
  SliceCache cache;
  Preslice<aod::FDParticles> perCol = aod::femtouniverseparticle::fdCollisionId;

  /// Particle 1 (track)
  Configurable<LabeledArray<float>> ConfTrkCutTable{"ConfTrkCutTable", {cutsTableTrack[0], nTrack, nCuts, TrackName, cutNames}, "Particle selections"};
  Configurable<int> ConfTrkPDGCodePartOne{"ConfTrkPDGCodePartOne", 2212, "Particle 1 (Track) - PDG code"};
  Configurable<uint32_t> ConfTrkCutPartOne{"ConfTrkCutPartOne", 5542474, "Particle 1 (Track) - Selection bit from cutCulator"};
  Configurable<int> ConfTrkPIDPartOne{"ConfTrkPIDPartOne", 2, "Particle 1 - Read from cutCulator"};
  Configurable<int> ConfNspecies{"ConfNspecies", 2, "Number of particle spieces with PID info"};
  Configurable<std::vector<float>> ConfTrkPIDnSigmaMax{"ConfTrkPIDnSigmaMax", std::vector<float>{4.f, 3.f, 2.f}, "This configurable needs to be the same as the one used in the producer task"};
  ConfigurableAxis ConfTrkTempFitVarBins{"ConfTrkDTempFitVarBins", {300, -0.15, 0.15}, "binning of the TempFitVar in the pT vs. TempFitVar plot"};
  ConfigurableAxis ConfTrkTempFitVarpTBins{"ConfTrkTempFitVarpTBins", {20, 0.5, 4.05}, "pT binning of the pT vs. TempFitVar plot"};

  /// Partition for particle 1
  Partition<aod::FDParticles> partsOne = (aod::femtouniverseparticle::partType == uint8_t(aod::femtouniverseparticle::ParticleType::kTrack)) && ((aod::femtouniverseparticle::cut & ConfTrkCutPartOne) == ConfTrkCutPartOne);

  /// Histogramming for particle 1
  FemtoUniverseParticleHisto<aod::femtouniverseparticle::ParticleType::kTrack, 1> trackHistoPartOne;

  /// Particle 2 (V0)
  Configurable<LabeledArray<float>> ConfV0ChildrenCutTable{"ConfV0ChildrenCutTable", {cutsTableV0Children[0], nV0Children, nCuts, V0ChildrenName, cutNames}, "V0 Children selections"};
  Configurable<int> ConfV0PDGCodePartTwo{"ConfV0PDGCodePartTwo", 3122, "Particle 2 (V0) - PDG code"};
  Configurable<uint32_t> ConfV0CutPartTwo{"ConfV0CutPartTwo", 338, "Particle 2 (V0) - Selection bit"};
  ConfigurableAxis ConfV0TempFitVarBins{"ConfV0TempFitVarBins", {300, 0.95, 1.}, "V0: binning of the TempFitVar in the pT vs. TempFitVar plot"};
  ConfigurableAxis ConfV0TempFitVarpTBins{"ConfV0TempFitVarpTBins", {20, 0.5, 4.05}, "V0: pT binning of the pT vs. TempFitVar plot"};

  Configurable<uint32_t> ConfCutChildPos{"ConfCutChildPos", 150, "Positive Child of V0 - Selection bit from cutCulator"};
  Configurable<uint32_t> ConfCutChildNeg{"ConfCutChildNeg", 149, "Negative Child of V0 - Selection bit from cutCulator"};
  Configurable<int> ConfChildPosIndex{"ConfChildPosIndex", 1, "Positive Child of V0 - Index from cutCulator"};
  Configurable<int> ConfChildNegIndex{"ConfChildNegIndex", 0, "Negative Child of V0 - Index from cutCulator"};
  Configurable<std::vector<float>> ConfChildPIDnSigmaMax{"ConfChildPIDnSigmaMax", std::vector<float>{4.f, 3.f}, "V0 child sel: Max. PID nSigma TPC"};
  Configurable<int> ConfChildnSpecies{"ConfChildnSpecies", 2, "Number of particle spieces (for V0 children) with PID info"};
  ConfigurableAxis ConfChildTempFitVarBins{"ConfChildTempFitVarBins", {300, -0.15, 0.15}, "V0 child: binning of the TempFitVar in the pT vs. TempFitVar plot"};
  ConfigurableAxis ConfChildTempFitVarpTBins{"ConfChildTempFitVarpTBins", {20, 0.5, 4.05}, "V0 child: pT binning of the pT vs. TempFitVar plot"};

  /// Partition for particle 2
  Partition<aod::FDParticles> partsTwo = (aod::femtouniverseparticle::partType == uint8_t(aod::femtouniverseparticle::ParticleType::kV0)) && ((aod::femtouniverseparticle::cut & ConfV0CutPartTwo) == ConfV0CutPartTwo);

  /// Histogramming for particle 2
  FemtoUniverseParticleHisto<aod::femtouniverseparticle::ParticleType::kV0, 2> trackHistoPartTwo;
  FemtoUniverseParticleHisto<aod::femtouniverseparticle::ParticleType::kV0Child, 3> posChildHistos;
  FemtoUniverseParticleHisto<aod::femtouniverseparticle::ParticleType::kV0Child, 4> negChildHistos;

  /// Histogramming for Event
  FemtoUniverseEventHisto eventHisto;

  int vPIDPartOne;
  std::vector<float> kNsigma;

  /// Correlation part
  Configurable<bool> ConfIsMC{"ConfIsMC", false, "Enable additional Histogramms in the case of a MonteCarlo Run"};
  Configurable<bool> ConfUse3D{"ConfUse3D", false, "Enable three dimensional histogramms (to be used only for analysis with high statistics): k* vs mT vs multiplicity"};
  ConfigurableAxis ConfMultBins{"ConfMultBins", {VARIABLE_WIDTH, 0.0f, 20.0f, 40.0f, 60.0f, 80.0f, 100.0f, 200.0f, 99999.f}, "Mixing bins - multiplicity"};
  ConfigurableAxis ConfVtxBins{"ConfVtxBins", {VARIABLE_WIDTH, -10.0f, -8.f, -6.f, -4.f, -2.f, 0.f, 2.f, 4.f, 6.f, 8.f, 10.f}, "Mixing bins - z-vertex"};
  ConfigurableAxis ConfkstarBins{"ConfkstarBins", {1500, 0., 6.}, "binning kstar"};
  ConfigurableAxis ConfkTBins{"ConfkTBins", {150, 0., 9.}, "binning kT"};
  ConfigurableAxis ConfmTBins{"ConfmTBins", {225, 0., 7.5}, "binning mT"};
  Configurable<int> ConfNEventsMix{"ConfNEventsMix", 5, "Number of events for mixing"};
  Configurable<bool> ConfIsCPR{"ConfIsCPR", true, "Close Pair Rejection"};
  Configurable<bool> ConfCPRPlotPerRadii{"ConfCPRPlotPerRadii", false, "Plot CPR per radii"};
  Configurable<float> ConfCPRdeltaPhiMax{"ConfCPRdeltaPhiMax", 0.01, "Max. Delta Phi for Close Pair Rejection"};
  Configurable<float> ConfCPRdeltaEtaMax{"ConfCPRdeltaEtaMax", 0.01, "Max. Delta Eta for Close Pair Rejection"};

  ConfigurableAxis ConfmTBins3D{"ConfmTBins3D", {VARIABLE_WIDTH, 1.02f, 1.14f, 1.20f, 1.26f, 1.38f, 1.56f, 1.86f, 4.50f}, "mT Binning for the 3Dimensional plot: k* vs multiplicity vs mT (set <<ConfUse3D>> to true in order to use)"};
  ConfigurableAxis ConfmultBins3D{"ConfMultBins3D", {VARIABLE_WIDTH, 0.0f, 20.0f, 30.0f, 40.0f, 99999.0f}, "multiplicity Binning for the 3Dimensional plot: k* vs multiplicity vs mT (set <<ConfUse3D>> to true in order to use)"};

  FemtoUniverseContainer<femtoUniverseContainer::EventType::same, femtoUniverseContainer::Observable::kstar> sameEventCont;
  FemtoUniverseContainer<femtoUniverseContainer::EventType::mixed, femtoUniverseContainer::Observable::kstar> mixedEventCont;
  FemtoUniversePairCleaner<aod::femtouniverseparticle::ParticleType::kTrack, aod::femtouniverseparticle::ParticleType::kV0> pairCleaner;
  FemtoUniverseDetaDphiStar<aod::femtouniverseparticle::ParticleType::kTrack, aod::femtouniverseparticle::ParticleType::kV0> pairCloseRejection;
  /// Histogram output
  HistogramRegistry qaRegistry{"TrackQA", {}, OutputObjHandlingPolicy::AnalysisObject};
  HistogramRegistry resultRegistry{"Correlations", {}, OutputObjHandlingPolicy::AnalysisObject};

  void init(InitContext&)
  {
    eventHisto.init(&qaRegistry);
    trackHistoPartOne.init(&qaRegistry, ConfTrkTempFitVarpTBins, ConfTrkTempFitVarBins, ConfIsMC, ConfTrkPDGCodePartOne);
    trackHistoPartTwo.init(&qaRegistry, ConfV0TempFitVarpTBins, ConfV0TempFitVarBins, ConfIsMC, ConfV0PDGCodePartTwo);
    posChildHistos.init(&qaRegistry, ConfChildTempFitVarpTBins, ConfChildTempFitVarBins, false, false);
    negChildHistos.init(&qaRegistry, ConfChildTempFitVarpTBins, ConfChildTempFitVarBins, false, false);

    sameEventCont.init(&resultRegistry, ConfkstarBins, ConfMultBins, ConfkTBins, ConfmTBins, ConfmultBins3D, ConfmTBins3D, ConfIsMC, ConfUse3D);
    sameEventCont.setPDGCodes(ConfTrkPDGCodePartOne, ConfV0PDGCodePartTwo);
    mixedEventCont.init(&resultRegistry, ConfkstarBins, ConfMultBins, ConfkTBins, ConfmTBins, ConfmultBins3D, ConfmTBins3D, ConfIsMC, ConfUse3D);
    mixedEventCont.setPDGCodes(ConfTrkPDGCodePartOne, ConfV0PDGCodePartTwo);
    pairCleaner.init(&qaRegistry);
    if (ConfIsCPR.value) {
      pairCloseRejection.init(&resultRegistry, &qaRegistry, ConfCPRdeltaPhiMax.value, ConfCPRdeltaEtaMax.value, ConfCPRPlotPerRadii.value);
    }
    vPIDPartOne = ConfTrkPIDPartOne.value;
    kNsigma = ConfTrkPIDnSigmaMax.value;
  }

  /// This function processes the same event and takes care of all the histogramming
  /// \todo the trivial loops over the tracks should be factored out since they will be common to all combinations of T-T, T-V0, V0-V0, ...
  void processSameEvent(o2::aod::FDCollision& col, o2::aod::FDParticles& parts)
  {
    const auto& magFieldTesla = col.magField();

    auto groupPartsOne = partsOne->sliceByCached(aod::femtouniverseparticle::fdCollisionId, col.globalIndex(), cache);
    auto groupPartsTwo = partsTwo->sliceByCached(aod::femtouniverseparticle::fdCollisionId, col.globalIndex(), cache);
    const int multCol = col.multNtr();

    eventHisto.fillQA(col);

    /// Histogramming same event
    for (auto& part : groupPartsOne) {
      if (part.p() > ConfTrkCutTable->get("Track", "MaxP") || part.pt() > ConfTrkCutTable->get("Track", "MaxPt") ||
          !isFullPIDSelected(part.pidcut(), part.p(), ConfTrkCutTable->get("Track", "PIDthr"), vPIDPartOne, ConfNspecies, kNsigma, ConfTrkCutTable->get("Track", "nSigmaTPC"), ConfTrkCutTable->get("Track", "nSigmaTPCTOF"))) {
        continue;
      }
      trackHistoPartOne.fillQA<false, false>(part);
    }

    for (auto& part : groupPartsTwo) {
      const auto& posChild = parts.iteratorAt(part.index() - 2);
      const auto& negChild = parts.iteratorAt(part.index() - 1);
      // check cuts on V0 children
      if (!((posChild.cut() & ConfCutChildPos) == ConfCutChildPos) || !((negChild.cut() & ConfCutChildNeg) == ConfCutChildNeg) ||
          !isFullPIDSelected(posChild.pidcut(), posChild.p(), ConfV0ChildrenCutTable->get("PosChild", "PIDthr"), ConfChildPosIndex.value, ConfChildnSpecies.value, ConfChildPIDnSigmaMax.value, ConfV0ChildrenCutTable->get("PosChild", "nSigmaTPC"), ConfV0ChildrenCutTable->get("PosChild", "nSigmaTPCTOF")) ||
          !isFullPIDSelected(negChild.pidcut(), negChild.p(), ConfV0ChildrenCutTable->get("PosChild", "PIDthr"), ConfChildNegIndex.value, ConfChildnSpecies.value, ConfChildPIDnSigmaMax.value, ConfV0ChildrenCutTable->get("NegChild", "nSigmaTPC"), ConfV0ChildrenCutTable->get("NegChild", "nSigmaTPCTOF"))) {
        continue;
      }
      trackHistoPartTwo.fillQA<false, false>(part);
      posChildHistos.fillQA<false, false>(posChild);
      negChildHistos.fillQA<false, false>(negChild);
    }

    /// Now build the combinations
    for (auto& [p1, p2] : combinations(CombinationsFullIndexPolicy(groupPartsOne, groupPartsTwo))) {
      if (p1.p() > ConfTrkCutTable->get("Track", "MaxP") || p1.pt() > ConfTrkCutTable->get("Track", "MaxPt") ||
          !isFullPIDSelected(p1.pidcut(), p1.p(), ConfTrkCutTable->get("Track", "PIDthr"), vPIDPartOne, ConfNspecies, kNsigma, ConfTrkCutTable->get("Track", "nSigmaTPC"), ConfTrkCutTable->get("Track", "nSigmaTPCTOF"))) {
        continue;
      }
      const auto& posChild = parts.iteratorAt(p2.index() - 2);
      const auto& negChild = parts.iteratorAt(p2.index() - 1);
      // check cuts on V0 children
      if (!((posChild.cut() & ConfCutChildPos) == ConfCutChildPos) || !((negChild.cut() & ConfCutChildNeg) == ConfCutChildNeg) ||
          !isFullPIDSelected(posChild.pidcut(), posChild.p(), ConfV0ChildrenCutTable->get("PosChild", "PIDthr"), ConfChildPosIndex.value, ConfChildnSpecies.value, ConfChildPIDnSigmaMax.value, ConfV0ChildrenCutTable->get("PosChild", "nSigmaTPC"), ConfV0ChildrenCutTable->get("PosChild", "nSigmaTPCTOF")) ||
          !isFullPIDSelected(negChild.pidcut(), negChild.p(), ConfV0ChildrenCutTable->get("PosChild", "PIDthr"), ConfChildNegIndex.value, ConfChildnSpecies.value, ConfChildPIDnSigmaMax.value, ConfV0ChildrenCutTable->get("NegChild", "nSigmaTPC"), ConfV0ChildrenCutTable->get("NegChild", "nSigmaTPCTOF"))) {
        continue;
      }
      if (ConfIsCPR.value) {
        if (pairCloseRejection.isClosePair(p1, p2, parts, magFieldTesla)) {
          continue;
        }
      }
      // track cleaning
      if (!pairCleaner.isCleanPair(p1, p2, parts)) {
        continue;
      }
      sameEventCont.setPair<false>(p1, p2, multCol, ConfUse3D);
    }
  }

  PROCESS_SWITCH(femtoUniversePairTaskTrackV0, processSameEvent, "Enable processing same event", true);

  /// This function processes the mixed event
  /// \todo the trivial loops over the collisions and tracks should be factored out since they will be common to all combinations of T-T, T-V0, V0-V0, ...
  void processMixedEvent(o2::aod::FDCollisions& cols, o2::aod::FDParticles& parts)
  {
    ColumnBinningPolicy<aod::collision::PosZ, aod::femtouniversecollision::MultNtr> colBinning{{ConfVtxBins, ConfMultBins}, true};

    for (auto& [collision1, collision2] : soa::selfCombinations(colBinning, 5, -1, cols, cols)) {

      const int multCol = collision1.multNtr();

      auto groupPartsOne = partsOne->sliceByCached(aod::femtouniverseparticle::fdCollisionId, collision1.globalIndex(), cache);
      auto groupPartsTwo = partsTwo->sliceByCached(aod::femtouniverseparticle::fdCollisionId, collision2.globalIndex(), cache);

      const auto& magFieldTesla1 = collision1.magField();
      const auto& magFieldTesla2 = collision2.magField();

      if (magFieldTesla1 != magFieldTesla2) {
        continue;
      }

      for (auto& [p1, p2] : combinations(CombinationsFullIndexPolicy(groupPartsOne, groupPartsTwo))) {
        if (p1.p() > ConfTrkCutTable->get("Track", "MaxP") || p1.pt() > ConfTrkCutTable->get("Track", "MaxPt") ||
            !isFullPIDSelected(p1.pidcut(), p1.p(), ConfTrkCutTable->get("Track", "PIDthr"), vPIDPartOne, ConfNspecies, kNsigma, ConfTrkCutTable->get("Track", "nSigmaTPC"), ConfTrkCutTable->get("Track", "nSigmaTPCTOF"))) {
          continue;
        }
        const auto& posChild = parts.iteratorAt(p2.index() - 2);
        const auto& negChild = parts.iteratorAt(p2.index() - 1);
        // check cuts on V0 children
        if (!((posChild.cut() & ConfCutChildPos) == ConfCutChildPos) || !((negChild.cut() & ConfCutChildNeg) == ConfCutChildNeg) ||
            !isFullPIDSelected(posChild.pidcut(), posChild.p(), ConfV0ChildrenCutTable->get("PosChild", "PIDthr"), ConfChildPosIndex.value, ConfChildnSpecies.value, ConfChildPIDnSigmaMax.value, ConfV0ChildrenCutTable->get("PosChild", "nSigmaTPC"), ConfV0ChildrenCutTable->get("PosChild", "nSigmaTPCTOF")) ||
            !isFullPIDSelected(negChild.pidcut(), negChild.p(), ConfV0ChildrenCutTable->get("PosChild", "PIDthr"), ConfChildNegIndex.value, ConfChildnSpecies.value, ConfChildPIDnSigmaMax.value, ConfV0ChildrenCutTable->get("NegChild", "nSigmaTPC"), ConfV0ChildrenCutTable->get("NegChild", "nSigmaTPCTOF"))) {
          continue;
        }
        if (ConfIsCPR.value) {
          if (pairCloseRejection.isClosePair(p1, p2, parts, magFieldTesla1)) {
            continue;
          }
        }
        // track cleaning
        if (!pairCleaner.isCleanPair(p1, p2, parts)) {
          continue;
        }
        mixedEventCont.setPair<false>(p1, p2, multCol, ConfUse3D);
      }
    }
  }

  PROCESS_SWITCH(femtoUniversePairTaskTrackV0, processMixedEvent, "Enable processing mixed events", true);
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  WorkflowSpec workflow{
    adaptAnalysisTask<femtoUniversePairTaskTrackV0>(cfgc),
  };
  return workflow;
}
