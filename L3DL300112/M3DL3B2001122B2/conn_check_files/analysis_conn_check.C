#include "TObject.h"
#include "TString.h"
#include "string"
#include <TBrowser.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TPad.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>

TString filename_data;
std::vector<string> file_names;
std::vector<int> brk_ch_summary;
int vcnt[128][32];
// averages channel in the neighborhood for dynamic threshold
std::vector<float> neigh_even;
std::vector<float> neigh_odd;

void Analysis() {
  // ............... oooo00000oooo........................
  //! Variables for reading, analyzing data and displaying histograms
  int ch_min = 0;
  int ch_max = 128;
  int ch_step = 1;

  int d_min = 0;
  int d_max = 31;
  int d_step = 1;

  filename_data = (filename_data + ".txt");
  ifstream scanfile;
  scanfile.open(filename_data);
  int fch;
  std::string line;
  std::string ele;

  std::cout << "------------- Reading data file: -----------" << std::endl;
  int count = 0;
  int ch_cnt = 0;
  while (!scanfile.eof()) {
    std::getline(scanfile, line);
    std::istringstream iss(line);
    if (!line.empty()) {
      iss >> ele;
      iss >> ele;
      cout << " ch " << ch_cnt << " ";
      count = 0;
      for (int d = d_min; d < d_max + 1; d++) {
        iss >> count;
        vcnt[ch_cnt][d] = count;
        printf("%4d ", vcnt[ch_cnt][d]);
      }
      cout << endl;
      ch_cnt++;
    }
  }

  scanfile.close();
}

bool Read_file_tests() {
  // Path of the file with name hmodule_lab_test
  // ----------------------------------------------------------------------------------------------------------------
  TString file_list = "a";
  // ----------------------------------------------------------------------------------------------------------------
  ifstream myfile;
  myfile.open(file_list + ".txt");
  std::string line;
  std::string delimiter = ".txt";
  size_t pos = 0;
  std::string token;

  // reading list file and storing it in a vector
  cout << "......................oooo00000oooo..............................."
       << endl;
  while (!myfile.eof()) {
    std::getline(myfile, line);
    if (line.empty())
      continue;
    else {
      while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        cout << token << endl;
        line.erase(0, pos + delimiter.length());
      }
      file_names.push_back(token);
    }
  }
  cout << file_names.size() << endl;
  if (file_names.size() != 0)
    return false;
  return true;
}

// ............... oooo00000oooo........................
//! Get_file_name from the file list
std::string Get_file_name(int i) { return file_names[i]; }

void write_results(TString filename_str, vector<int> brk_ch) {
  ofstream myfile;
  myfile.open("conn_check_summary.txt", std::ios::out | std::ios::app);
  myfile << filename_str << ": ";
  if (brk_ch.size() != 0) {
    for (int i = 0; i < brk_ch.size(); i++) {
      myfile << brk_ch[i] << ",";
    }
    myfile << "\n";
  } else {
    myfile << "0"
           << "\n";
  }
  myfile.close();
}

template <typename T> T calculate_median(std::vector<T> v, size_t n) {
  // Sort the vector
  std::sort(v.begin(), v.end());

  // Check if the number of elements is odd
  if (n % 2 != 0) {
    return (double)v[n / 2];
  }

  // If the number of elements is even, return the average
  // of the two middle elements
  return (double)(v[(n - 1) / 2] + v[n / 2]) / 2.0;
}

// ------------------------ Main function ---------------------

