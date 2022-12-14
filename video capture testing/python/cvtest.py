import cv2 as cv
import time


if __name__ == '__main__':
    params = [cv.CAP_PROP_FRAME_WIDTH, 1920, cv.CAP_PROP_FRAME_HEIGHT, 1080]
    cap = cv.VideoCapture()
    if not cap.open(0, cv.CAP_ANY, params):
        raise IOError("Unable to open camera")
    print("Camera back end:", cap.getBackendName())
    if not cap.isOpened():
        raise IOError("Unable to open camera")
    #cap.set(cv.CAP_PROP_FRAME_WIDTH, 1920)
    #cap.set(cv.CAP_PROP_FRAME_HEIGHT, 1080)

    max_counter = 100
    counter = 0
    t0 = time.process_time()

    while True:
        ret, image = cap.read()
        if counter == max_counter:
            t = time.process_time()
            elapsed_time = (t - t0) / counter
            t0 = t
            counter = 0
            print(elapsed_time)
        counter += 1

    cap.release()
