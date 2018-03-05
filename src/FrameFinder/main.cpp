#include <iostream>

#include "FFhelpers.h"
#include "timer.h"
#include <thread>

using namespace std;
using namespace timer;

/**
 * Set _PATH_DATA to the folder in which your images recide
 *
 */
#define _PATH_DATA      "/Users/jl/Downloads/fagprojekt/data/ImgD1/"
#define _PATH_ACCEPTED  "./accepted.txt"
#define _PATH_DISCARDED "./discarded.txt"

int main() {
    Timer t;

    vector<string> a;

    tic(t);
    FF::get_files_sorted(a, _PATH_DATA, FF::FF_ONLY_FILE);
    toc(t);

    tic(t);
    FF::accept_or_reject(a, _PATH_DATA, _PATH_ACCEPTED,_PATH_DISCARDED);
    toc(t);


    return 0;

}