int analysis_conn_check() {

  for (int ch = 0; ch < 128; ch++) {
    for (int d = 0; d < 32; d++) {
      vcnt[ch][31 - d] = 0;
    }
  }

  TString dir = "";
  TString filename_root = "";
  std::vector<int> broken_channels;
  std::vector<int> broken_channelsv2;
  // reading file with list of measurements files.
  Read_file_tests();
  // Analysis function is called for every file in the list. At the end,
  // everything is closed and root files can be accessed via TBrowser
  for (int i = 0; i < int(file_names.size()); i++) {
    filename_data = dir + Get_file_name(i).c_str();
    filename_root = dir + Get_file_name(i).c_str();
    cout << "Filename_selected: " << filename_data << endl;
    Analysis();

    broken_channels.clear();
    broken_channelsv2.clear();

    // Creating files
    TFile *file1 = new TFile(filename_root + ".root", "recreate");
    file1->cd();
    // Creating histograms
    TH1F *h_ave_hits = new TH1F("h_hits", "h_hits", 128, 0, 128);
    TH1F *h_broken_ch = new TH1F("h_broken_ch", "h_broken_ch", 128, 0, 128);
    TH1F *h_broken_ch_residuals =
        new TH1F("h_broken_ch_residuals", "h_broken_ch_residuals", 128, 0, 128);
    TH1F *h1_ave_even = new TH1F("h1_ave_even", "h1_ave_even", 100, 0, 400000);
    TH1F *h1_ave_odd = new TH1F("h1_ave_odd", "h1_ave_odd", 100, 0,
                                400000); // access these channels
    TH1F *h_ave_even = new TH1F("h_ave_even", "h_ave_even", 64, 0, 128);
    TH1F *h_ave_odd = new TH1F("h_ave_odd", "h_ave_odd", 64, 1, 129);
    TH2F *h2hits_amp =
        new TH2F("h2hits_amp", "h2hits_amp", 128, 0, 128, 31, 1, 31);
    TH1F *h1_ave_residuals_even = new TH1F(
        "h1_ave_residuals_even", "h1_ave_residuals_even", 100, -100000, 100000);
    TH1F *h1_ave_residuals_odd = new TH1F(
        "h1_ave_residuals_odd", "h1_ave_residuals_odd", 100, -100000, 100000);
    // histograms storing threshold values for each channel, even and odd
    // separately
    TH1F *h1_thr_even = new TH1F("h1_thr_even", "h1_thr_even", 128, 0, 128);
    TH1F *h1_thr_odd = new TH1F("h1_thr_odd", "h1_thr_odd", 128, 0, 128);

    // histograms storing median values based on the neighboring channels
    TH1F *h1_med_even = new TH1F("h1_med_even", "h1_med_even", 128, 0, 128);
    TH1F *h1_med_odd = new TH1F("h1_med_odd", "h1_med_odd", 128, 0, 128);

    // ---------------------------------------------- Histograms name
    // ----------------------------------------------------
    h_ave_hits->SetTitle("Noise counts vs channel; Channel number; Counts");
    h_broken_ch->SetTitle("Noise counts vs channel; Channel number; Counts");
    h1_ave_even->SetTitle("Noise counts; Number of noise hits; Entries");
    h1_ave_odd->SetTitle("Noise counts; Number of noise hits; Entries");
    h2hits_amp->SetTitle(
        "Noise counts distribution; Channel number; ADC value");
    h1_thr_even->SetTitle("Threshold vs channel; Channel number; Threshold");
    h1_thr_odd->SetTitle("Threshold vs channel; Channel number; Threshold");
    h1_med_even->SetTitle(
        "Median of neighbors vs channel; Channel number; Median of neighbors");
    h1_med_odd->SetTitle(
        "Median of neighbors vs channel; Channel number; Median of neighbors");

    int ch_hits[128] = {0};
    float ave_even = 0.;
    float ave_odd = 0.;

    // Initial iteration to calculate averages. Odd and Even channels separated
    for (int ch = 0; ch < 128; ch++) {
      for (int d = 1; d < 32; d++) {
        h2hits_amp->Fill(ch, 31 - d, vcnt[ch][d]);
        ch_hits[ch] += vcnt[ch][d];
      }
      h_ave_hits->Fill(ch, ch_hits[ch]);
      if (ch % 2 == 0) {
        h1_ave_even->Fill(ch_hits[ch]);
        h_ave_even->Fill(ch, ch_hits[ch]);
      } else {
        h1_ave_odd->Fill(ch_hits[ch]);
        h_ave_odd->Fill(ch, ch_hits[ch]);
      }
    }

    // Fitting the histograms to determine mean value and sigma
    h1_ave_even->Fit("gaus");
    float first_ave_even = h1_ave_even->GetFunction("gaus")->GetParameter(1);
    float first_sigma_even = h1_ave_even->GetFunction("gaus")->GetParameter(2);
    h1_ave_odd->Fit("gaus");
    float first_ave_odd = h1_ave_odd->GetFunction("gaus")->GetParameter(1);
    float first_sigma_odd = h1_ave_odd->GetFunction("gaus")->GetParameter(2);

    // Second iteration to discard broken or noisy channels
    float z_alpha = 3.5;
    int ch_cnt_even = 0;
    int ch_cnt_odd = 0;
    for (int ch = 0; ch < 128; ch++) {
      if (ch % 2 == 0 &&
          abs(ch_hits[ch] - first_ave_even) < z_alpha * first_sigma_even) {
        ave_even += ch_hits[ch];
        ch_cnt_even++;
      }
      if (ch % 2 != 0 &&
          abs(ch_hits[ch] - first_ave_odd) < z_alpha * first_sigma_odd) {
        ave_odd += ch_hits[ch];
        ch_cnt_odd++;
      }
    }
    // Determining the average values
    // if (ch_cnt_even == 0)
    //   ch_cnt_even = 1;
    // ave_even = ave_even / ch_cnt_even;
    // if (ch_cnt_odd == 0)
    //   ch_cnt_odd = 1;
    // ave_odd = ave_odd / ch_cnt_odd;

    // Last iteration to discriminate broken channels
    z_alpha = 0.55;
    // number of channels in the neighborhood for dynamic threshold
    int n_neigh_ch = 10;
    // averages of neigh_even, neigh_odd vectors (averaged over neighboring
    // channels)
    float neigh_med_even, neigh_med_odd;
    // dynamic thresholds for even and odd channels based on the neighborhood
    float thr_even, thr_odd;

    for (int ch = 0; ch < 128; ch++) {
      // loop over all the channels
      neigh_even.clear();
      // erase the neighbors of the previous channel
      neigh_odd.clear();
      // the channel may not have enough neighbors to the left or right
      int start_ch = std::max(0, ch - n_neigh_ch);
      int end_ch = std::min(128, ch + n_neigh_ch + 1);

      if (ch % 2 == 0) {
        // if the channel is even

        for (int i = start_ch; i < end_ch; i++) {
          // Loop through neighboring channels to the left and right of the
          // current channel
          if (i != ch) {

            neigh_even.push_back(ch_hits[i]);
          }
        }

        // calculate the median of neighboring channels
        neigh_med_even = calculate_median(neigh_even, neigh_even.size());
        h1_med_even->Fill(ch, neigh_med_even);
        thr_even = z_alpha * neigh_med_even;
        h1_thr_even->Fill(ch, thr_even);

        if (ch_hits[ch] < thr_even) {
          // if the channel is broken based on the dynamic threshold

          broken_channels.push_back(ch);
        }

      }

      else if (ch % 2 != 0) {
        // if the channel is odd

        for (int i = start_ch; i < end_ch; i++) {
          // Loop through neighboring channels to the left and right of the
          // current channel
          if (i != ch) {

            neigh_odd.push_back(ch_hits[i]);
          }
        }

        // calculate the median of neighboring channels
        neigh_med_odd = calculate_median(neigh_odd, neigh_odd.size());
        h1_med_odd->Fill(ch, neigh_med_odd);
        thr_odd = z_alpha * neigh_med_odd;
        h1_thr_odd->Fill(ch, thr_odd);

        if (ch_hits[ch] < thr_odd) {
          // if the channel is broken based on the dynamic threshold

          broken_channels.push_back(ch);
        }

      } else {
        continue;
      }
    }

    // Fitting a 9th-order polynomial to the data
    // h_ave_hits->Fit("pol9", "W");
    int pol_grade = 9;
    TString pol_fit = "pol" + std::to_string(pol_grade);
    h_ave_odd->Fit(pol_fit, "W");
    h_ave_even->Fit(pol_fit, "W");
    float pol_fit_even[10] = {0};
    float pol_fit_odd[10] = {0};
    for (int i = 0; i < 10; i++) {
      pol_fit_even[i] = h_ave_even->GetFunction(pol_fit)->GetParameter(i);
      pol_fit_odd[i] = h_ave_odd->GetFunction(pol_fit)->GetParameter(i);
      // cout <<pol_fit[i]<<endl;
    }
    float residuals = 0;
    for (int ch = 0; ch < 128; ch++) {
      residuals = 0;
      if (ch % 2 == 0) {
        for (int n = 0; n < pol_grade + 1; n++) {
          residuals += pol_fit_even[n] * pow(ch, n);
        }
        h1_ave_residuals_even->Fill(h_ave_hits->GetBinContent(ch + 1) -
                                    residuals);
      } else {
        for (int n = 0; n < pol_grade + 1; n++) {
          residuals += pol_fit_odd[n] * pow(ch, n);
        }
        h1_ave_residuals_odd->Fill(h_ave_hits->GetBinContent(ch + 1) -
                                   residuals);
      }
    }
    // Fitting residuals
    h1_ave_residuals_even->Fit("gaus");
    h1_ave_residuals_odd->Fit("gaus");
    float par_residuals_even[2] = {0.};
    float par_residuals_odd[2] = {0.};

    par_residuals_even[0] =
        h1_ave_residuals_even->GetFunction("gaus")->GetParameter(1);
    par_residuals_even[1] =
        h1_ave_residuals_even->GetFunction("gaus")->GetParameter(2);
    par_residuals_odd[0] =
        h1_ave_residuals_odd->GetFunction("gaus")->GetParameter(1);
    par_residuals_odd[1] =
        h1_ave_residuals_odd->GetFunction("gaus")->GetParameter(2);

    // cout <<"Fitting residuals parameters:
    // "<<mean_residuals<<"\t"<<sigma_residuals<<endl;

    z_alpha = 3.5;
    for (int ch = 0; ch < 128; ch++) {
      residuals = 0;
      if (ch % 2 == 0) {
        for (int n = 0; n < pol_grade + 1; n++) {
          residuals += pol_fit_even[n] * pow(ch, n);
        }
        if (par_residuals_even[0] -
                (h_ave_hits->GetBinContent(ch + 1) - residuals) >
            z_alpha * par_residuals_even[1]) {
          broken_channelsv2.push_back(ch);
          cout << "Broken channels residuals: " << ch << endl;
        }
      } else {
        for (int n = 0; n < pol_grade + 1; n++) {
          residuals += pol_fit_odd[n] * pow(ch, n);
        }
        if (par_residuals_odd[0] -
                (h_ave_hits->GetBinContent(ch + 1) - residuals) >
            z_alpha * par_residuals_odd[1]) {
          broken_channelsv2.push_back(ch);
          cout << "Broken channels residuals: " << ch << endl;
        }
      }
    }

    // Writing the histogram of the broken channels. It could be done above!!!
    // (for Dario)
    if (broken_channels.size() != 0) {
      for (int j = 0; j < broken_channels.size(); j++) {
        h_broken_ch->Fill(broken_channels[j], ch_hits[broken_channels[j]]);
      }
    }
    if (broken_channelsv2.size() != 0) {
      for (int j = 0; j < broken_channelsv2.size(); j++) {
        h_broken_ch_residuals->Fill(broken_channelsv2[j],
                                    ch_hits[broken_channelsv2[j]]);
      }
    }

    // TF1 *f_ave_even = new TF1("f_ave_even", "[0]", 0, 128);
    // f_ave_even->SetParameter(0, neigh_med_even);
    // TF1 *f_thr_even = new TF1("f_thr_even", "[0]", 0, 128);
    // f_thr_even->SetParameter(0, thr_even);

    // TF1 *f_ave_odd = new TF1("f_ave_odd", "[0]", 0, 128);
    // f_ave_odd->SetParameter(0, neigh_med_odd);
    // TF1 *f_thr_odd = new TF1("f_thr_odd", "[0]", 0, 128);
    // f_thr_odd->SetParameter(0, thr_odd);

    TCanvas *c1 = new TCanvas("c1", "c1", 900, 600);

    c1->SetGrid();
    h_ave_hits->Draw("HIST");
    // h_ave_hits->Fit("pol9", "W", "same");
    // h_ave_hits->GetFunction("pol9")->SetLineColor(kOrange);
    h_ave_hits->SetLineColor(kAzure - 3);
    h_ave_hits->GetYaxis()->SetRangeUser(
        0, 1.5 * h_ave_hits->GetMaximum()); // 1.5 * ave_even
    // Adding broken channels
    h_broken_ch->Draw("PHISTSAME");
    h_broken_ch->SetMarkerSize(1.5);
    h_broken_ch->SetMarkerStyle(23);
    h_broken_ch->SetMarkerColor(kBlack);

    // h_broken_ch_residuals->Draw("PHISTSAME");
    // h_broken_ch_residuals->SetMarkerSize(2.5);
    // h_broken_ch_residuals->SetMarkerStyle(32);
    // h_broken_ch_residuals->SetMarkerColor(kBlack);

    /* f_ave_even->Draw("same");
     f_ave_even->SetLineWidth(2);
     f_ave_even->SetLineStyle(7);
     f_ave_even->SetLineColor(kGreen + 2);
     f_thr_even->Draw("same");
     f_thr_even->SetLineWidth(2);
     f_thr_even->SetLineStyle(1);
     f_thr_even->SetLineColor(kGreen + 2); */

    h1_med_even->Draw("][HIST PMC SAME");
    h1_med_even->SetLineStyle(7);
    h1_med_even->SetLineColor(kGreen + 2);

    h1_thr_even->Draw("][HIST PMC SAME");
    h1_thr_even->SetLineStyle(1);
    h1_thr_even->SetLineColor(kGreen + 2);

    h1_med_odd->Draw("][HIST PMC SAME");
    h1_med_odd->SetLineStyle(7);
    h1_med_odd->SetLineColor(kMagenta);

    h1_thr_odd->Draw("][HIST PMC SAME");
    h1_thr_odd->SetLineStyle(1);
    h1_thr_odd->SetLineColor(kMagenta + 2);

    /*    f_ave_odd->Draw("same");
        f_ave_odd->SetLineWidth(2);
        f_ave_odd->SetLineStyle(7);
        f_ave_odd->SetLineColor(kMagenta);
        f_thr_odd->Draw("same");
        f_thr_odd->SetLineWidth(2);
        f_thr_odd->SetLineStyle(1);
        f_thr_odd->SetLineColor(kMagenta + 2);*/

    TLegend *lg_brk_summ = new TLegend(0.60, 0.60, 0.90, 0.90, "", "nbNDC");
    lg_brk_summ->SetHeader("Connectivity check");
    lg_brk_summ->AddEntry(h_ave_hits, "Noise hits", "l");
    lg_brk_summ->AddEntry(h_broken_ch, "broken channels", "p");
    // lg_brk_summ->AddEntry(h_broken_ch_residuals,"broken ch v2","p");
    // lg_brk_summ->AddEntry(f_ave_even, "Average counts even channels", "l");
    lg_brk_summ->AddEntry(h1_med_even, "Median of neighboring even channels",
                          "l");

    // lg_brk_summ->AddEntry(f_ave_odd, "Average counts odd channels", "l");
    lg_brk_summ->AddEntry(h1_med_odd, "Median of neighboring odd channels",
                          "l");

    // lg_brk_summ->AddEntry(f_thr_even, "Thr broken even channels", "l");
    lg_brk_summ->AddEntry(h1_thr_even, "Threshold broken even channels", "l");

    // lg_brk_summ->AddEntry(f_thr_odd, "Thr broken odd channels", "l");
    lg_brk_summ->AddEntry(h1_thr_odd, "Threshold broken odd channels", "l");

    // lg_brk_summ->AddEntry(h_ave_hits->GetFunction("pol9"),"Polynomial
    // fitting","l");
    lg_brk_summ->SetBorderSize(0);
    lg_brk_summ->SetFillColor(0);
    lg_brk_summ->Draw("");
    c1->Write();
    c1->Close();

    // h2hits_amp->Draw("COLZ");
    // h_ave_hits->Draw("HIST");
    //  Writing histograms to file
    gStyle->SetOptStat();
    h2hits_amp->Write();
    h_ave_hits->Write();
    h1_ave_even->Write();
    h1_ave_odd->Write();
    h_broken_ch->Write();
    h1_ave_residuals_even->Write();
    h1_ave_residuals_odd->Write();
    h_ave_even->Write();
    h_ave_odd->Write();
    h_broken_ch_residuals->Write();
    // add median histograms and threshold histograms
    h1_med_even->Write();
    h1_med_odd->Write();
    h1_thr_even->Write();
    h1_thr_odd->Write();

    file1->Close();

    // Writing summary file
    write_results(filename_root, broken_channels);

  } /*
  // Loop to print summary results
  cout<< "---------------- SUMMARY of BROKEN CHANNELS:
  -----------------"<<endl; for (int i = 0; i<int(file_names.size()); i++){
    cout<< Get_file_name(i).c_str() << ": " <<broken_channels.size() <<endl;
  }*/
  return 0;
}
