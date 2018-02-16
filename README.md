# Pose-Estimation
Limited-rotation pose-estimation of a pre-labeled marker.

I have designed a blue rectangular-marker with an L-shaped red-fill inside it. I have labelled the outer-corners of rectangle as 1-2-3-4. The above code, will work only if the rotation is less than 90 degrees in yaw, pitch or roll when it is oriented in its home-positions;
The L-shape has no significance, except for the user to realize 1-2-3-4(home-position) when giving an input signal to the camera.

The above code computes rotation and translation to significantly good accuracy and outputs only translation. 

This code could be improvised to create a robust assymetrical-marker pose-estimator, which could be used in estimating planar distance of different points in the world from the marker.

STEPS INVOLVED:-

1. Marker segmentation from background.
2. Extracting the corners of the marker.
3. Labelling the corners of the marker corresponding to the world-cordinates
4. Calculation of Rotation and translation using solvePnP
