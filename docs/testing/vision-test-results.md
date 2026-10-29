# Vision Test Results

## 1. Objective

This document records the testing of the computer-vision modules used in the vision-based automated conveyor sorting system.

The tests cover live video capture from a mobile phone IP camera, colour classification, shape classification and detection reliability under different conditions.

## 2. Test Environment

| Parameter | Configuration |
|---|---|
| Camera source | Mobile phone used as an IP camera |
| Video acquisition | OpenCV `VideoCapture` |
| Image processing | Python, OpenCV and NumPy |
| Camera URL | Configured in `vision/camera_config.py` |
| Colour detection script | `vision/color_detection.py` |
| Shape detection script | `vision/shape_detection.py` |
| ESP32 integration | Pending implementation and validation |

Do not publish a camera URL if it reveals a service you have exposed beyond your local network. The local development address may be documented as an example.

## 3. Colour Detection Results

The current configuration uses four active colours: green, blue, orange and yellow. Red is available as an alternative in the HSV configuration.

| Test ID | Object colour | Expected result | Actual result | Status |
|---|---|---|---|---|
| COL-01 | Green | GREEN | Record after testing | Not tested |
| COL-02 | Blue | BLUE | Record after testing | Not tested |
| COL-03 | Orange | ORANGE | Record after testing | Not tested |
| COL-04 | Yellow | YELLOW | Record after testing | Not tested |
| COL-05 | Red, after configuration change | RED | Record after testing | Not tested |
| COL-06 | No configured colour in sufficient quantity | UNKNOWN | Record after testing | Not tested |
| COL-07 | Multiple colours in the frame | Colour with the highest matching pixel count, if above threshold | Record after testing | Not tested |

### Colour Test Procedure

1. Start the IP camera application on the mobile phone.
2. Confirm the stream URL in `vision/camera_config.py`.
3. Run `python vision/color_detection.py`.
4. Place one coloured object in front of the camera against a contrasting background.
5. Record the displayed classification and pixel counts.
6. Repeat each test at different distances and under different lighting.
7. Record incorrect detections and adjust HSV limits or the minimum pixel threshold if necessary.

## 4. Shape Detection Results

The current shape detector uses image contours and geometric approximation to identify basic shapes.

| Test ID | Test object | Expected classification | Actual result | Status |
|---|---|---|---|---|
| SHP-01 | Triangle | TRIANGLE | Record after testing | Not tested |
| SHP-02 | Square | SQUARE | Record after testing | Not tested |
| SHP-03 | Rectangle | RECTANGLE | Record after testing | Not tested |
| SHP-04 | Circle | CIRCLE | Record after testing | Not tested |
| SHP-05 | Irregular object | OTHER or another observed result | Record after testing | Not tested |
| SHP-06 | No clearly identifiable object | No shape detected or no accepted contour | Record after testing | Not tested |

### Shape Test Procedure

1. Run `python vision/shape_detection.py`.
2. Place one object at a time in the camera's field of view.
3. Observe the bounding box, contour and displayed classification.
4. Repeat the test with different object orientations and distances.
5. Record errors caused by shadows, reflections, poor contrast or background edges.

The detector's output is an estimate based on contour geometry. It should not be assumed correct without comparing it against the actual object.

## 5. Detection Performance Record

Use this table to measure repeatability under controlled conditions.

| Configuration | Trials | Correct detections | Incorrect detections | Missed detections |
|---|---:|---:|---:|---:|
| Colour — normal lighting | — | — | — | — |
| Colour — low lighting | — | — | — | — |
| Colour — different distances | — | — | — | — |
| Shape — normal lighting | — | — | — | — |
| Shape — different orientations | — | — | — | — |
| Shape — contrasting background | — | — | — | — |

Once results are collected, calculate detection accuracy as:

**Detection accuracy (%) = (Correct detections ÷ Total evaluated trials) × 100**

Define the trial conditions consistently. A missed detection should be counted separately from a wrong classification.

## 6. Observations and Issues

Record observed issues here after testing.

| Observation | Likely factor to investigate | Corrective action | Retest status |
|---|---|---|---|
| Incorrect colour classification | Lighting or HSV limits | Tune the relevant HSV range | Not tested |
| Colour not detected | Pixel threshold or object distance | Review `MIN_PIXELS` | Not tested |
| Incorrect shape classification | Contour quality or approximation settings | Adjust image-processing parameters | Not tested |
| Unstable camera stream | Network or camera application | Check stream reliability | Not tested |

These are possible causes to investigate, not confirmed findings from testing.

## 7. Vision-to-ESP32 Integration

The vision scripts currently process camera frames independently. The proposed Wi-Fi classification endpoint is not yet implemented in the supplied ESP32 firmware.

| Test ID | Integration test | Status |
|---|---|---|
| INT-01 | Python sends a colour classification over Wi-Fi | Blocked — endpoint pending |
| INT-02 | Python sends a shape classification over Wi-Fi | Blocked — endpoint pending |
| INT-03 | ESP32 validates received classifications | Blocked — firmware handler pending |
| INT-04 | ESP32 triggers the intended sorting action | Blocked — end-to-end integration pending |

Update these statuses after the matching ESP32 endpoint has been implemented and tested.

## 8. Conclusion

Complete this section after testing. Summarise the observed colour and shape detection performance, operating conditions, known limitations and corrective actions.

Do not claim that the vision system is fully validated until repeatable test results have been recorded and the intended vision-to-ESP32 sorting workflow has been tested on the physical conveyor.
