// Distributed two-dimensional Discrete FFT transform
// ALAN BANNON

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include <algorithm>

#include "Complex.h"
#include "InputImage.h"

constexpr unsigned int NUMTHREADS = 4;

void Transform1D(Complex *h, int w, Complex *H, int numberInSequence);
void Transform2D(const char* inputFN);
void Transpose(Complex* input, int w, Complex* output);
void do_join(std::thread& t);

using namespace std;

//undergrad students can assume NUMTHREADS will evenly divide the number of rows in tested images
//graduate students should assume NUMTHREADS will not always evenly divide the number of rows in tested images.
//I will test with a different image than the one given

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  // 1) Use the InputImage object to read in the Tower.txt file and
  //    find the width/height of the input image.
  // 2) Create a vector of complex objects of size width * height to hold
  //    values calculated
  // 3) Do the individual 1D transforms on the rows assigned to each thread
  // 4) Force each thread to wait until all threads have completed their row calculations
  //    prior to starting column calculations
  // 5) Perform column calculations
  // 6) Wait for all column calculations to complete
  // 7) Use SaveImageData() to output the final results

    InputImage image(inputFN);  // Create the helper object for reading the image

    Complex *H = new Complex[image.GetWidth() * image.GetHeight()]; // Allocate the results array

    Complex *h = image.GetImageData();

    std::vector<std::thread> vectors;

    for(int i = 0; i < NUMTHREADS; ++i)
    {
        //std::future<void> myFuture0 = std::async(Transform1D, h, image.GetWidth(), H, i);
        vectors.push_back(std::thread(Transform1D, h, image.GetWidth(), H, i));
    }

    std::for_each(vectors.begin(),vectors.end(),do_join);


}


void do_join(std::thread& t)
{
    t.join();
}

void Transform1D(Complex *h, int w, Complex *H, int numberInSequence)
{
    // Implement a simple 1-d DFT using the double summation equation
    // given in the assignment handout.  h is the time-domain input
    // data, w is the width (N), and H is the output array.
    // nunmberInSequence tells the funciton how many calls have already been made, and so where it should
    // start in the h array.

    int startPoint   = ((w*w)/NUMTHREADS)*numberInSequence;  // where each thread will start working in the array
    int numberOfRows = w/NUMTHREADS; // we can assume the image is square

    int workingIndex = 0;
    for(int i = 0; i < numberOfRows; ++i)
    {
        workingIndex = i + startPoint;
        H[workingIndex] = h[workingIndex];      // TODO insert real DFT function here
    }

}

void Transpose(Complex* input, int w, Complex* output)
{
    // write a function to transpose the array
}

int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
    {
        Transform2D(fn.c_str()); // Perform the transform.

        //InverseTansform2D();
    }
}
  

  
