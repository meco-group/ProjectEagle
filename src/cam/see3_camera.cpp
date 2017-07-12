#include "cam/see3_camera.h"
#include <bitset>

See3Camera::See3Camera(int device) :
    V4L2Camera(device)
{
    // format(2688, 1520, V4L2_PIX_FMT_Y16);
    format(1280, 720, V4L2_PIX_FMT_Y16);
    // format(1920, 1080, V4L2_PIX_FMT_Y16);
    setBrightness(7);
}

int See3Camera::process_buffer(cv::Mat &img)
{
    readBayerIR(img);
    return 0;
}

bool See3Camera::readBayerIR(cv::Mat &img)
{
    img.create(getHeight()/2,getWidth()/2,CV_8UC3);
    cv::Mat IR = cv::Mat(getHeight()/2,getWidth()/2,CV_8UC1);

    int nRows = img.rows;
    int nCols = img.cols;

    int i,j;
    uint8_t* p_row_bgr;
    uint16_t* p_row_bayer1 = (uint16_t*)getBuffer();
    uint16_t* p_row_bayer2 = p_row_bayer1 + getWidth();
    uint16_t blue_16, green_16, red_16;

#ifndef SEE3CAMERA_IR
    uint8_t* p_row_ir;
    uint16_t ir_16;
#endif //SEE3CAMERA_IR

    for( i = 0; i < nRows; ++i)
    {
        p_row_bgr = img.ptr<uint8_t>(i);
        p_row_bayer1 = (uint16_t*)getBuffer() + i*2*getWidth();
        p_row_bayer2 = p_row_bayer1 + getWidth();

#ifndef SEE3CAMERA_IR
        p_row_ir = IR.ptr<uint8_t>(i);
#endif //SEE3CAMERA_IR

        for ( j = 0; j < nCols; ++j)
        {
            blue_16 = p_row_bayer1[0];
            green_16 = p_row_bayer1[1];
            red_16 = p_row_bayer2[1];

            p_row_bgr[0] = (uint8_t)((blue_16>0xff)?0xff:blue_16);
            p_row_bgr[1] = (uint8_t)((green_16>0xff)?0xff:green_16);
            p_row_bgr[2] = (uint8_t)((red_16>0xff)?0xff:red_16);

#ifndef SEE3CAMERA_IR
            ir_16 = p_row_bayer2[0];
            p_row_ir[0] = (uint8_t)((ir_16>0xff)?0xff:ir_16);
            p_row_ir++;
#endif //SEE3CAMERA_IR

            p_row_bgr += 3;
            p_row_bayer1 += 2;
            p_row_bayer2 += 2;
        }
    }

    return true;
}

bool See3Camera::setResolution(const std::vector<int>& resolution){
    std::vector<std::vector<int> > possible_resolutions = {{672, 380}, {1280, 720}, {1920, 1080}, {2688, 1520}};
    bool check;
    for (int k=0; k<possible_resolutions.size(); k++){
        check = true;
        for (int i=0; i<2; i++){
            check &= (possible_resolutions[k][i] == resolution[i]);
        }
        if (check) {
            break;
        }
    }
    if (!check){
        std::cout << "Wrong resolution set!" << std::endl;
        return false;
    }

    format(resolution[0], resolution[1], V4L2_PIX_FMT_Y16);
    return true;
}

bool See3Camera::setBrightness(int brightness){
    if (brightness < 0 || brightness > 40){
        std::cout << "Brighness should lie between 0 and 40!" << std::endl;
        return false;
    }

    return v4l2_set_brightness(brightness) == 0;
}

bool See3Camera::setExposure(int exposure){
    if (exposure < 1 || exposure > 10000){
        std::cout << "Exposure should lie between 1 and 10000!" << std::endl;
        return false;
    }

    return v4l2_set_exposure(exposure) == 0;
}

bool See3Camera::setISO(int iso){
    if (iso < 100 || iso > 2500){
        std::cout << "ISO should lie between 100 and 2500!" << std::endl;
        return false;
    }

    return v4l2_set_iso(iso) == 0;
}

bool See3Camera::read(cv::Mat &img)
{
    V4L2Camera::read(img);
    // flip
    cv::flip(img, img, -1);
    // crop a little bit
    int width = img.size().width;
    int height = img.size().height;
    double crop_ratio = .85;
    cv::Rect roi(0.5*(width-crop_ratio*width), 0.5*(height-crop_ratio*height), crop_ratio*width, crop_ratio*height);
    img = img(roi);
    return true;
}
