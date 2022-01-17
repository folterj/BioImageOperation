# Bio Image Operation script operations (v1.7.5 / 2022-01-17)


**Set** (Path, Width, Height, Fps, PixelSize, WindowSize)

Set parameters

 - Path:	 File path ("path")
 - Width:	 Width (numeric value)
 - Height:	 Height (numeric value)
 - Fps:	 Frames per second (numeric value)
 - PixelSize:	 Size of a pixel in arbitrary unit (numeric value)
 - WindowSize:	 Window size for moving average calculations [s] (numeric value)


**SetPath** (**Path**)

Set path for relative file paths (by default path of current script file)

 - Path:	 File path ("path")


**Source** (**Path**)

Open sources for individual processing

 - Path:	 File path ("path")


**CreateImage** (Width, Height, ColorMode, Red, Green, Blue)

Create a new image

 - Width:	 Width (numeric value)
 - Height:	 Height (numeric value)
 - ColorMode:	 Color mode (GrayScale, Color, ColorAlpha)
 - Red:	 Red color component (numeric value between 0 and 1)
 - Green:	 Green color component (numeric value between 0 and 1)
 - Blue:	 Blue color component (numeric value between 0 and 1)


**OpenImage** (**Path**, Start, Length, Interval, Total)

Open image file(s) for processing, accepts file name pattern

 - Path:	 File path ("path")
 - Start:	 Start (time reference as (hours:)minutes:seconds, or frame number)
 - Length:	 Length (time reference as (hours:)minutes:seconds, or frame number)
 - Interval:	 Interval in number of frames (numeric value)
 - Total:	 Total number of frames at regular interval (numeric value)


**OpenVideo** (**Path**, API, Start, Length, Interval, Total)

Open video file(s) and process frames, accepts file name pattern (ffmpeg formats supported)

 - Path:	 File path ("path")
 - API:	 OpenCV API code (See OpenCV API codes) (numeric value)
 - Start:	 Start (time reference as (hours:)minutes:seconds, or frame number)
 - Length:	 Length (time reference as (hours:)minutes:seconds, or frame number)
 - Interval:	 Interval in number of frames (numeric value)
 - Total:	 Total number of frames at regular interval (numeric value)


**OpenCapture** (API, Path, Source, Fps, Length, Interval, Total)

