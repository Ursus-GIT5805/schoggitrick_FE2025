#include <iostream>

#include <raspicam/raspicam_cv.h>
#include <opencv2/core/mat.hpp>

class Camera {
private:
	raspicam::RaspiCam_Cv cam;

public:
	Camera(){
		this->cam.set( cv::CAP_PROP_MODE, 2 );
		// cam.set( cv::CAP_PROP_FRAME_WIDTH, 2592 );
		// cam.set( cv::CAP_PROP_FRAME_HEIGHT, 1944 );

		if ( !cam.open() ) {
			std::cerr << "Error opening camera!\n";
			exit(1);
		}
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
	}
};
