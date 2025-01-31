using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

bool imdb::getCredits(const string& player, vector<film>& films) const {
  int aOffset = bActorSearch(player);
  if(aOffset == -1)
    return false;
  else {
    char *actor = (char *)actorFile + aOffset;
    short *numMovies = getNumMovies(player, actor);
    bool toPad = needToPadActF(player); 
    bool foo = isEvenLen(player);
    fillFilmVec(numMovies, films, toPad);
    return true;
  }
 }


bool imdb::getCast(const film& movie, vector<string>& players) const { 
  int mOffset = bMovieSearch(movie);
  int padNum = 0;
  // Possible padding between year and number of actors
  if(!isEvenLen(movie.title)) padNum = 1;
  if(mOffset == -1) return false;
  else {
    short *numActors = (short *)((char *)movieFile + mOffset + padNum + strlen((movie.title).c_str()) + 2);
    // Possible padding between number of actors and actors 
    if(needToPadMovF(movie))
      padNum = 2;
    else
      padNum = 0;
   
    fillActVec(numActors, padNum, players);
    return true;
  }
 }

/**
 * Returns the offset (in terms of number of bytes) from actorFile to the actor data using binary search 
 * @param player the actor/actress of interest.  
 */
int imdb::bActorSearch(const string& player) const {
  int h = *(int *)actorFile; //number of actors in imdb
  int l = 1;
  while (l <= h){
    int m = (l + h) / 2;
    int mOffset = *((int *)actorFile + m);
    char *temp_player = (char *)actorFile + mOffset;   
    if(strcmp(player.c_str(),temp_player) == 0)
      return mOffset;
    else if (strcmp(player.c_str(),temp_player) > 0)
      l = m + 1;
    else if (strcmp(player.c_str(),temp_player) < 0)
      h = m - 1;
  }
  return -1;
}

/*
 * Returns the offset (in terms of number of bytes) from movieFile to the movie data using binary search
 * @param movie the movie of interest. 
 */
int imdb::bMovieSearch(const film& movie) const {
  int h = *(int *)movieFile; //number of movies in imdb
  int l = 1;
  film f;
  while (l <= h){
    int m = (l + h) / 2;
    int mOffset = *((int *)movieFile + m);
    char *temp_movie = (char *)movieFile + mOffset;
    createFilm(f,temp_movie);
    if(movie ==  f)
      return mOffset;
    else if (movie < f)
      h = m - 1;
    else
      l = m + 1;
  }
  return -1;
}


/**
 * Returns true if the string parameter has even length
 * @param player the player of interest. 
 */
bool imdb::isEvenLen(const string& player) const {
  return (strlen(player.c_str()) % 2 == 0);
}

/**
 * Returns the number of movies an actor was in
 * @param player the actor/actress of interest.
 * @param actor the address in the actor record where the name of the actor starts.  
 */
short* imdb::getNumMovies(const string& player, char *actor) const {
  bool isEven = isEvenLen(player);
  short *num;
  if(isEven)
    num = (short *)(actor + strlen(player.c_str()) + 2);
  else 
    num = (short *)(actor + strlen(player.c_str()) + 1);

  return num;
}

/**
 * Returns true if the two bytes in the actorFile array, which indicate the number of movies, have to be padded with two zero bytes
 * @param player the actor/actress of interest.   
 */
bool imdb::needToPadActF(const string& player) const {
  bool isEven = isEvenLen(player);
  if(isEven){
    if((strlen(player.c_str()) + 4) % 4 == 0) return false;
    return true;
  }
  else {
    if((strlen(player.c_str()) + 3) % 4 == 0) return false;
    return true;
  }
}

/*
 * Returns true if the two bytes in the movieFile array indicating the number of actors have to be padded with two zero bytes   
 * @param movie the movie of interest.   
 */
bool imdb::needToPadMovF(const film& movie) const {
  int nameYearLen = strlen((movie.title).c_str()) + 2;
  if(nameYearLen % 2 == 0){
    if((nameYearLen + 2) % 4 == 0)   return false;
    return true;
  }
  else {
    if((nameYearLen + 3) % 4 == 0) return false;
    return true;
  }
}

/*
 * Assigns a title and a year to the film object passed b reference
 * @param movie the film object to be filled.
 * @param movieAddr the starting address of the movie of interest in the movieFile array       
 */
void imdb::createFilm(film& movie, char *movieAddr) const {
  string m(movieAddr);
  char year = *(movieAddr + strlen(movieAddr) + 1);
  movie.title = m;
  movie.year = (int)year + 1900;
}

/**
 * Creates a film object for each movie of an actor/actress and stores the film objects in the vector "films"
 * @param numMovies pointer to the number of movies of the actor/actress of interest.
 * @param films vector to be filled film objects.
 * @param toPad boolean indicating whether the two bytes in the actorFile array, which indicate the number of movies, 
 * have to be padded with two zero bytes.         
 */
void imdb::fillFilmVec(short *numMovies, vector<film>& films, bool toPad) const {
  short num = *numMovies;
  int *base;
  if(toPad) base = (int *)((char *)numMovies + 4);
  else base = (int *)((char *)numMovies + 2);
  film f;
  for(int i = 0; i < num; i++) {
    int mOffset = *(base + i);
    char *movie = (char *)movieFile + mOffset;
    createFilm(f, movie);
    films.push_back(f);
  }
}

/*
 * Stores the actors/actress of a movie in the vector "players"
 * @param numActors the adress to the starting byte in the movieFile array indicating the number of actors of a movie.
 * @param padNum number of zero bytes following the actual number of actors in the movieFile array.   
 */
void imdb::fillActVec(short *numActors, int padNum, vector<string> &players) const {
  for(int i = 0; i < *numActors; i++) {
    int aOffset = *(int *)((char *)(numActors) + 2 + padNum + i * 4);
     char *actor = (char *)(actorFile) + aOffset;
    string name(actor);
    players.push_back(name);
   }
}

imdb::~imdb(){
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
