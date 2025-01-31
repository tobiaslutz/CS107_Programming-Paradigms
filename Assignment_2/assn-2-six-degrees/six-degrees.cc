#include <vector>
#include <queue>
#include <set>
#include <string>
#include <iostream>
#include <iomanip>
#include "imdb.h"
#include "path.h"
using namespace std;

/**
 * Using the specified prompt, requests that the user supply
 * the name of an actor or actress.  The code returns
 * once the user has supplied a name for which some record within
 * the referenced imdb existsif (or if the user just hits return,
 * which is a signal that the empty string should just be returned.)
 *
 * @param prompt the text that should be used for the meaningful
 *               part of the user prompt.
 * @param db a reference to the imdb which can be used to confirm
 *           that a user's response is a legitimate one.
 * @return the name of the user-supplied actor or actress, or the
 *         empty string.
 */

static string promptForActor(const string& prompt, const imdb& db)
{
  string response;
  while (true) {
    cout << prompt << " [or <enter> to quit]: ";
    getline(cin, response);
    if (response == "") return "";
    vector<film> credits;
    if (db.getCredits(response, credits)) return response;
    cout << "We couldn't find \"" << response << "\" in the movie database. "
	 << "Please try again." << endl;
  }
}

/**
 * Switches source and target actor if source actor has more movies than target actor and returns true; otherwise false
 * @param s source actor/actress
 * @param t target actor/actress
 * @param db imdb object providing getCredits
 */
bool revSearchDir(string &s,string &t, imdb &db) {
  vector<film> movies;
  db.getCredits(s, movies);
  int numMovSrc = movies.size();
  movies.erase(movies.begin(), movies.end());
  db.getCredits(t, movies);
  int numMovTar = movies.size();
  if(numMovSrc > numMovTar) {
    string temp = s;
    s = t;
    t = temp;
    return true;
  }
  else return false;
}

/*
 * Returns true if the set s contains the film object f; otherwise false
 */
bool containsFilm(film &f, set<film> &s) {
  set<film>::iterator it;
  for(it = s.begin(); it != s.end(); it++) {
    if(*it == f) return true;
  }
  return false;
}

/**
 * Returns true if the set s contains the actor/actress Pl; otherwise false           
 */
bool containsActor(string &Pl, set<string> &s) {
  set<string>::iterator it;
  for(it = s.begin(); it != s.end(); it++) {
    if(*it == Pl) return true;
  }
  return false;
}

static void printPath(path &p, bool &rev) {
  if(!rev) p.reverse();
  for(int i = p.getLength(); i > 0; i--) {
    string firstPl = p.getLastPlayer();
    string movie = p.getLastMovie().title;
    int year = p.getLastMovie().year;
    p.undoConnection();
    string lastPl = p.getLastPlayer();
    printf("%s was in \"%s\" (%i) with %s\n ", firstPl.c_str(), movie.c_str(), year, lastPl.c_str());
  }
}

/**
 * Implementation of the breadth-first search algorithm
 *
 * @param s name of the s(ource) actor
 * @param t name of the t(arget) actor
 * @param db instantiation of the imdb class providing the getCast and getCredit method   
 */
static void generateShortestPath(string s, string t, imdb &db) {
  queue<path> partialPaths;
  set<string> previouslySeenActors;
  set<film> previouslySeenFilms;
  vector<film>::iterator itMovie;
  vector<string>::iterator itActor;

  bool srcTargRev = revSearchDir(s, t, db);
  path p(s);
  partialPaths.push(p);
  while(!partialPaths.empty() && (partialPaths.front().getLength() <= 5)) {
    path front = partialPaths.front();
    string actor = front.getLastPlayer();
    partialPaths.pop();
    vector<film> movies;
    db.getCredits(actor, movies);
    for(itMovie = movies.begin(); itMovie != movies.end(); itMovie++) {
      if(!containsFilm(*itMovie, previouslySeenFilms)) {
	previouslySeenFilms.insert(*itMovie);
	vector<string> players;
	db.getCast(*itMovie, players);
	for(itActor = players.begin(); itActor != players.end(); itActor++) {
	  if(!containsActor(*itActor, previouslySeenActors)) {
	    previouslySeenActors.insert(*itActor);
	    path clone = front;
	    clone.addConnection(*itMovie, *itActor);
	    if(*itActor == t) {
	       printPath(clone, srcTargRev);
	      return;
	    }
	    partialPaths.push(clone);
	  }
	}
      }
    }
  }
  printf("%s and %s are not connected by a sequence of movies and mutual co-stars. \n", s.c_str(),t.c_str());
}

/**
 * Serves as the main entry point for the six-degrees executable.
 * There are no parameters to speak of.
 *
 * @param argc the number of tokens passed to the command line to
 *             invoke this executable.  It's completely ignored
 *             here, because we don't expect any arguments.
 * @param argv the C strings making up the full command line.
 *             We expect argv[0] to be logically equivalent to
 *             "six-degrees" (or whatever absolute path was used to
 *             invoke the program), but otherwise these are ignored
 *             as well.
 * @return 0 if the program ends normally, and undefined otherwise.
 */

int main(int argc, const char *argv[])
{
  imdb db(determinePathToData(argv[1])); // inlined in imdb-utils.h
  if (!db.good()) {
    cout << "Failed to properly initialize the imdb database." << endl;
    cout << "Please check to make sure the source files exist and that you have permission to read them." << endl;
    exit(1);
  }
  
  while (true) {
    string source = promptForActor("Actor or actress", db);
    if (source == "") break;
    string target = promptForActor("Another actor or actress", db);
    if (target == "") break;
    if (source == target) {
      cout << "Good one.  This is only interesting if you specify two different people." << endl;
    } else {
      generateShortestPath(source, target, db);
    }
  }
  
  cout << "Thanks for playing!" << endl;
  return 0;
}

