#### Version 1.7.14
* Improved camera capture: added codec parameter
* OpenCV 4.6.0

#### Version 1.7.13
* Fixed statusbar stats for live capture

#### Version 1.7.12
* Tracking: fine tuned identity charactersistics using match factor
* Status text: show final run time when completed succesfully
* Release image memory on script completion / abort

#### Version 1.7.11
* Video capture: added setting desired frame width and height parameters

#### Version 1.7.10
* Tracking: allow larger track movement depending on length being inactive
* Show (auto) threshold only with debug flag

#### Version 1.7.9
* Fixed bug in cluster and track csv output (ByLabel format)

#### Version 1.7.8
* Improved threshold showing OTSU value as output
* Refactor in greedy algorithm
* Increased annotation font size for hires sources

#### Version 1.7.7
* Added Int / Float image depth type conversions
* Improved ShowImage() handling of divergent image types
* Refactored various image operations

#### Version 1.7.6
* Added debug mode to optical correction

#### Version 1.7.5
* Improved background/median algorithm with light/dark modification
* Improved status text in terms of overall progress
* Added optical correction

#### Version 1.7.4
* Revised output filename generation
* Small fix for pausing in nested Source loop

#### Version 1.7.3
* Added raw distance as output
* Fixed/improved handling numeric paths
* Ensure creating save paths
* Small bug fix in bracket matching

#### Version 1.7.2
* Added dual script format support: Java and Python style now accepted
* Added new Source operation: GetSeriesMean (added example script)
* Added more image series operations
* Reworked tracking, handling merging better and using mean area and major length for ID
* Fixed bug and improved support for video capture (using OpenCapture)
* Added feature to tracking output: projection
* Improved cross-platform compatibility
* Added Hungarian algorithm (work in progress) separating tracking algorithm into separate class
* Added UI overlay to enable progress, syntax highlighting, and integrated benchmarking
* Fixed bug pause skipping remaining operations in loop
* Bug fix / improved error handling including messages shown in console mode

#### Version 1.7.1
* Simplified versioning
* Improved tracking using better ID information
* Added Pause operation
* Added basic script generation
* Reintroduced Totals option for sources
* Improved background subtraction and added min/max image buffer functions
* Renamed weighting function
* Larger points in cluster/track drawing functions
* Make labeling colors better distinguishable
* Redid color tables
* Redid cluster/track output information, added features and advanced features
* Redid log files to handle writing to many files efficiently
* Work-around for reading GoPro videos (known OpenCV issue)
* Bug fix: correctly closing output csv files
* Bug fix: set source sizes after scale/crop
* Bug fix: memory leak in tracking
* Improved error handling, including closing while in operation

#### Version 1.6.440.6
* Converted script help from RTF to MD, also shown in UI

#### Version 1.6.440.5
* Bug fix: correct paths in export
* Bug fix: booleans without value allowed
* Bug fix: correct split export cluster/tracks
* Bug fix: correct visibility order of cluster drawing

#### Version 1.6.440.3
* Bug fix: wildcard file patterns
* Bug fix: display grayscale images
* Bug fix: fixed printing cluster labels
* Bug fix: correct display numbers in window titles
* Handle redraw multiple text/image windows,
* Added dynamic resizing to image windows
* Added fixed size font

#### Version 1.6.440.2:
* Added command-line functionality, running scripts and operation help
* Added functionality providing corresponding text output in console

#### Version 1.6.440.1:
* Major re-release: Now using pure std/C++ and Qt (Qt5) as main gui, enabling potentially cross-platform
* Operation arguments checking is improved, consistently with operation syntax check now also occurring before execution starts
* Improved script instructions / generated manual
* Known issues: SSL not working (used in checking for updates)

#### Version 1.5.440.1:
* Improved tracking: now uses more accurate id information
* Fixed bug in cluster images
* Support OpenCV 4.4.0

#### Version 1.4.420.22:
* Save clusters/tracks: now uses frames and time as well as saving contours, added different output formats
* Fixed various small bugs
* Support OpenCV 4.2.0 (and correct version of openh264)

#### Version 1.4.410.21:
* Fixed memory leak in showing images with stride
* Use time when saving tracks etc
* Proper file modified detection including auto save on run

#### Version 1.4.410.20:
* Fixed bug in videos without length/frames property
* Fixed bug showing images with stride/padding
* Fixed bug not showing images after error occurs
* Minor fixes for video progress
* Support OpenCV 4.1.0

#### Version 1.4.400.10:
* Added saving clusters and tracks by label

#### Version 1.4.400.9: 
* Exposed video API selection

#### Version 1.4.400.8:
* Fixed bugs in image series and cluster drawing
* Simplified stats calculations improving automatic tracking parameter calculation
* Supports OpenCV 4.0.0

#### Version 1.4.341.7:
* Added GPL license information

#### Version 1.4.341.6:
* Further improved automatically finding optimal tracking parameters
* Added 'Check for updates' functionality
* Minor bug fixes

#### Version 1.4.341.5:
* Significantly improved automatically finding optimal tracking parameters
* Draw logarithmic color legend based on input parameters
* Many improvements and bug fixes and updates to the help/manual

#### Version 1.4.341.4:
* Implemented directly writing to video file using H264 encoding

#### Version 1.4.341.3:
* Significant improvement in overall performance

#### Version 1.4.341.2 (OpenCV 3.4.1):
* Implemented all track draw functions including many options
* Improved script argument checking and help
* Uses OpenCV 3.4.1

#### Version 1.4.34.1 (OpenCV 3.4) - This is a major rewrite from C#/Emgu to C++/OpenCV. These are the key points:
* Using OpenCV directly - with full library support and using the latest version
* Real-time processing of HD video source (on standard consumer hardware) due to significant performance improvements
* Direct memory management with superior memory footprint
* Improved error handling and argument checking
* Built-in dynamically-generated script help
* Improved cluster detection: using connected components for accurate cluster sizes
* Very efficient custom image display, for minimal impact on main image processing loop
* Improved installer with pre-requisites detection and download (using WiX)
* Re-branded artwork
