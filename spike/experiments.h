//
// Created by peter on 07/07/17.
//

#ifndef PROJECTEAGLE_TESTS_H
#define PROJECTEAGLE_TESTS_H

#include "cal/board_settings.h"
#include "cal/calibration_settings.h"
#include "cal/calibrator.h"
#include "cam/camera_settings.h"
#include "examples_config.h"
#include "detector.h"


void cam_test();
void cal_test();
void detect_test();
void gen_settings();
void detect_pattern();
void stereo_cal();

#endif //PROJECTEAGLE_TESTS_H
