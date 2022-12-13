import cv2 as cv
import time


if __name__ == '__main__':
    cap = cv.VideoCapture()
    cap.open(0)
    if not cap.isOpened():
        raise IOError("Cannot open webcam")
    cap.set(cv.CAP_PROP_FRAME_WIDTH, 1920)
    cap.set(cv.CAP_PROP_FRAME_HEIGHT, 1080)

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
