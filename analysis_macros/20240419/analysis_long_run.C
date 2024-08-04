#include "TString.h"
#include <sstream>
#include "string"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cmath>
#include <stdint.h>
#include <unistd.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TFitResult.h>
#include <TFile.h>
#include <TProfile.h>
#include <TTree.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TPad.h>
#include <TF1.h>
#include <TBrowser.h>
#include "TObject.h"
#include <TLatex.h>
#include <vector>
#include <stdlib.h> 
#include <cstdio>
#include <ctime>

TString filename_data;
std::vector<string> file_names;
std::vector<float> enc_values;

float enc[100][128];
//float ave_enc_odd[50];
//float ave_enc_even[50];
float encf[100][128];
float thr[100][128];
float thrf[100][128];
float gain[100][128];

void clean_arrays(){
  for (int j = 0; j <100; j++){
    for (int ch = 0; ch<128; ch++){
      enc[j][ch] = 0;
      encf[j][ch] = 0;
      thr[j][ch] =  0;
      thrf[j][ch] = 0;
      gain[j][ch] = 0;
    }
  }
  
}

void Read_data(int nfile, bool fast_flag_user = false){

  filename_data = (filename_data+".root");
  TFile *file_open = new TFile(filename_data);
  
  cout<<"FILENAME: "<<filename_data <<endl;
  TH1F *henc = (TH1F*)file_open->Get("h_enc");
  //TH1F *henc = (TH1F*)file_open->Get("hfit_s_erfc");
  TH1F *hthr = (TH1F*)file_open->Get("h_adc_thr");
  TH1F *hgain  = (TH1F*)file_open->Get("h_adc_gain");

  TH1F *hmean_fast = (TH1F*)file_open->Get("h_fast_thr");
  TH1F *hnoise_fast= (TH1F*)file_open->Get("h_enc_fast");
  
  for (int ch = 0; ch <128; ch++){
    enc[nfile][ch]   = henc->GetBinContent(ch+1);
    thr[nfile][ch]   = hthr->GetBinContent(ch+1);
    gain[nfile][ch]  = hgain->GetBinContent(ch+1);
    if (fast_flag_user == true){
      encf[nfile][ch] = hnoise_fast->GetBinContent(ch+1);
      thrf[nfile][ch] = hmean_fast->GetBinContent(ch+1);
    }
  }  
  cout<<"Data file: "<<nfile<<endl;
 file_open->Close();
}


bool Read_file_tests(TString file_list){
  
  ifstream myfile;
  myfile.open(file_list+ ".txt");
  std::string line;
  std::string delimiter = ".root";
  size_t pos = 0;
  std::string token;

 // reading list file and storing it in a vector
  cout<<"......................oooo00000oooo..............................."<<endl;
  while(!myfile.eof()){
    std::getline(myfile, line);
    if (line.empty()) continue;
     else {
    while ((pos = line.find(delimiter)) != std::string::npos) {
      token = line.substr(0, pos);
      cout << token << endl;
      line.erase(0, pos + delimiter.length());
      }  
      file_names.push_back(token);
     }
    }
  cout<<file_names.size()<<endl;  
  if (file_names.size()!=0) return false;
  return true; 
}

// ............... oooo00000oooo........................
//! Get_file_name from the file list
std::string Get_file_name(int i) {return file_names[i];}



