#include <iostream>
#include <fstream>

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

const Histo1D* convert(const TH1* rh, const string& path) noexcept {
  const Int_t nbins = rh->GetNbinsX()+2;
  const TArrayD& sumw2 = *rh->GetSumw2();

  vector<HistoBin1D> bins;
  bins.reserve(nbins);
  
  for (Int_t i=0; i<nbins; ++i) {
    bins.emplace_back(
      make_pair(rh->GetBinLowEdge(i),rh->GetBinLowEdge(i+1)),
      Dbn1D(1, rh->GetBinContent(i), sumw2.GetSize() ? sumw2[i] : 1., 1., 1.)
    );
  }

  test(rh->GetName())
  return new Histo1D(bins,'/'+path,rh->GetName());
}

void read(TDirectory *d, vector<const Histo1D*>& hists) noexcept {
  test(d->GetName())
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
  cout << "File: " << fin->GetName() << endl;
  
  // read root file and convert all TH1s
  vector<const Histo1D*> hists;
  read(fin,hists);
  cout << hists.size() << " hists" << endl;

  // write output file
  WriterYODA::create().write(fout,hists);

  return 0;
}
