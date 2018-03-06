#include <iostream>

#include "rbc.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>

using namespace cv;

int main(int argc, char **argv) {

  if (argc != 2) {
    printf("usage: DisplayImage.out <Image_Path>\n");
    return -1;
  }

  RBC a;

  Mat image;
  //image = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    image = imread("/Users/eskidlbs/Desktop/data/ImgD1/081116D1_28932.png", CV_LOAD_IMAGE_COLOR);

  if (!image.data) {
    printf("No image data \n");
    return -1;
  }
  namedWindow("Display Image", WINDOW_AUTOSIZE);
  imshow("Display Image", image);

  waitKey(0);

  return 0;
}