// ------------------------------------------------ MAIN FUNCTION ---------------------------------------------------
void analysis_long_run(TString file_list, int asic_hw_id, TString polarity){
// Three arguments: 
// file_list  --> Name of the file where the pscan to be analyzed are listed
// asic_hw_id --> ASIC hardware ID that is analyzed
// polarity   --> Polarity of the data that is analized

  //TString module_ID  = "M3DL3B1001120B2";
  // -------------------------------------- Enabling or disabling FAST disc reading  -----------------------------------
  bool fast_flag = true; 

  // ----------------------------------------- Creating file with histograms  ------------------------------------------
  TFile *f1;
  //TString filename_root = dir + "module_test_" + module_ID + ".root";
  TString filename_root = Form("long_run_asic_%d_" + polarity + ".root", asic_hw_id);
  cout <<filename_root<<endl;
  f1 = new TFile(filename_root, "recreate");
  TDirectory *cd_h_enc = f1->mkdir("h_enc");


  // ---------------------------------------- Reading data files ----------------------------------------------
  // ENC - ADC
  clean_arrays(); 
  Read_file_tests(file_list);
  //Read_file_tests();
  for (int i = 0; i<int(file_names.size()); i++){
    filename_data= Get_file_name(i);
    cout<<"Filename_selected: "<<filename_data<<endl;
    Read_data(i, fast_flag);
  }

  int nfiles = int(file_names.size());
  cout<<"Number of files: "<<nfiles<<endl;
  
  // ---------------------------------------- Histograms [Distributions] ----------------------------------------------
  float time = 0;
  float time_min = 0;
  float time_interval = 10.0;                                              // Time interval between measurements: 5 min
  float time_max = time_min + nfiles*time_interval; 
  // --- ENC - ADC ---
  TH1F *h1_enc_even[nfiles];
  TH1F *h1_enc_odd[nfiles];
  TH1F *h1_enc_adc[nfiles];
  TH1F *h1_enc_fast[nfiles];
  TH1F *h1_gain[nfiles];
  TH1F *h1_thr_adc[nfiles];
  TH1F *h1_thr_fast[nfiles];

  // --- ENC vs Time ---
  TH1F *h_time_enc_even = new TH1F("h_time_enc_even", "h_time_enc_even", nfiles, time_min, time_max);
  TH1F *h_time_enc_odd  = new TH1F("h_time_enc_odd",  "h_time_enc_odd",  nfiles, time_min, time_max);
  TH1F *h_time_enc_adc  = new TH1F("h_time_enc_adc",  "h_time_enc_adc",  nfiles, time_min, time_max);
  // --- Gain ADC ----
  TH1F *h_time_gain  = new TH1F("h_time_gain",  "h_time_gain",  nfiles, time_min, time_max);
  // --- THR ADC ----
  TH1F *h_time_thr_adc  = new TH1F("h_time_thr_adc",  "h_time_thr_adc",  nfiles, time_min, time_max);
  // --- ENC FAST vs Time ---
  TH1F *h_time_enc_fast = new TH1F("h_time_enc_fast", "h_time_enc_fast", nfiles, time_min, time_max);
  // --- THR FAST ----
  TH1F *h_time_thr_fast = new TH1F("h_time_thr_fast", "h_time_thr_fast", nfiles, time_min, time_max);

  // ------------ Histograms titles ------------
  // --- ENC ---
  h_time_enc_even->SetTitle("Long run stability; Time [min]; ENC [e]");
  h_time_enc_odd->SetTitle("Long run stability; Time [min]; ENC [e]");
  h_time_enc_adc->SetTitle("Long run stability; Time [min]; ENC [e]");
  h_time_enc_fast->SetTitle("Long run stability; Time [min]; FAST ENC [e]");
  h_time_gain->SetTitle("Long run stability; Time [min]; ADC gain [e/LSB]");
  h_time_thr_adc->SetTitle("Long run stability; Time [min]; ADC Thr [e]");
  h_time_thr_fast->SetTitle("Long run stability; Time [min]; FAST Thr [e]");

  cout << "Histograms creation"<<endl;
  // ------------------------------------- Filling, fitting and writing data histograms  -------------------------------------
  TF1 *f_gaus_fit = new TF1("f_gaus_fit", "gaus", 800, 5000);
  for (int j =0; j< nfiles; j++){

    // --- Histograms string names ---
    TString hname_enc_even(Form("h1_enc_even_%d",j)); 
    TString hname_enc_odd(Form( "h1_enc_odd_%d", j)); 
    TString hname_enc_adc(Form( "h1_enc_adc_%d", j)); 
    TString hname_enc_fast(Form( "h1_enc_fast_%d", j)); 
    TString hname_gain(Form( "h1_gain_%d", j)); 
    TString hname_thr_adc(Form( "h1_thr_adc_%d", j)); 
    TString hname_thr_fast(Form( "h1_thr_fast_%d", j));
    // --- Histograms definitions ---
    h1_enc_even[j] = new TH1F(hname_enc_even,"", 200, 0, 10000);
    h1_enc_odd[j]  = new TH1F(hname_enc_odd, "", 200, 0, 10000);
    h1_enc_adc[j]  = new TH1F(hname_enc_adc, "", 200, 0, 10000);
    h1_enc_fast[j]  = new TH1F(hname_enc_fast, "", 200, 0, 10000);
    h1_gain[j]  = new TH1F(hname_gain, "", 100, 0, 4000);
    h1_thr_adc[j]  = new TH1F(hname_thr_adc, "", 200, 5000, 20000);
    h1_thr_fast[j]  = new TH1F(hname_thr_fast, "", 200, 5000, 20000);
    // --- Histograms axis titles ---
    h1_enc_even[j]->SetTitle("Long run stability; ENC [e]; Entries");
    h1_enc_odd[j]->SetTitle("Long run stability; ENC [e]; Entries");
    h1_enc_adc[j]->SetTitle("Long run stability; ENC [e]; Entries");
    h1_enc_fast[j]->SetTitle("Long run stability; FAST ENC [e]; Entries");
    h1_gain[j]->SetTitle("Long run stability; ADC GAIN [e/LSB]; Entries");
    h1_thr_adc[j]->SetTitle("Long run stability; ADC THR [e]; Entries");
    h1_thr_fast[j]->SetTitle("Long run stability; FAST THR [e]; Entries");

    for (int ch =0; ch<128; ch++){
      h1_enc_adc[j]->Fill(enc[j][ch]);
      h1_gain[j]->Fill(gain[j][ch]);
      h1_thr_adc[j]->Fill(thr[j][ch]);
      if (fast_flag == true){
        h1_thr_fast[j]->Fill(thrf[j][ch]);
        h1_enc_fast[j]->Fill(encf[j][ch]);
      }
      if (ch%2 == 0)h1_enc_even[j]->Fill(enc[j][ch]);
      else h1_enc_odd[j]->Fill(enc[j][ch]);
    }
    // --------------------------------------- Fitting ENC data histograms --------------------------------------------
    // --- even channels ---
    cout<<"Analyzing ADC ENC:"<<j<<endl;
    int binmax = h1_enc_even[j]->GetMaximumBin();
    float mean_f = h1_enc_even[j]->GetXaxis()->GetBinCenter(binmax); 
    f_gaus_fit->SetParameter(1,mean_f);
    float lim_min = mean_f - 300;
    float lim_max = mean_f + 300;
    h1_enc_even[j]->Fit("f_gaus_fit", "SWR", "",lim_min, lim_max);
    h_time_enc_even->SetBinContent(j+1, h1_enc_even[j]->GetFunction("f_gaus_fit")->GetParameter(1));
    h_time_enc_even->SetBinError(j+1, h1_enc_even[j]->GetFunction("f_gaus_fit")->GetParameter(2));

    // --- odd channels ---
    binmax = h1_enc_odd[j]->GetMaximumBin();
    mean_f = h1_enc_odd[j]->GetXaxis()->GetBinCenter(binmax); 
    f_gaus_fit->SetParameter(1,mean_f);
    lim_min = mean_f - 300;
    lim_max = mean_f + 300;
    h1_enc_odd[j]->Fit("f_gaus_fit", "SWR", "",lim_min, lim_max);
    h_time_enc_odd->SetBinContent(j+1, h1_enc_odd[j]->GetFunction("f_gaus_fit")->GetParameter(1));
    h_time_enc_odd->SetBinError(j+1, h1_enc_odd[j]->GetFunction("f_gaus_fit")->GetParameter(2));
    h_time_enc_adc->SetBinContent(j+1, 0.5*(h1_enc_odd[j]->GetFunction("f_gaus_fit")->GetParameter(1)+h1_enc_even[j]->GetFunction("f_gaus_fit")->GetParameter(1)));

    // -------------------------------- Fitting ADC/FAST THR & GAIN histograms ------------------------------------
    // --- ADC GAIN ---
    cout<<"Analyzing ADC GAIN:"<<j<<endl;
    binmax = h1_gain[j]->GetMaximumBin();
    mean_f = h1_gain[j]->GetXaxis()->GetBinCenter(binmax); 
    f_gaus_fit->SetParameter(1,mean_f);
    lim_min = mean_f - 500;
    lim_max = mean_f + 500;
    h1_gain[j]->Fit("f_gaus_fit", "SWWR", "",lim_min, lim_max);
    h_time_gain->SetBinContent(j+1, h1_gain[j]->GetFunction("f_gaus_fit")->GetParameter(1));
    h_time_gain->SetBinError(j+1, h1_gain[j]->GetFunction("f_gaus_fit")->GetParameter(2));

    // --- ADC Thr ---
    cout<<"Analyzing ADC THR:"<<j<<endl;
    binmax = h1_thr_adc[j]->GetMaximumBin();
    mean_f = h1_thr_adc[j]->GetXaxis()->GetBinCenter(binmax); 
    f_gaus_fit->SetParameter(1,mean_f);
    lim_min = mean_f - 1000;
    lim_max = mean_f + 1000;
    h1_thr_adc[j]->Fit("f_gaus_fit", "SWR", "",lim_min, lim_max);
    h_time_thr_adc->SetBinContent(j+1, h1_thr_adc[j]->GetFunction("f_gaus_fit")->GetParameter(1));
    h_time_thr_adc->SetBinError(j+1, h1_thr_adc[j]->GetFunction("f_gaus_fit")->GetParameter(2));

    if (fast_flag == true){
      // --- FAST ENC ---
      cout<<"Analyzing FAST ENC:"<<j<<endl;
      binmax = h1_enc_fast[j]->GetMaximumBin();
      mean_f = h1_enc_fast[j]->GetXaxis()->GetBinCenter(binmax); 
      f_gaus_fit->SetParameter(1,mean_f);
      lim_min = mean_f - 500;
      lim_max = mean_f + 500;
      h1_enc_fast[j]->Fit("f_gaus_fit", "SWR", "",lim_min, lim_max);
      h_time_enc_fast->SetBinContent(j+1, h1_enc_fast[j]->GetFunction("f_gaus_fit")->GetParameter(1));
      h_time_enc_fast->SetBinError(j+1, h1_enc_fast[j]->GetFunction("f_gaus_fit")->GetParameter(2));

      // --- FAST Thr ---
      cout<<"Analyzing FAST THR:"<<j<<endl;
      binmax = h1_thr_fast[j]->GetMaximumBin();
      mean_f = h1_thr_fast[j]->GetXaxis()->GetBinCenter(binmax); 
      f_gaus_fit->SetParameter(1,mean_f);
      lim_min = mean_f - 1000;
      lim_max = mean_f + 1000;
      h1_thr_fast[j]->Fit("f_gaus_fit", "SWR", "",lim_min, lim_max);
      h_time_thr_fast->SetBinContent(j+1, h1_thr_fast[j]->GetFunction("f_gaus_fit")->GetParameter(1));
      h_time_thr_fast->SetBinError(j+1, h1_thr_fast[j]->GetFunction("f_gaus_fit")->GetParameter(2));
    }
  
    // --------------------------------------- Writting data histograms -------------------------------------------
    cd_h_enc->cd();
    h1_enc_even[j]->Write();
    h1_enc_odd[j]->Write();
    h1_enc_fast[j]->Write();
    h1_gain[j]->Write();
    h1_thr_adc[j]->Write();
    h1_thr_fast[j]->Write();

  }
  delete f_gaus_fit;
  f1->cd();
  h_time_enc_even->Write();
  h_time_enc_odd->Write();
  h_time_enc_adc->Write();
  h_time_enc_fast->Write();
  h_time_gain->Write();
  h_time_thr_adc->Write();
  h_time_thr_fast->Write();


  gStyle->SetErrorX(0);
  TCanvas *c1 = new TCanvas("c1", "c1", 1200, 600);
  c1->SetGrid();
  h_time_enc_even->Draw("PE1");
  h_time_enc_even->GetYaxis()->SetRangeUser(600, 1800);
  h_time_enc_even->SetLineStyle(1);
  h_time_enc_even->SetLineWidth(1);
  h_time_enc_even->SetLineColor(kMagenta-4); 
  h_time_enc_even->SetMarkerColor(kMagenta-4);
  h_time_enc_even->SetMarkerStyle(25);
  h_time_enc_even->SetMarkerSize(1.4);
  h_time_enc_odd->Draw("PE1same");
  h_time_enc_odd->SetLineStyle(1);
  h_time_enc_odd->SetLineWidth(1);
  h_time_enc_odd->SetLineColor(kGreen+2); 
  h_time_enc_odd->SetMarkerColor(kGreen+2);
  h_time_enc_odd->SetMarkerStyle(25);
  h_time_enc_odd->SetMarkerSize(1.4);
  h_time_enc_adc->Draw("Lsame");
  h_time_enc_adc->SetLineWidth(3);
  h_time_enc_adc->SetLineColor(kBlack);
  h_time_enc_adc->SetLineStyle(7);

  TLegend *lg_enc_summ = new TLegend(0.50,0.65, 0.85, 0.85);
  TString header = Form("Long run stability ASIC %d " + polarity, asic_hw_id);
  lg_enc_summ->SetHeader(header);
  lg_enc_summ->AddEntry(h_time_enc_even,"even channels","pl");
  lg_enc_summ->AddEntry(h_time_enc_odd,"odd channels","pl");
  lg_enc_summ->AddEntry(h_time_enc_adc,"average","l");
  lg_enc_summ->SetBorderSize(0);
  lg_enc_summ->SetFillColor(0);
  lg_enc_summ->Draw("");

  c1->Write();
  c1->Close();

  f1->Close();
}