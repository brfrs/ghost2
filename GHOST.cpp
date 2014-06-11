#include <iostream>
#include <string>
#include <stdlib.h>
#include "config.hpp"
#include "graph.hpp"
#include "readFromGexf.hpp"
#include "readFromNet.hpp"
#include "computeSpectralSignatures.hpp"
#include "blastDistance.hpp"
#include "spectralToDistance.hpp"
#include "alignGraphs.hpp"

using std::string;
using std::cout;

void computeAlignment(ConfigData c)
{
  // read in graph
  Graph G,H;
  if(c.Ggraph.substr(c.Ggraph.size()-5) == ".gexf")
    G = readFromGexf(c.Ggraph);
  else if(c.Ggraph.substr(c.Ggraph.size()-4) == ".net")
    G = readFromNet(c.Ggraph);
  else {cout << "bad extension: " << c.Ggraph << "\n"; exit(0);}

  if(c.Hgraph.substr(c.Hgraph.size()-5) == ".gexf")
    H = readFromGexf(c.Hgraph);
  else if(c.Hgraph.substr(c.Hgraph.size()-4) == ".net")
    H = readFromNet(c.Hgraph);
  else {cout << "bad extension: " << c.Hgraph << "\n"; exit(0);}

  // compute spectral signatures
  if(c.Gsigs == "")
  {
    computeSpectralSignatures(&G, c.hops, c.numProcessors);
    c.Gsigs = (G.getName() + ".sig.gz");
  }
  if(c.Hsigs == "")
  {
    computeSpectralSignatures(&H, c.hops, c.numProcessors);
    c.Hsigs = (H.getName() + ".sig.gz");
  }
  if(c.dumpSignatures) return; // if user only wanted sigs

  // get evalues if given
  blastMap *evals = new blastMap;
  if(c.SeqScores == "")
    evals = NULL;
  else
    *evals = getBlastMap(c.SeqScores);

  // compute distances
  vector<D_alpha> dist = 
    getDistances(c.Gsigs, c.Hsigs, (G.getName()+"_vs_"+H.getName()+".sdf"), 
                 c.alpha, evals , c.numProcessors);
  if(c.dumpDistances) return; // if user wanted just the distances...
  delete evals;

  // align graphs
  alignGraphs(G, H, dist, c.beta, c.nneighbors);
}

int main(int argc, char** argv)
{
  ConfigData c;

  // read in options
  for(int i=1; i<argc; i++)
  {
    if(string(argv[i]) == "-c") 
      if((i+1)<argc) 
        c.configure(string(argv[i+1]));
    if(string(argv[i]) == "-k")
      if((i+1)<argc)
        c.hops = atoi(argv[i+1]);
    if(string(argv[i]) == "-p")
      if((i+1)<argc)
        c.numProcessors = atoi(argv[i+1]);
  }

  if(c.Ggraph == "" || c.Hgraph == "") // required input
    { cout << "gexf files not provided\n"; return 0; }

  // and here we go!
  computeAlignment(c);
  return 0;
}

