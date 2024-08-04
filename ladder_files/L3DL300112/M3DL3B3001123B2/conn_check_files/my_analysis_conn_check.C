#include "TObject.h"
#include "TString.h"
#include "string"
#include <RtypesCore.h>
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
#include <TPaveStats.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

TString filename_data;
std::vector<string> file_names;
int vcnt[128][32];
// averages channel in the neighborhood for dynamic threshold
std::vector<float> neigh_even;
std::vector<float> neigh_odd;
// weighted average based on Y projection of problematic or suspicious channels
// based on the vicinity of a dynamic threshold
std::vector<Double_t> sus_even;
std::vector<Double_t> sus_odd;

void Analysis() {
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

  // std::cout << "------------- Reading data file: -----------" << std::endl;
  int count = 0;
  int ch_cnt = 0;
  while (!scanfile.eof()) {
    std::getline(scanfile, line);
    std::istringstream iss(line);
    if (!line.empty()) {
      iss >> ele;
      iss >> ele;
      // cout << " ch " << ch_cnt << " ";
      count = 0;
      for (int d = d_min; d < d_max + 1; d++) {
        iss >> count;
        vcnt[ch_cnt][d] = count;
        // printf("%4d ", vcnt[ch_cnt][d]);
      }
      // cout << endl;
      ch_cnt++;
    }
  }

  scanfile.close();
}

bool Read_file_tests() {
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
  // cout <<
  // "......................oooo00000oooo..............................."
  //      << endl;
  while (!myfile.eof()) {
    std::getline(myfile, line);
    if (line.empty())
      continue;
    else {
      while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        // cout << token << endl;
        line.erase(0, pos + delimiter.length());
      }
      file_names.push_back(token);
    }
  }
  // cout << file_names.size() << endl;
  if (file_names.size() != 0)
    return false;
  return true;
}

// get_file_name from the file list
std::string get_file_name(int i) { return file_names[i]; }

// write the summary of broken channels for each file
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

/**
 *  Check if a file exists
 *  filename - the name of the file to check
 * returns  true if the file exists, else false
 */
