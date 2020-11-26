# BioImageOperation

Bio Image Operation or 'BIO', is a next generation Image processing tool focusing on biological applications, balancing ease of use with desired flexibility required for research.

![BIO screenshot](bio.png)

This tool has been developed in collaboration with biologists, using extensive captured images.

The solution balancing both the need for research purposes and flexibility required for this, and desired ease of use is realised in a script based user interface.

The tool uses the widely used OpenCV for many of it's image operations, with an efficient tracking algorithm allowing real time processing.

This is a Microsoft Visual Studio 2019 / C++ CMake project

The project can either be opened as a Visual Studio project opening the solution file, or as a CMake project by opening the BioImageOperation root folder.

## Requirements
- CMake ([cmake.org](https://cmake.org/)) / Visual Studio ([visualstudio.microsoft.com](https://visualstudio.microsoft.com/))
- Qt 5 ([qt.io](https://www.qt.io))
- OpenCV 4.4.0 ([opencv.org](https://opencv.org))
- Openh264 1.8.0 library ([github.com/cisco/openh264](https://github.com/cisco/openh264))

## Scripting step by step guide

### 1.	Sourcing
Source images or video(s)
```javascript
OpenVideo("ants_in_concrete.mov")
{
  ShowImage()
}
```
This operation will open the source video, and the open bracket (‘{‘) defines a set of inner operations that are executed within the brackets for each source image. ShowImage shows the current image in a image window.

### 2.	Pre-processing
This can include any operation to the image before background subtraction, commonly converting to grayscale
```javascript
OpenVideo("ants_in_concrete.mov")
{
  GrayScale()
  ShowImage()
}
```

### 3.	Background
This is split into two parts: first defining and/or updating a background image, and second to subtract it from the current image.
With a static background, a static background image will provide better results. Such background image can be produced in BIO using a separate script, gathering a total of at least 10 or 20 sample images across the source material. For example, if the source consists of 1000 images, an interval of 50 would produce 20 sample images. Example of a script to obtain a static background image using median:
```javascript
OpenVideo("ants_in_concrete.mov", Interval=50)
{
  GrayScale()
  AddSeries()
}
GetSeriesMedian()
SaveImage("background.png")
```

Back to the main script, this background image can be loaded before the OpenVideo loop:

```javascript
background = OpenImage("background.png")

OpenVideo("ants_in_concrete.mov")
{
  GrayScale()
  DifferenceAbs(background)
  
  ShowImage()
}
```

### Background alternative
With a varying background (changing light intensity, movement etc), a dynamic background image can be used for better results. The frequency and weight of this update should be carefully tuned for best results.
```javascript
OpenVideo("ants_in_concrete.mov")
{
  GrayScale()
  5:background = UpdateBackground(weight=0.05)
  DifferenceAbs(background)
  
  ShowImage()
}
```

### 4.	Threshold
The threshold function is used for two reasons. This will produce a binary image for cluster detection. The threshold value should be tuned for best results. Alternatively using the threshold function without value will use the Otsu method. However, setting a value normally gives better results.
```javascript
OpenVideo("ants_in_concrete.mov")
{
  GrayScale()
  5:background = UpdateBackground(weight=0.05)
  DifferenceAbs(background)
  Threshold(0.1)
  
  ShowImage()
}
```

### 5.	Detection
Cluster detection is performed on binary images. This operation will generate a Tracker. When this operation is used without parameters, these are automatically defined using a basic statistical algorithm. The resulting parameters can be shown using `ShowTrackInfo()`.
```javascript
OpenVideo("ants_in_concrete.mov")
{
  GrayScale()
  5:background = UpdateBackground(weight=0.05)
  DifferenceAbs(background)
  Threshold(0.1)
  
  CreateClusters()
  
  ShowTrackInfo()
  ShowImage()
}
```

These parameters can then be used in the corresponding operation. Suggested parameters should normally be tuned for best results, by visualising the detected clusters.
```javascript
OpenVideo("ants_in_concrete.mov")
{
  Grayscale()
  5:background = UpdateBackground(weight=0.05)
  DifferenceAbs(background)
  Threshold(0.1)
  
  CreateClusters(MinArea=80, MaxArea=1000)
  
  DrawClusters()
  ShowImage()
}
```

### 6.	Tracking
Tracking is performed on an existing Tracker, which is created in the CreateClusters operation. When this operation is used without parameters, these are automatically defined using a basic statistical algorithm.
```javascript
OpenVideo("ants_in_concrete.mov")
{
  Grayscale()
  5:background = UpdateBackground(weight=0.05)
  DifferenceAbs(background)
  Threshold(0.1)
	
  CreateClusters(MinArea=80, MaxArea=1000)
  CreateTracks()
  
  ShowTrackInfo()
  ShowImage()
}
```

These parameters can again be used in the corresponding operation. Suggested parameters should normally be tuned for best results, by visualising tracking. Tracks can be drawn on the current binary image, but to enable colors, a color image should be used. The current image can be converted to a color image.
```javascript
OpenVideo("ants_in_concrete.mov")
{
  Grayscale()
  5:background = UpdateBackground(weight=0.05)
  DifferenceAbs(background)
  Threshold(0.1)
	
  CreateClusters(MinArea=80, MaxArea=1000)
  CreateTracks(maxMove=20, minActive=3, maxInactive=3)
  
  Color()
  DrawTracks()
  ShowImage()
}
```

### 7.	Visualisation
### 8.	Output



## Links

[User Manual](BioImageOperation%20manual.pdf)

[Script Manual](BioImageOperation%20script.pdf)

See [joostdefolter.info](http://joostdefolter.info/ant-research) for more info and binaries

For support and discussion, please use the [Image.sc forum](https://forum.image.sc) and post to the forum with the tag 'BioImageOperation'.
