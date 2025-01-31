/**
 * File: rsg.cc
 * ------------
 * Provides the implementation of the full RSG application, which
 * relies on the services of the built-in string, ifstream, vector,
 * and map classes as well as the custom Production and Definition
 * classes provided with the assignment.
 */
 
#include <map>
#include <fstream>
#include <string>
#include "definition.h"
#include "production.h"
#include <assert.h>
using namespace std;

/**
 * Takes a reference to a legitimate infile (one that's been set up
 * to layer over a file) and populates the grammar map with the
 * collection of definitions that are spelled out in the referenced
 * file.  The function is written under the assumption that the
 * referenced data file is really a grammar file that's properly
 * formatted.  You may assume that all grammars are in fact properly
 * formatted.
 *
 * @param infile a valid reference to a flat text file storing the grammar.
 * @param grammar a reference to the STL map, which maps nonterminal strings
 *                to their definitions.
 */

static void readGrammar(ifstream& infile, map<string, Definition>& grammar)
{
  while (true) {
    string uselessText;
    getline(infile, uselessText, '{');
    if (infile.eof()) return;  // true? we encountered EOF before we saw a '{': no more productions!
    infile.putback('{');
    Definition def(infile);
    grammar[def.getNonterminal()] = def;
  }
}

/**
 * produceSentence produces one possible outcome of a context free grammar. For this purpose, it takes a nonterminal
 *(initially <start>), chooses a random production of the nonterminal and appends it to a vector. In case the production
 * itself contains a nonterminal, produceSentence is applied recursively to this nonterminal.
 *
 * @ param nonterminal: a nonterminal corresponding to a definition
 * @ param sen: a reference to a vector which stores the series of productions
 * @ param grammar: a hash map, where the key is a nonterminal specifying the definition  the while the value is the
 * complete definition object. 
 */
void  produceSentence(string nonterminal, vector<string>& sen , map<string, Definition>& grammar)
{
  assert(grammar.count(nonterminal) > 0);
  Production prod = grammar[nonterminal].getRandomProduction();
  for (Production::iterator curr = prod.begin(); curr != prod.end(); ++curr) {
      string terminalCand = *curr;
      int delStart = terminalCand.find("<");
      if(delStart == -1)
	sen.push_back(*curr);
      if(delStart != -1){
	int delEnd = terminalCand.find(">");
	string nextNonterminal = terminalCand.substr(delStart, delEnd - delStart + 1);
	produceSentence(nextNonterminal, sen, grammar);
      }    
    }
}

/**
 * Performs the rudimentary error checking needed to confirm that
 * the client provided a grammar file.  It then continues to
 * open the file, read the grammar into a map<string, Definition>,
 * and then print out the total number of Definitions that were read
 * in.  You're to update and decompose the main function to print
 * three randomly generated sentences, as illustrated by the sample
 * application.
 *
 * @param argc the number of tokens making up the command that invoked
 *             the RSG executable.  There must be at least two arguments,
 *             and only the first two are used.
 * @param argv the sequence of tokens making up the command, where each
 *             token is represented as a '\0'-terminated C string.
 */

int main(int argc, char *argv[])
{
  if (argc == 1) {
    cerr << "You need to specify the name of a grammar file." << endl;
    cerr << "Usage: rsg <path to grammar text file>" << endl;
    return 1; // non-zero return value means something bad happened 
  }
  
  ifstream grammarFile(argv[1]);
  if (grammarFile.fail()) {
    cerr << "Failed to open the file named \"" << argv[1] << "\".  Check to ensure the file exists. " << endl;
    return 2; // each bad thing has its own bad return value
  }
  
  // things are looking good...
  map<string, Definition> grammar;
  readGrammar(grammarFile, grammar);
  cout << "The grammar file called \"" << argv[1] << "\" contains "
       << grammar.size() << " definitions." << endl;


  // the following code produces an outcome of the grammar
  vector<string> sen; /** stores the outcome whereas each production in the outcome corresponds to an entry*/
  string finalSen; /** stores the entries of the vector sen as one string */

  produceSentence("<start>", sen, grammar);
  vector<string>::const_iterator curr = sen.begin();
  vector<string>::const_iterator end = sen.end();
  
  // Assemble vector sen into a string
  for (; curr != end; ++curr)
    finalSen += *curr + " ";
  
  // Print the outcome
  cout << finalSen << endl;
  return 0;
}