bool file_exists(const std::string &filename) {
  struct stat buf;
  if (stat(filename.c_str(), &buf) != -1) {
    return true;
  }
  return false;
}
// write the parameters used to find the total number of broken channels
void write_parameters(float z_alpha, int n_neigh_ch, float sus_perc,
                      float n_sigmas, size_t n_broken_channels) {
  /* z_alpha - percentage of neighbors' median for the threshold
   n_neigh_ch - number of channels in the neighborhood for dynamic threshold
   sus_perc - percentage of threshold vicinity for determining suspicious
   channels n_sigmas - accepted deviation from the median of weighted average of
   suspicious channels' neighbors */
  ofstream my_file;
  if (!file_exists("conn_check_parameters.txt")) {
    my_file.open("conn_check_parameters.txt", std::ios::out);

    my_file << fixed << setw(20) << "z_alpha" << '\t' << fixed << setw(20)
            << "n_neigh_ch" << '\t' << fixed << setw(20) << "sus_perc" << '\t'
            << fixed << setw(20) << "n_sigmas" << '\t' << fixed << setw(20)
            << "n_broken_channels" << '\n';

    my_file << fixed << setw(20) << z_alpha << '\t' << fixed << setw(20)
            << n_neigh_ch << '\t' << fixed << setw(20) << sus_perc << '\t'
            << fixed << setw(20) << n_sigmas << '\t' << fixed << setw(20)
            << n_broken_channels << '\n';
  } else {
    my_file.open("conn_check_parameters.txt", std::ios::app);

    my_file << fixed << setw(20) << z_alpha << '\t' << fixed << setw(20)
            << n_neigh_ch << '\t' << fixed << setw(20) << sus_perc << '\t'
            << fixed << setw(20) << n_sigmas << '\t' << fixed << setw(20)
            << n_broken_channels << '\n';
  }

  my_file.close();
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

Double_t calculate_weighted_av_ch(int ch, TH2F *h2) {
  // histogram for keeping Y projection of h2hits_amp for this channel
  // to transition from bins to channels, we always need to add 1
  // since bins start from 1 but channels start from 0 we shift
  TH1D *py_h2hits_amp = h2->ProjectionY("py_h2hits_amp", ch + 1, ch + 1, "");
  // get the total sum of weights ing the Y projection
  Double_t sum_of_weights_py_h2hits = py_h2hits_amp->Integral();
  // initialize the weighted average sum of amplitude for this channel
  Double_t sum_average_amp = 0.0;

  for (Int_t n_bin = 1; n_bin < py_h2hits_amp->GetNbinsX(); n_bin++) {
    Double_t weight_py_h2hits = py_h2hits_amp->GetBinContent(n_bin);
    Double_t adc_value = py_h2hits_amp->GetXaxis()->GetBinLowEdge(n_bin);
    sum_average_amp += weight_py_h2hits * adc_value;
  }
  delete py_h2hits_amp;
  if (sum_of_weights_py_h2hits != 0) {
    Double_t weighted_av_ch = sum_average_amp / sum_of_weights_py_h2hits;
    return weighted_av_ch;
  } else {
    return sum_average_amp;
  }
  // Double_t weighted_av_ch = sum_average_amp / sum_of_weights_py_h2hits;
  // return weighted_av_ch;
}

// Write a vector of channels to the histogram
void fill_hist_with_ch(TH1F *hist, std::vector<int> channels, int *ch_hits) {
  if (channels.size() != 0) {
    for (int j = 0; j < channels.size(); j++) {
      hist->Fill(channels[j], ch_hits[channels[j]]);
    }
  }
}

// ------------------------ Main function ---------------------
int my_analysis_conn_check() {

  for (int ch = 0; ch < 128; ch++) {
    for (int d = 0; d < 32; d++) {
      vcnt[ch][31 - d] = 0;
    }
  }

  TString dir = "";
  TString filename_root = "";
  std::vector<int> broken_channels;
  std::vector<int> suspicious_channels;
  std::vector<int> noisy_channels;

  // counter of broken channels accross all the files
  size_t n_broken_channels = 0;
  // percentage of neighbors' median for the threshold
  float z_alpha = 0.65;
  // number of channels in the neighborhood for dynamic threshold
  int n_neigh_ch = 36;
  // percentage of threshold vicinity for determining suspicious channels
  float sus_perc = 0.30;
  // accepted deviation from the median of weighted average of neighbors for
  // suspicious channels
  float n_sigmas = 3.5;

  // reading file with list of measurements files.
  Read_file_tests();
  // Analysis function is called for every file in the list. At the end,
  // everything is closed and root files can be accessed via TBrowser
  for (int i = 0; i < int(file_names.size()); i++) {
    filename_data = dir + get_file_name(i).c_str();
    filename_root = dir + get_file_name(i).c_str();
    // cout << "Filename_selected: " << filename_data << endl;
    Analysis();
    // erase the previous channels
    broken_channels.clear();
    suspicious_channels.clear();
    noisy_channels.clear();

    // Creating files
    TFile *file1 = new TFile(filename_root + ".root", "recreate");
    file1->cd();
    // Creating histograms
    TH1F *h_ave_hits = new TH1F("h_hits", "h_hits", 128, 0, 128);
    TH1F *h_broken_ch = new TH1F("h_broken_ch", "h_broken_ch", 128, 0, 128);

    TH1F *h1_ave_even = new TH1F("h1_ave_even", "h1_ave_even", 100, 0, 400000);
    TH1F *h1_ave_odd = new TH1F("h1_ave_odd", "h1_ave_odd", 100, 0,
                                400000); // access these channels
    TH1F *h_ave_even = new TH1F("h_ave_even", "h_ave_even", 64, 0, 128);
    TH1F *h_ave_odd = new TH1F("h_ave_odd", "h_ave_odd", 64, 1, 129);
    TH2F *h2hits_amp =
        new TH2F("h2hits_amp", "h2hits_amp", 128, 0, 128, 31, 1, 31);

    // histograms storing thresholds for each channel, even and odd separately
    TH1F *h_thr_even = new TH1F("h_thr_even", "h_thr_even", 128, 0, 128);
    TH1F *h_thr_odd = new TH1F("h_thr_odd", "h_thr_odd", 128, 0, 128);

    // histograms storing median values based on the neighboring channels
    TH1F *h_med_even = new TH1F("h_med_even", "h_med_even", 128, 0, 128);
    TH1F *h_med_odd = new TH1F("h_med_odd", "h_med_odd", 128, 0, 128);

    // histogram storing suspicious channels
    TH1F *h_sus_ch = new TH1F("h_sus_ch", "h_sus_ch", 128, 0, 128);

    // histogram storing extra noisy channels
    TH1F *h_noisy_ch = new TH1F("h_noisy_ch", "h_noisy_ch", 128, 0, 128);

    // histogram for weighted average ADC of each channel
    TH1D *h_weighted_av =
        new TH1D("h_weighted_av", "h_weighted_av", 128, 0, 128);

    // histogram for storing median of weighed average ADC of neighbors
    TH1D *h_med_weighted_av =
        new TH1D("h_med_weighted_av", "h_med_weighted_av", 128, 0, 128);

    // histogram to get std dev of weighted average
    TH1D *h1_weighted_av_std =
        new TH1D("h1_weighted_av_std", "h1_weighted_av_std", 100, 0, 15);

    // ---------------------------------------------- Histograms name
    // ----------------------------------------------------
    h_ave_hits->SetTitle("Noise counts vs channel; Channel number; Counts");
    h_broken_ch->SetTitle("Noise counts vs channel; Channel number; Counts");
    h1_ave_even->SetTitle("Noise counts; Number of noise hits; Entries");
    h1_ave_odd->SetTitle("Noise counts; Number of noise hits; Entries");
    h2hits_amp->SetTitle(
        "Noise counts distribution; Channel number; ADC value");
    h_thr_even->SetTitle("Threshold vs channel; Channel number; Threshold");
    h_thr_odd->SetTitle("Threshold vs channel; Channel number; Threshold");
    h_med_even->SetTitle(
        "Median of neighbors vs channel; Channel number; Median of neighbors");
    h_med_odd->SetTitle(
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

    // fill the weighted average hist with the value for current channel
    for (int ch = 0; ch < 128; ch++) {
      if (!std::isnan(calculate_weighted_av_ch(ch, h2hits_amp))) {
        h_weighted_av->Fill(ch, calculate_weighted_av_ch(ch, h2hits_amp));
        h1_weighted_av_std->Fill(calculate_weighted_av_ch(ch, h2hits_amp));
      }
    }
    // stddev of weighted average
    Double_t sus_neigh_sigma = h1_weighted_av_std->GetStdDev();
    // medians of neigh_even, neigh_odd vectors
    float neigh_med_even, neigh_med_odd;
    // dynamic thresholds for even and odd channels based on the neighborhood
    float thr_even, thr_odd;

    // Second iteration to discriminate broken channels
    for (int ch = 0; ch < 128; ch++) {

      // erase the neighbors of the previous channel
      neigh_odd.clear();
      neigh_even.clear();

      // erase the weighted sums of surroundings of the previous channel
      sus_even.clear();
      sus_odd.clear();

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
            // add weighted sum of ADC in neighboring channels
            sus_even.push_back(calculate_weighted_av_ch(i, h2hits_amp));
          }
        }

        // calculate the median of neighboring channels
        neigh_med_even = calculate_median(neigh_even, neigh_even.size());
        h_med_even->Fill(ch, neigh_med_even);
        thr_even = z_alpha * neigh_med_even;
        h_thr_even->Fill(ch, thr_even);

        // calculate the median of weighted sum in neighboring channels
        h_med_weighted_av->Fill(ch,
                                calculate_median(sus_even, sus_even.size()));

        if (ch_hits[ch] > thr_even * n_sigmas) {
          // if the channel is extra noisy it's also masked
          noisy_channels.push_back(ch);
        }

        if (ch_hits[ch] < thr_even) {
          // if the channel is broken based on the dynamic threshold

          broken_channels.push_back(ch);
          //______________________
          if (h_ave_hits->GetBinContent(ch + 1) == 0) {
            // No analog response (NAR): broken channel
            std::cout << "broken dynamic threshold NAR channel " << ch << '\n';
          } else {
            std::cout << "broken dynamic threshold channel " << ch << '\n';
          }
          //______________________
          //___Last criterium to find broken channels___
        } 
        // else if (std::abs(ch_hits[ch] - thr_even) < thr_even * sus_perc) {

        //   // if the channel is suspicious based on the vicinity of threshold
        //   // criteria separate for each channel
        //   // remembering about the ch + 1 to bin shift
        //   if (std::abs(h_weighted_av->GetBinContent(ch + 1) -
        //                h_med_weighted_av->GetBinContent(ch + 1)) >
        //       n_sigmas * sus_neigh_sigma) {
        //     // if the median of ADC weighted sum in neighboring channels is
        //     // significantly different than for the suspicious channel
        //     suspicious_channels.push_back(ch);
        //     broken_channels.push_back(ch);

        //     if (h_ave_hits->GetBinContent(ch + 1) == 0) {
        //       //          // No analog response (NAR): broken channel
        //       std::cout << "broken sus NAR channel " << ch << '\n';
        //     } else {
        //       std::cout << "broken sus channel " << ch << '\n';
        //     }
        //   } else {
        //     suspicious_channels.push_back(ch);
        //   }
        // }

      }

      else if (ch % 2 != 0) {
        // if the channel is odd

        for (int i = start_ch; i < end_ch; i++) {
          // Loop through neighboring channels to the left and right of the
          // current channel
          if (i != ch) {

            neigh_odd.push_back(ch_hits[i]);
            sus_odd.push_back(calculate_weighted_av_ch(i, h2hits_amp));
          }
        }

        // calculate the median of neighboring channels
        neigh_med_odd = calculate_median(neigh_odd, neigh_odd.size());
        h_med_odd->Fill(ch, neigh_med_odd);
        thr_odd = z_alpha * neigh_med_odd;
        h_thr_odd->Fill(ch, thr_odd);

        // calculate the median of weighted sum in neighboring channels
        h_med_weighted_av->Fill(ch, calculate_median(sus_odd, sus_odd.size()));

        if (ch_hits[ch] > thr_odd * n_sigmas) {
          // if the channel is extra noisy it's also masked
          noisy_channels.push_back(ch);
        }

        if (ch_hits[ch] < thr_odd) {
          // if the channel is broken based on the dynamic threshold

          broken_channels.push_back(ch);
          if (h_ave_hits->GetBinContent(ch + 1) == 0) {
            // No analog response (NAR): broken channel
            std::cout << "broken dynamic threshold NAR channel " << ch << '\n';
          } else {
            std::cout << "broken dynamic threshold channel " << ch << '\n';
          }

        } 
        // else if (std::abs(ch_hits[ch] - thr_odd) < thr_odd * sus_perc) {

        //   // if the channel is suspicious based on the vicinity of threshold
        //   // criteria separate for each channel
        //   //___Last criterium to find broken channels___
        //   // remembering about the ch + 1 to bin shift
        //   if (std::abs(h_weighted_av->GetBinContent(ch + 1) -
        //                h_med_weighted_av->GetBinContent(ch + 1)) >
        //       n_sigmas * sus_neigh_sigma) {
        //     // if the median of ADC weighted sum in neighboring channels if
        //     // significantly different
        //     suspicious_channels.push_back(ch);
        //     broken_channels.push_back(ch);
        //     if (h_ave_hits->GetBinContent(ch + 1) == 0) {
        //      // No analog response (NAR): broken channel
        //       std::cout << "broken sus NAR channel " << ch << '\n';
        //     } else {
        //       std::cout << "broken sus channel " << ch << '\n';
        //     }
        //   } else {
        //     suspicious_channels.push_back(ch);
        //   }
        // }
      }
    }

    // Writing the histogram of the broken, suspicious and noisy channels
    fill_hist_with_ch(h_broken_ch, broken_channels, ch_hits);
    fill_hist_with_ch(h_sus_ch, suspicious_channels, ch_hits);
    fill_hist_with_ch(h_noisy_ch, noisy_channels, ch_hits);

    TCanvas *c1 = new TCanvas("c1", "c1", 900, 600);

    c1->SetGrid();
    h_ave_hits->Draw("HIST");
    h_ave_hits->SetStats(0);
    h_ave_hits->GetYaxis()->SetTitleOffset(1.2);
    h_ave_hits->GetYaxis()->SetNdivisions(505, 4);
    h_ave_hits->SetLineColor(kAzure - 3);
    h_ave_hits->GetYaxis()->SetRangeUser(0, 1.5 * h_ave_hits->GetMaximum());

    // Adding broken channels
    h_broken_ch->Draw("PHISTSAME");
    h_broken_ch->SetMarkerSize(1.5);
    h_broken_ch->SetMarkerStyle(23);
    h_broken_ch->SetMarkerColor(kBlack);

    // Adding suspicious channels
    h_sus_ch->Draw("PHISTSAME");
    h_sus_ch->SetMarkerSize(1.5);
    h_sus_ch->SetMarkerStyle(22);
    h_sus_ch->SetMarkerColor(kAzure + 2);

    // Adding extra noisy channels
    h_noisy_ch->Draw("PHISTSAME");
    h_noisy_ch->SetMarkerSize(2);
    h_noisy_ch->SetMarkerStyle(33);
    h_noisy_ch->SetMarkerColor(kOrange + 1);

    h_med_even->Draw("PHISTSAME");
    h_med_even->SetLineWidth(0);
    h_med_even->SetMarkerStyle(kFullCircle);
    h_med_even->SetMarkerColor(kGreen + 2);

    h_thr_even->Draw("PHIST SAME");
    h_thr_even->SetLineWidth(0);
    h_thr_even->SetMarkerStyle(kFullDotLarge);
    h_thr_even->SetMarkerColor(kGreen + 2);

    h_med_odd->Draw("PHIST SAME");
    h_med_odd->SetLineWidth(0);
    h_med_odd->SetMarkerStyle(kFullCircle);
    h_med_odd->SetMarkerColor(kMagenta);

    h_thr_odd->Draw("PHIST   SAME");
    h_thr_odd->SetLineWidth(0);
    h_thr_odd->SetMarkerStyle(kFullDotLarge);
    h_thr_odd->SetMarkerColor(kMagenta + 2);

    TLegend *lg_brk_summ = new TLegend(0.65, 0.65, 0.90, 0.895, "", "nbNDC");
    lg_brk_summ->SetHeader("Connectivity check");
    lg_brk_summ->AddEntry(h_ave_hits, "Noise hits", "l");
    lg_brk_summ->AddEntry(h_broken_ch, "Broken channels", "p");
    lg_brk_summ->AddEntry(h_sus_ch, "Suspicious channels", "p");
    if (noisy_channels.size() != 0)
      lg_brk_summ->AddEntry(h_noisy_ch, "Noisy channels", "p");
    lg_brk_summ->AddEntry(h_med_even, "Median of neighboring even channels",
                          "p");
    lg_brk_summ->AddEntry(h_med_odd, "Median of neighboring odd channels", "p");
    lg_brk_summ->AddEntry(h_thr_even, "Threshold broken even channels", "p");
    lg_brk_summ->AddEntry(h_thr_odd, "Threshold broken odd channels", "p");

    lg_brk_summ->SetBorderSize(0);
    lg_brk_summ->SetFillColor(0);
    lg_brk_summ->Draw("");
    c1->Write();
    c1->Close();

    //  Writing histograms to file
    gStyle->SetOptStat();
    h2hits_amp->Write();
    h_ave_hits->Write();
    h1_ave_even->Write();
    h1_ave_odd->Write();
    h_broken_ch->Write();

    h_ave_even->Write();
    h_ave_odd->Write();
    // add median histograms and threshold histograms
    h_med_even->Write();
    h_med_odd->Write();
    h_thr_even->Write();
    h_thr_odd->Write();
    // add weighted average, its stddev and median
    h_weighted_av->Write();
    h_med_weighted_av->Write();
    h1_weighted_av_std->Write();
    // close the opened root file
    file1->Close();
    // Writing summary file
    write_results(filename_root, broken_channels);
    n_broken_channels += broken_channels.size();
  }
  write_parameters(z_alpha, n_neigh_ch, sus_perc, n_sigmas, n_broken_channels);
  return 0;
}