Open capturing from video (IP) path or camera source

 - API:	 OpenCV API code (See OpenCV API codes) (numeric value)
 - Path:	 File path ("path")
 - Source:	 Camera source (#) (numeric value)
 - Fps:	 Frames per second (numeric value)
 - Length:	 Length (time reference as (hours:)minutes:seconds, or frame number)
 - Interval:	 Interval in number of frames (numeric value)
 - Total:	 Total number of frames at regular interval (numeric value)


**SaveImage** (**Path**, Label, Start, Length)

Save image to file

 - Path:	 File path ("path")
 - Label:	 Label id (string)
 - Start:	 Start (time reference as (hours:)minutes:seconds, or frame number)
 - Length:	 Length (time reference as (hours:)minutes:seconds, or frame number)


**SaveVideo** (**Path**, Label, Start, Length, Fps, Codec)

Create video file and save image to video file (supports installed encoders)

 - Path:	 File path ("path")
 - Label:	 Label id (string)
 - Start:	 Start (time reference as (hours:)minutes:seconds, or frame number)
 - Length:	 Length (time reference as (hours:)minutes:seconds, or frame number)
 - Fps:	 Frames per second (numeric value)
 - Codec:	 Video encoding codec (4 character codec reference (FOURCC))


**ShowImage** (Label, Display)

Show image on screen (low priority screen updates)

 - Label:	 Label id (string)
 - Display:	 Display id (number 1 - 4)


**StoreImage** (**Label**)

Store current image in memory

 - Label:	 Label id (string)


**GetImage** (**Label**)

Get specified stored image from memory

 - Label:	 Label id (string)


**Grayscale** (Label)

Convert image to gray scale

 - Label:	 Label id (string)


**Color** (Label)

Convert image to color

 - Label:	 Label id (string)


**ColorAlpha** (Label)

Convert image to color with alpha channel

 - Label:	 Label id (string)


**GetSaturation** (Label)

Extract saturation from image

 - Label:	 Label id (string)


**GetHsValue** (Label)

Extract (HSV) Value from image

 - Label:	 Label id (string)


**GetHsLightness** (Label)

Extract (HSL) Lightness from image

 - Label:	 Label id (string)


**Scale** (Width, Height, Label)

Scale image (in pixels, or values between 0 and 1)

 - Width:	 Width (numeric value)
 - Height:	 Height (numeric value)
 - Label:	 Label id (string)


**Crop** (X, Y, Width, Height, Label)

Crop image (in pixels, or values between 0 and 1)

 - X:	 X position (numeric value)
 - Y:	 Y position (numeric value)
 - Width:	 Width (numeric value)
 - Height:	 Height (numeric value)
 - Label:	 Label id (string)


**Mask** (**Label**)

Perform mask on current image

 - Label:	 Label id (string)


**Threshold** (Label, Level)

Convert image to binary using threshold level, or in case not provided using automatic Otsu method

 - Label:	 Label id (string)
 - Level:	 Threshold value (numeric value between 0 and 1)


**Erode** (Label, Radius)

Apply erode filter (default 3x3 pixels)

 - Label:	 Label id (string)
 - Radius:	 Radius in pixels (numeric value)


**Dilate** (Label, Radius)

Apply dilate filter (default 3x3 pixels)

 - Label:	 Label id (string)
 - Radius:	 Radius in pixels (numeric value)


**Difference** (**Label**)

Perform difference of current image and specified image

 - Label:	 Label id (string)


**DifferenceAbs** (**Label**)

Perform absolute difference of current image and specified image

 - Label:	 Label id (string)


**Add** (**Label**)

Adds specified image to current image

 - Label:	 Label id (string)


**Multiply** (**Factor**)

Perform multiplication of all color channels by specified factor

 - Factor:	 Multiplication factor (numeric value)


**Invert** (Label)

Invert image

 - Label:	 Label id (string)


**SetBackground** (Label)

Initialise adaptive background buffer with image

 - Label:	 Label id (string)


**UpdateBackground** (Label, Weight)

Add image to the adaptive background buffer

 - Label:	 Label id (string)
 - Weight:	 Weight value (numeric value between 0 and 1)


**UpdateWeight** (Label, Weight)

Add image using weight to simple image buffer

 - Label:	 Label id (string)
 - Weight:	 Weight value (numeric value between 0 and 1)


**UpdateMin** (Label)

Add image and perform minimum on simple image buffer

 - Label:	 Label id (string)


**UpdateMax** (Label)

Add image and perform maximum on simple image buffer

 - Label:	 Label id (string)


**ClearSeries** ()

Clear image series buffer



**AddSeries** (Label, Maximum)

Add image to image series buffer

 - Label:	 Label id (string)
 - Maximum:	 Maximum number of images to keep (numeric value)


**GetSeriesMedian** (MedianMode)

Obtain image median of image series buffer

 - MedianMode:	 Median variation mode (Normal, Light, Dark)


**GetSeriesMean** ()

Obtain image mean of image series buffer



**AddAccum** (Label, AccumMode)

Add image to the accumulative buffer

 - Label:	 Label id (string)
 - AccumMode:	 Accumulation mode (Age, Usage)


**GetAccum** (Power, Palette)

Retrieve the accumulative buffer and convert to image

 - Power:	 Exponential power of value range (1E-[power] ... 1) (numeric value)
 - Palette:	 Palette (GrayScale, Heat, Rainbow)


**OpticalCalibration** (**NX**, **NY**, Label)

Calibrate optical correction using consistent internal edges of checkerboard pattern

 - NX:	 Number in X axis (numeric value)
 - NY:	 Number in Y axis (numeric value)
 - Label:	 Label id (string)


**OpticalCorrection** (Label)

Perform optical correction

 - Label:	 Label id (string)


**CreateClusters** (Tracker, MinArea, MaxArea, Debug)

Create clusters; auto calibrate using initial images if no parameters specified

 - Tracker:	 Tracker id (string)
 - MinArea:	 Minimum area in number of pixels (numeric value)
 - MaxArea:	 Maximum area in number of pixels (numeric value)
 - Debug:	 Debug mode (true / false)


**CreateTracks** (Tracker, MaxMove, MinActive, MaxInactive, Debug)

Create cluster tracking; auto calibrate using initial images if no parameters specified

 - Tracker:	 Tracker id (string)
 - MaxMove:	 Maximum movement distance (single frame) (numeric value)
 - MinActive:	 Minimum number of frames being active before state is active (numeric value)
 - MaxInactive:	 Maximum number of frames being inactive before state is inactive (numeric value)
 - Debug:	 Debug mode (true / false)


**CreatePaths** (Tracker, Distance, Debug)

Create common path usage

 - Tracker:	 Tracker id (string)
 - Distance:	 Maximum path distance (numeric value)
 - Debug:	 Debug mode (true / false)


**DrawClusters** (Label, Tracker, DrawMode)

Draw clusters

 - Label:	 Label id (string)
 - Tracker:	 Tracker id (string)
 - DrawMode:	 Draw mode(s) (combine using | character) (None, Point, Circle, Ellipse, Box, Angle, Label, LabelArea, LabelLength, LabelAngle, Track, Tracks, Fill, ClusterDefault, TracksDefault)


**DrawTracks** (Label, Tracker, DrawMode)

Draw tracked clusters

 - Label:	 Label id (string)
 - Tracker:	 Tracker id (string)
 - DrawMode:	 Draw mode(s) (combine using | character) (None, Point, Circle, Ellipse, Box, Angle, Label, LabelArea, LabelLength, LabelAngle, Track, Tracks, Fill, ClusterDefault, TracksDefault)


**DrawPaths** (Label, Tracker, PathDrawMode, Power, Palette)

Draw common paths

 - Label:	 Label id (string)
 - Tracker:	 Tracker id (string)
 - PathDrawMode:	 Path draw mode (Age, Usage, Usage2, Links, LinksMove)
 - Power:	 Exponential power of value range (1E-[power] ... 1) (numeric value)
 - Palette:	 Palette (GrayScale, Heat, Rainbow)


**DrawTrackCount** (Label, Tracker)

Draw tracking count on image

 - Label:	 Label id (string)
 - Tracker:	 Tracker id (string)


**SaveClusters** (**Path**, Tracker, Format, Contour)

Save clusters to CSV file

 - Path:	 File path ("path")
 - Tracker:	 Tracker id (string)
 - Format:	 Output format (ByTime, ByLabel, Split)
 - Contour:	 Extract contours (true / false)


**SaveTracks** (**Path**, Tracker, Format, Contour)

Save cluster tracking to CSV file

 - Path:	 File path ("path")
 - Tracker:	 Tracker id (string)
 - Format:	 Output format (ByTime, ByLabel, Split)
 - Contour:	 Extract contours (true / false)


**SavePaths** (**Path**, Tracker)

Save paths to CSV file

 - Path:	 File path ("path")
 - Tracker:	 Tracker id (string)


**ShowTrackInfo** (Tracker, Display)

Show tracking information on screen

 - Tracker:	 Tracker id (string)
 - Display:	 Display id (number 1 - 4)


**SaveTrackInfo** (**Path**, Tracker)

Save tracking information to CSV file

 - Path:	 File path ("path")
 - Tracker:	 Tracker id (string)


**DrawLegend** (Label, Display, Position)

Draw legend

 - Label:	 Label id (string)
 - Display:	 Display id (number 1 - 4)
 - Position:	 Draw position (Full, TopLeft, BottomLeft, TopRight, BottomRight)


**Wait** (MS)

Pause execution for a period (1000 ms default)

 - MS:	 Time in milliseconds (numeric value)


**Pause** ()

Pause processing



**Benchmark** ()

For benchmarking/debugging




(**Arguments:** [**required**] [optional])
