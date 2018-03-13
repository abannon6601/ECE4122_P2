// Distributed two-dimensional Discrete FFT transform
// ALAN BANNON

#define _USE_MATH_DEFINES
#include <cmath>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include <algorithm>
#include <math.h>

#include "Complex.h"
#include "InputImage.h"

constexpr unsigned int NUMTHREADS = 4;

void Transform1D(Complex *h, int w, Complex *H, int numberInSequence, int inverse);
void Transform2D(const char *inputFN, int inverse);
void Transpose(Complex *input, int w);
void do_join(std::thread& t);

using namespace std;


//undergrad students can assume NUMTHREADS will evenly divide the number of rows in tested images
//graduate students should assume NUMTHREADS will not always evenly divide the number of rows in tested images.
//I will test with a different image than the one given

void Transform2D(const char *inputFN)
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

    Complex *h = image.GetImageData();  // create a local pointer to the raw data

    std::vector<std::thread> vectors;

    for(int i = 0; i < NUMTHREADS; ++i)
    {
        vectors.push_back(std::thread(Transform1D, h, image.GetWidth(), H, i, -1)); // fire vectors at the problem
    }

    std::for_each(vectors.begin(),vectors.end(),do_join);   // ensure all threads are complete before continuing
    vectors.clear();

    // row calculations now complete

    Transpose(H, image.GetWidth());  // transpose the result matrix

     for(int i = 0; i < image.GetWidth()*image.GetHeight(); ++i) // copy the result matrix back into the orginal array
     {
         h[i] = H[i];
     }

     for(int i = 0; i < NUMTHREADS; ++i)
     {
         vectors.push_back(std::thread(Transform1D, h, image.GetWidth(), H, i, -1)); // fire vectors at the problem
     }

     std::for_each(vectors.begin(),vectors.end(),do_join);   // ensure all threads are complete before continuing
     vectors.clear();

     // colomb calculations now complete

     Transpose(H, image.GetWidth());  // transpose the result matrix

    string filename = "MyAfter2D.txt";
    image.SaveImageData(filename.c_str(),H,image.GetWidth(),image.GetHeight());




    // now do inverse transform


    for(int i = 0; i < image.GetWidth()*image.GetHeight(); ++i) // copy the result matrix back into the orginal array
    {
        h[i] = H[i];
    }

    for(int i = 0; i < NUMTHREADS; ++i)
    {
        vectors.push_back(std::thread(Transform1D, h, image.GetWidth(), H, i, 1)); // fire vectors at the problem
        //    std::cout << "FFT2D_DEBUG: Running thread: " << i << std::endl;
    }

    std::for_each(vectors.begin(),vectors.end(),do_join);   // ensure all threads are complete before continuing
    vectors.clear();

    Transpose(H, image.GetWidth());  // transpose the result matrix

    for(int i = 0; i < image.GetWidth()*image.GetHeight(); ++i) // copy the result matrix back into the orginal array
    {
        h[i] = H[i];
    }

    for(int i = 0; i < NUMTHREADS; ++i)
    {
        vectors.push_back(std::thread(Transform1D, h, image.GetWidth(), H, i, 1)); // fire vectors at the problem
    }

    std::for_each(vectors.begin(),vectors.end(),do_join);   // ensure all threads are complete before continuing
    vectors.clear();

    Transpose(H, image.GetWidth());  // transpose the result matrix

    for(int i = 0; i < image.GetWidth()*image.GetHeight(); ++i) // remove the small errors introduced during the DFT process to recover the original image
    {
        H[i].imag = 0;
        H[i].real = round(H[i].real);
    }

    filename = "MyAfterInverse.txt";
    image.SaveImageData(filename.c_str(),H,image.GetWidth(),image.GetHeight());
}

void Transpose(Complex *input, int dim) // transposes the square matrix  *input of dimention dim
{
    Complex *HoldingInput  = new Complex[dim * dim]; // Allocate a working array

    int x,y;
    for(y = 0; y <dim; ++y)
    {
        for(x = 0; x <dim; ++x)
        {
           HoldingInput[x + y*dim] = input[x*dim + y];
        }
    }

    for(int i = 0; i < dim*dim; ++i)
    {
        input[i] = HoldingInput[i];
    }

    delete(HoldingInput);
    // there are faster and more efficent ways to do this, but this is easier to follow and code
}

void do_join(std::thread& t)    // simple function called to join a thread
{
    t.join();
}

void Transform1D(Complex *h, int w, Complex *H, int numberInSequence, int inverse)
{
    // Implement a simple 1-d DFT using the double summation equation
    // given in the assignment handout.  h is the time-domain input
    // data, w is the width (N), and H is the output array.
    // nunmberInSequence tells the funciton how many calls have already been made, and so where it should
    // start in the h array.

    Complex W;
    W.imag = inverse * (sin((2*M_PI)/w));   //setup constants. "inverse" here is used if this is an IDFT
    W.real = cos((2*M_PI)/w);

    int startPoint   = ((w*w)/NUMTHREADS)*numberInSequence;  // where each thread will start working in the array
    int numberOfRows = w/NUMTHREADS; // we can assume the image is square

    int workingIndex = 0;
    int rowStartIndex = 0;

    Complex sum_holdder(1,0);
    Complex w_n(1,0);           // utitilty objects
    Complex w_nk(1,0);

    for(int i = 0; i < numberOfRows; i++)
    {
        w_n.real = 1;
        w_n.imag = 0;

        rowStartIndex = startPoint + i*w;

        for(int n = 0; n < w; n++)
        {
            workingIndex = rowStartIndex + n;

            sum_holdder.real = 0;
            sum_holdder.imag = 0;
            w_nk.real = 1;
            w_nk.imag = 0;

            for(int k = 0; k < w; k++)
            {
                sum_holdder = sum_holdder + w_nk * h[rowStartIndex + k];    //perform each step of the DFT
                w_nk = w_nk*w_n;
            }
            if(inverse == -1)
            {
                H[workingIndex].real = sum_holdder.real;    //DFT
                H[workingIndex].imag = sum_holdder.imag;
            }
            else
            {
                H[workingIndex].real = sum_holdder.real/w;  // inverse DFT
                H[workingIndex].imag = sum_holdder.imag/w;
            }


            w_n = w_n * W;  // incriment the power of w_n
        }
    }

}

int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line

    Transform2D(fn.c_str()); // Perform the transform.

}
  

  
