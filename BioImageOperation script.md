# Bio Image Operation script operations (v1.6.440.5 / 2020-12-01)


**SetPath** (**Path**)

Set path for relative file paths (by default path of current script file)

 - Path:	 File path ("path")


**CreateImage** (Width, Height, ColorMode, Red, Green, Blue)

Create a new image

 - Width:	 Width in pixels (numeric value)
 - Height:	 Height in pixels (numeric value)
 - ColorMode:	 Color mode (GrayScale, Color, ColorAlpha)
 - Red:	 Red color component (numeric value between 0 and 1)
 - Green:	 Green color component (numeric value between 0 and 1)
 - Blue:	 Blue color component (numeric value between 0 and 1)


**OpenImage** (**Path**, Start, Length, Interval)

Open image file(s) for processing, accepts file name pattern

 - Path:	 File path ("path")
 - Start:	 Start (time reference as (hours:)minutes:seconds, or frame number)
 - Length:	 Length (time reference as (hours:)minutes:seconds, or frame number)
 - Interval:	 Interval in number of frames (numeric value)


**OpenVideo** (**Path**, API, Start, Length, Interval)

Open video file(s) and process frames, accepts file name pattern (ffmpeg formats supported)

 - Path:	 File path ("path")
 - API:	 OpenCV API code (See OpenCV API codes) (numeric value)
 - Start:	 Start (time reference as (hours:)minutes:seconds, or frame number)
 - Length:	 Length (time reference as (hours:)minutes:seconds, or frame number)
 - Interval:	 Interval in number of frames (numeric value)


**OpenCapture** (API, Path, Source, Width, Height, Interval)

Open capturing from video (IP) path or camera source

 - API:	 OpenCV API code (See OpenCV API codes) (numeric value)
 - Path:	 File path ("path")
 - Source:	 Camera source (#) (numeric value)
 - Width:	 Width in pixels (numeric value)
 - Height:	 Height in pixels (numeric value)
 - Interval:	 Interval in number of frames (numeric value)


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

 - Width:	 Width in pixels (numeric value)
 - Height:	 Height in pixels (numeric value)
 - Label:	 Label id (string)


**Crop** (X, Y, Width, Height, Label)

Crop image (in pixels, or values between 0 and 1)

 - X:	 X position in pixels (numeric value)
 - Y:	 Y position in pixels (numeric value)
 - Width:	 Width in pixels (numeric value)
 - Height:	 Height in pixels (numeric value)
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


**UpdateBackground** (Label, Weight)

Add image to the adaptive background buffer

 - Label:	 Label id (string)
 - Weight:	 Weight value (numeric value between 0 and 1)


**UpdateAverage** (Label, Weight)

Add image to the average buffer

 - Label:	 Label id (string)
 - Weight:	 Weight value (numeric value between 0 and 1)


**ClearSeries** ()

Clear image series buffer



**AddSeries** (Label, Maximum)

Add image to image series buffer

 - Label:	 Label id (string)
 - Maximum:	 Maximum number of images to keep (numeric value)


**GetSeriesMedian** ()

Retrieve median image of image series buffer



**AddAccum** (Label, AccumMode)

Add image to the accumulative buffer

 - Label:	 Label id (string)
 - AccumMode:	 Accumulation mode (Age, Usage)


**GetAccum** (Power, Palette)

Retrieve the accumulative buffer and convert to image

 - Power:	 Exponential power of value range (1E-[power] ... 1) (numeric value)
 - Palette:	 Palette (GrayScale, Heat, Rainbow)


**CreateClusters** (Tracker, MinArea, MaxArea)

Create clusters; auto calibrate using initial images if no parameters specified

 - Tracker:	 Tracker id (string)
 - MinArea:	 Minimum area in number of pixels (numeric value)
 - MaxArea:	 Maximum area in number of pixels (numeric value)


**CreateTracks** (Tracker, MaxMove, MinActive, MaxInactive)

Create cluster tracking; auto calibrate using initial images if no parameters specified

 - Tracker:	 Tracker id (string)
 - MaxMove:	 Maximum movement distance (single frame) (numeric value)
 - MinActive:	 Minimum number of frames being active before state is active (numeric value)
 - MaxInactive:	 Maximum number of frames being inactive before state is inactive (numeric value)


**CreatePaths** (Tracker, Distance)

Create common path usage

 - Tracker:	 Tracker id (string)
 - Distance:	 Maximum path distance (numeric value)


**DrawClusters** (Label, Tracker, DrawMode)

Draw clusters

 - Label:	 Label id (string)
 - Tracker:	 Tracker id (string)
 - DrawMode:	 (Combination of) draw mode(s) (None, Point, Circle, Box, Angle, Label, Labeln, Track, Tracks, Fill, ClusterDefault, TracksDefault)


**DrawTracks** (Label, Tracker, DrawMode)

Draw tracked clusters

 - Label:	 Label id (string)
 - Tracker:	 Tracker id (string)
 - DrawMode:	 (Combination of) draw mode(s) (None, Point, Circle, Box, Angle, Label, Labeln, Track, Tracks, Fill, ClusterDefault, TracksDefault)


**DrawPaths** (Label, Tracker, PathDrawMode, Power, Palette)

Draw common paths

 - Label:	 Label id (string)
 - Tracker:	 Tracker id (string)
 - PathDrawMode:	 Path draw mode (Age, Usage, Usage2, Links, LinksMove)
 - Power:	 Exponential power of value range (1E-[power] ... 1) (numeric value)
 - Palette:	 Palette (GrayScale, Heat, Rainbow)


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


**DrawTrackInfo** (Label, Tracker)

Draw tracking stats on image

 - Label:	 Label id (string)
 - Tracker:	 Tracker id (string)


**SaveTrackInfo** (**Path**, Tracker)

Save tracking information to CSV file

 - Path:	 File path ("path")
 - Tracker:	 Tracker id (string)


**SaveTrackLog** (**Path**, Tracker)

Save tracking log to CSV file

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


**Debug** ()

Debug mode




**Arguments:** **Required** Optional
