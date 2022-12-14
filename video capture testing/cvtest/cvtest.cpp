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

    cap.setExceptionMode(true);

    vector<int> params = { VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, 1920, VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, 1080 };
    try {
        if (!cap.open(0, VideoCaptureAPIs::CAP_ANY, params)) {
            cout << "Unable to open camera" << endl;
            return -1;
        }

        cout << "Camera back end: " << cap.getBackendName() << endl;
        if (!cap.isOpened()) {
            cout << "Unable to open camera" << endl;
            return -1;
        }
        //cap.set(VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, 1920);
        //cap.set(VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, 1080);

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
                cout << totalElapseds << endl;
            }
            counter++;
        }
    } catch (cv::Exception& e) {
        cout << "Exception " << e.what() << endl;
    }
}
