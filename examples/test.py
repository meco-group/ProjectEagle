import cv2
import time
import numpy

camera = cv2.VideoCapture(0)
camera.set(cv2.CAP_PROP_FRAME_WIDTH,640)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT,480)

nof = 100

print "Chrono start!"
begin = time.time()
frame = 0
while(frame < nof):
	retval, im = camera.read()
	retval, buffer = cv2.imencode(".jpg",im,[int(cv2.IMWRITE_JPEG_QUALITY), 20]);
	frame += 1
	
end = time.time()
print "Chrono stop!"

duration = end-begin
fps = nof/duration

print "Duration: %f[s]" % duration
print "FPS: %f" % fps
