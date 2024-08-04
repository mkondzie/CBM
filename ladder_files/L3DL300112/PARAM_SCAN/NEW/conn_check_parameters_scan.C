#include <TFile.h>
#include <TString.h>
#include <TTree.h>
#include <fstream>
#include <iostream>
#include <string>

void conn_check_parameters_scan(
    TString filename = "conn_check_parameters_20.txt") {

  Ssiz_t start = filename.Last('_') + 1;
  Ssiz_t end = filename.First(".");

  // Determine the true number of broken channels (labeled by eye)
  TString true_n_brk_ch_string = filename(start, end - start);
  Int_t true_n_brk_ch = true_n_brk_ch_string.Atoi();

  // convert TString to std::string
  std::ifstream file(filename.Data());

  TString line;
  TString format("%lf %lf %lf %lf %lf");
  TString rootFilename = TString::Format("conn_check_parameters_%d", true_n_brk_ch);
  TFile *rootFile = new TFile(rootFilename + ".root", "RECREATE");
  TTree *tree = new TTree("param_scan", rootFilename);

  Double_t z_alpha, n_neigh_ch, sus_perc, n_sigmas, n_brk_ch;
  Double_t ratio_brk_ch, diff_brk_ch;
  // Set branch addresses
  tree->Branch("z_alpha", &z_alpha);
  tree->Branch("n_neigh_ch", &n_neigh_ch);
  tree->Branch("sus_perc", &sus_perc);
  tree->Branch("n_sigmas", &n_sigmas);
  tree->Branch("n_brk_ch", &n_brk_ch);
  tree->Branch("ratio_brk_ch", &ratio_brk_ch);
  tree->Branch("diff_brk_ch", &diff_brk_ch);

  ///////////////
  while (line.ReadLine(file)) {

    if (sscanf(line.Data(), format.Data(), &z_alpha, &n_neigh_ch, &sus_perc,
               &n_sigmas, &n_brk_ch) != 5) {
      continue;
    }
        ratio_brk_ch = n_brk_ch / true_n_brk_ch;
        diff_brk_ch = n_brk_ch - true_n_brk_ch;

    tree->Fill();
    // std::cout << z_alpha << "\t" << n_neigh_ch << "\t" << sus_perc << "\t"
    //           << n_sigmas << "\t" <<  n_brk_ch << '\n';
  }
  
  file.close();
  tree->Write();
  rootFile->Close();
}
