#include <iostream>
#include <fstream>
#include <cmath>

#include <TFile.h>
#include <TDirectory.h>
#include <TKey.h>
#include <TH1.h>

#include <YODA/Histo1D.h>
#include <YODA/Dbn1D.h>
#include <YODA/WriterYODA.h>

using namespace std;
using namespace YODA;

#define test(var) \
  cout <<"\033[36m"<< #var <<"\033[0m"<< " = " << var << endl;

const Histo1D* convert(const TH1* rh, string path) noexcept {
  const Int_t nbins = rh->GetNbinsX();
  const TArrayD& sumw2 = *rh->GetSumw2();

  vector<HistoBin1D> bins;
  bins.reserve(nbins);
  
  Dbn1D total;
  for (Int_t i=1; i<=nbins; ++i) {
    Dbn1D dbn(0, rh->GetBinContent(i), sumw2.GetSize() ? sumw2[i] : 0., 0., 0.);
    bins.emplace_back(make_pair(rh->GetBinLowEdge(i),rh->GetBinLowEdge(i+1)), dbn);
    total += dbn;
  }

  path = path.substr(path.find('/'));
  if (path.back()!='/') path += '/';
  path += rh->GetName();

  auto hist = new Histo1D(bins, path, rh->GetName());
  total += ( hist->underflow() = Dbn1D(
    0, rh->GetBinContent(0), sumw2.GetSize() ? sumw2[0] : 0., 0., 0.) );
  total += ( hist-> overflow() = Dbn1D(
    0, rh->GetBinContent(nbins+1), sumw2.GetSize() ? sumw2[nbins+1] : 0., 0., 0.) );
  total += Dbn1D(lround(rh->GetEntries()), 0.,0.,0.,0.);
  hist->totalDbn() = total;

  return hist;
}

void read(TDirectory *d, vector<const AnalysisObject*>& hists) noexcept {
  TKey *key;
  TIter nextkey(d->GetListOfKeys());
  while ((key = static_cast<TKey*>(nextkey()))) {
    TObject *obj = key->ReadObj();
    if (obj->InheritsFrom(TDirectory::Class())) {
      read(static_cast<TDirectory*>(obj),hists);
    } else if (obj->InheritsFrom(TH1::Class())) {
      hists.push_back(
        convert(static_cast<TH1*>(obj),d->GetPath())
      );
    }
  }
}

int main(int argc, char **argv) {
  if (argc!=2 && argc!=3) {
    cout << "Usage: " << argv[0] << " in.root [out.yoda]" << endl;
    return 0;
  }
  
  // determine output file name
  string fout;
  if (argc==2) {
    string fin(argv[1]);
    size_t sep = fin.find('/');
    if (sep!=string::npos) fin = fin.substr(sep+1);
    sep = fin.rfind('.');
    if (sep!=string::npos) fin = fin.substr(0,sep);
    fout = fin + ".yoda";
  } else fout = argv[2];

  // open root file
  TFile *fin = new TFile(argv[1],"read");
  if (fin->IsZombie()) return 1;
  cout << "Input : " << fin->GetName() << endl;
  
  // read root file and convert all TH1s
  vector<const AnalysisObject*> hists;
  read(fin,hists);
  cout << hists.size() << " hists" << endl;

  // write output file
  WriterYODA::create().write(fout,hists);
  cout << "Output: " << fout << endl;

  return 0;
}
