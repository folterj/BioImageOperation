#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace cv;

typedef high_resolution_clock Clock;


int main()
{
    VideoCapture cap;
    Mat image;
    bool ret;
    Clock::time_point t, t0;
    duration<double> totalElapsed;
    double totalElapseds;
    int max_counter = 100;
    int counter = 0;

    cap.open(0);
    if (!cap.isOpened()) {
        throw new Exception();
    }
    cap.set(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, 1080);

    t0 = high_resolution_clock::now();

    while (true) {
        ret = cap.read(image);
        //cap >> image;
        if (counter == max_counter) {
            t = high_resolution_clock::now();
            totalElapsed = (t - t0) / counter;
            totalElapseds = totalElapsed.count();
            t0 = t;
            counter = 0;
            cout  << totalElapseds << endl;
        }
        counter++;
    }
}
