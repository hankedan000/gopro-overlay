# Class Diagrams
## data Classes
<!--
@startuml plantuml_imgs/dataClasses

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

@enduml
-->
![](plantuml_imgs/dataClasses.png)

<!--
@startuml plantuml_imgs/dataClasses2

class DetectionGate {
	+DetectionGate(cv::Vec2d a, cv::Vec2d b)
	+bool detect(cv::Vec2d c1, cv::Vec2d c2)

	-cv::Vec2d a;
	-cv::Vec2d b;
}

class Sector {
	+std::string name()
	+DetectionGate entry()
	+DetectionGate exit()

	-std::string name_
	-DetectionGate entry_
	-DetectionGate exit_
}

class Track {
	+void setStart(DetectionGate start)
	+DetectionGate getStart()
	+void setFinish(DetectionGate finish)
	+DetectionGate getFinish()

	+void addSector(Sector s)
	+void removeSector(Sector *s)
	+void removeSector(size_t idx)
	+Sector *getSector(size_t idx)
	+size_t sectorCount()

	+size_t pathCount()
	+cv::Vec2d getPathPoint(size_t idx)
	+DetectionGate getNearestDetectionGate(cv::Vec2d p, double width_meters)
	+cv::Vec2d findClosestPoint(cv::Vec2d p)
	+std::pair<cv::Vec2d, size_t> findClosestPointWithIdx(cv::Vec2d p)

	-DetectionGate start_
	-DetectionGate finish_
	-std::vector<Sector> sectors_
	-std::vector<cv::Vec2d> path_
}

DetectionGate -- Sector
DetectionGate -- Track
Sector -- Track

@enduml
-->
![](plantuml_imgs/dataClasses2.png)

## graphics Classes
<!--
@startuml plantuml_imgs/graphicsClasses

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

@enduml
-->
![](plantuml_imgs/graphicsClasses.png)

## TBD Classes
<!--
@startuml plantuml_imgs/tbdClasses

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

@enduml
-->
![](plantuml_imgs/tbdClasses.png)
