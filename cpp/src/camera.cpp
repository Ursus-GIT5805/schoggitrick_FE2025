#include <iostream>

#include <opencv2/imgproc.hpp>
#include <raspicam/raspicam_cv.h>
#include <opencv2/core/mat.hpp>

#include "imgproc.cpp"

class Camera {
private:
	raspicam::RaspiCam_Cv cam;

// #ifdef DEBUG
    cv::VideoWriter writer;
// #endif

public:
	Camera(){
		this->cam.set( cv::CAP_PROP_MODE, 2 );
		cam.set( cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH );
		cam.set( cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT );
		cam.set(cv::CAP_PROP_EXPOSURE, 5);
		cam.set(cv::CAP_PROP_AUTO_EXPOSURE, -1.0);


		if ( !cam.open() ) {
			std::cerr << "Error opening camera!" << std::endl;
			exit(1);
		}

// #ifdef DEBUG
		writer.open("appsrc ! \
videoconvert ! \
video/x-raw,format=YUY2,width=" + std::to_string(CAM_WIDTH) + ",height=" + std::to_string(CAM_HEIGHT) + ",framerate=30/1 ! \
jpegenc ! \
rtpjpegpay ! \
udpsink host=laetitia port=5000",
					0,
					(double)30, // fps
					cv::Size(CAM_WIDTH, CAM_HEIGHT),
					true); // color

		if (!writer.isOpened()) {
			std::cerr << "Can't create video writer!" << std::endl;
			exit(1);
		}
// #endif
	}

	~Camera() {
		this->cam.release();
	}

	// Grabs and retrieves the current frame
	cv::Mat read() {
		cv::Mat mat;

		this->cam.grab();
		this->cam.retrieve(mat);

		return mat;

		// cv::Mat out;
		// cv::cvtColor(mat, out, cv::COLOR_RGB2BGR);

		// return out;
	}

// #ifdef DEBUG
	void stream_frame(cv::Mat frame) {
		writer << frame;
	}
// #endif
};
