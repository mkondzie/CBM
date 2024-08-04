#include "TProfile2D.h"
#include "TString.h"
#include "TStyle.h"
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TTreeFormula.h>

void conn_check_parameters_plot() {
  TFile *root_file = new TFile("conn_check_parameters.root", "READ");

  TTree *param_scan;
  root_file->GetObject("param_scan", param_scan);
  auto prof_z_alpha_n_neigh_ch_ratio =
      new TProfile2D("", "", 9, 0.2, 0.7, 12, 0, 40, -2, 4);

  Double_t z_alpha, n_neigh_ch;
  Double_t diff_brk_ch;

  param_scan->SetBranchAddress("z_alpha", &z_alpha);
  param_scan->SetBranchAddress("n_neigh_ch", &n_neigh_ch);
  param_scan->SetBranchAddress("diff_brk_ch", &diff_brk_ch);

  TString condition = "n_brk_ch < 128";
  TTreeFormula *formula = new TTreeFormula("", condition, param_scan);

  Long64_t n_entries = param_scan->GetEntries();
  for (Long64_t i = 0; i < n_entries; i++) {
    param_scan->GetEntry(i);
    //  evaluate the cut condition using the TTreeFormula
    if (formula->EvalInstance()) {
      prof_z_alpha_n_neigh_ch_ratio->Fill(z_alpha, n_neigh_ch, diff_brk_ch);
    }
  }
  
  TCanvas *canvas = new TCanvas("canvas", "", 670, 600);
  prof_z_alpha_n_neigh_ch_ratio->Draw("COLZ");
}
