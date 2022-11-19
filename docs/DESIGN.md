# Class Diagrams
## data Classes
```plantuml
class TelemetrySample {
	+gpt::CombinedSample gpSamp
	+size_t lap
	+size_t sector
	+double lapTimeOffset
}

class TelemetrySeeker {
	+void next();
	+void seekToIdx(size_t idx)
	+void seekToTime(double timeOffset)
	+void seekToLap(size_t lap)
	+size_t seekedIdx()
	-size_t seekedIdx_
	-std::shared_ptr<std::vector<TelemetrySample>> samples_
}

class TelemetrySource {
	+TelemetrySource(std::shared_ptr<std::vector<CombinedSample>> samples,std::shared_ptr<TelemetrySeeker> seeker)
	+TelemetrySample at(size_t idx)
	+size_t seekedIdx()
	+size_t size()
	-std::shared_ptr<std::vector<CombinedSample>> samples_
	-std::shared_ptr<TelemetrySeeker> seeker_
}

class VideoSource {
	+VideoSource(cv::VideoCapture vc,std::shared_ptr<TelemetrySeeker> seeker)
	+cv::Mat getFrame(size_t idx)
	+size_t seekedIdx()
	+size_t frameCount()
	-cv::VideoCapture videoCapture_
	-std::shared_ptr<TelemetrySeeker> seeker_
}

TelemetrySample -- TelemetrySource
TelemetrySample -- TelemetrySeeker
TelemetrySeeker -- TelemetrySource
TelemetrySeeker -- VideoSource

```

## graphics Classes
```plantuml
class RenderedObject {
	+cv::Mat getImage()
	+void render(cv::Mat img, int x, int y, sc::Size size)
	+int getNativeWidth()
	+int getNativeHeight()
	+cv::Size getNativeSize()
	+cv::Size getScaledSizeFromTargetHeight(int targetHeight)
	+void setVisible(bool visible)
	+bool isVisible()
	+void setBoundingBoxVisible(bool visible)
	+bool isBoundingBoxVisible()
	#cv::Mat outImg_
	#bool visible_
	#bool boundingBoxVisible_
}

class TelemetryObject {
	+void addSource(TelemetrySource tSrc)
	+TelemetrySource getSource(size_t idx)
	+size_t sourceCount()
	#std::vector<TelemetrySrouce> sources_;
}

class TextObject {
	+void setText(string text)
	+void setFontFace(int fontFace)
	+void setScale(double scale)
	+void setColor(cv::Scalar color)
	+void setThickness(int thickness)
	-string text_
	-int fontFace_
	-double fontScale_
	-cv::Scalara fontColor_
	-int fontThickness_
}

class VideoObject {
	+void setSource(VideoSource vSrc)
	+VideoSource getSource()
	#VideoSource source_;
}

class TrackMapObject {
	+void init(size_t trackStartIdx, size_t trackFinishIdx)
	+void setDotColor(size_t sourceIdx, cv::Scalar color)
	-cv::Mat trackOutlineImg_
}

class FrictionCircleObject {
	+void setTailLength(size_t length)
	-cv::Mat circleOutlineImg_
	-size_t taileLength_
}

class LapTimerObject {
	+void init(size_t lapStartIdx, size_t lapEndIdx)
}

class SpeedometerObject {
}

class TelemetryPrintoutObject {
	+void setFontFace(int fontFace)
	+void setFontColor(cv::Scalar color)
	-int fontFace_
	-cv::Scalara fontColor_
}

RenderedObject <|-- TelemetryObject
RenderedObject <|-- VideoObject
RenderedObject <|-- TextObject
TelemetryObject <|-- TrackMapObject
TelemetryObject <|-- FrictionCircleObject
TelemetryObject <|-- LapTimerObject
TelemetryObject <|-- SpeedometerObject
TelemetryObject <|-- TelemetryPrintoutObject

TelemetrySource -- TelemetryObject
VideoSource -- VideoObject
```

## TBD Classes
```plantuml

class Track {
	+string name
	+gpt::CoordLL start
	+gpt::CoordLL finish
}

class Session {
	-Track track
	-string[] videoFilepaths

	+addVideo(string filepath)
	+size_t videoCount()
	+loadVideo()
}

```
