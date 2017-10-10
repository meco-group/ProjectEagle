#ifndef CONFIG_HELPER_H
#define CONFIG_HELPER_H

#include <opencv/cv.hpp>
#include <iostream>
#include <map>
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

namespace eagle {
    void dump_matrices(const std::string& xml_path, std::map<std::string, cv::Mat>& matrices) {
        using namespace rapidxml;
        std::ifstream ifile(xml_path);
        std::vector<char> buffer((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
        buffer.push_back('\0');
        ifile.close();
        xml_document<> doc;
        doc.parse<0>(&buffer[0]);
        xml_node<>* camera_node = doc.first_node("opencv_storage")->first_node("camera");
        std::map<std::string, cv::Mat>::iterator it;
        xml_node<>* node;
        std::string data;
        std::vector<double> mat_vec;
        for (it=matrices.begin(); it != matrices.end(); it++) {
            mat_vec.assign((double*)(it->second).datastart, (double*)(it->second).dataend);
            data = "";
            for (int k=0; k<mat_vec.size(); k++) {
                data += std::to_string(mat_vec[k]);
                data += " ";
            }
            node = camera_node->first_node((it->first).c_str());
            char* node_name = doc.allocate_string("data");
            char* node_value = doc.allocate_string(data.c_str());
            node->remove_node((node->first_node("data")));
            node->append_node(doc.allocate_node(node_element, node_name, node_value));
        }
        std::ofstream ofile(xml_path);
        ofile << "<?xml version=\"1.0\"?>\n";
        ofile << doc;
        ofile.close();
    }
    
    void set_calibrated(const std::string& xml_path, bool value) {
        using namespace rapidxml;
        std::ifstream ifile(xml_path);
        std::vector<char> buffer((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
        buffer.push_back('\0');
        ifile.close();
        xml_document<> doc;
        doc.parse<0>(&buffer[0]);
        xml_node<>* calibrator_node = doc.first_node("opencv_storage")->first_node("calibrator");
        calibrator_node->remove_node((calibrator_node->first_node("calibrated")));
        calibrator_node->append_node(doc.allocate_node(node_element, "calibrated", (value) ? "1" : "0"));
        std::ofstream ofile(xml_path);
        ofile << "<?xml version=\"1.0\"?>\n";
        ofile << doc;
        ofile.close();
    }
    
    void set_integrated(const std::string& xml_path, bool value) {
        using namespace rapidxml;
        std::ifstream ifile(xml_path);
        std::vector<char> buffer((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
        buffer.push_back('\0');
        ifile.close();
        xml_document<> doc;
        doc.parse<0>(&buffer[0]);
        xml_node<>* calibrator_node = doc.first_node("opencv_storage")->first_node("calibrator");
        calibrator_node->remove_node((calibrator_node->first_node("integrated")));
        calibrator_node->append_node(doc.allocate_node(node_element, "integrated", (value) ? "1" : "0"));
        std::ofstream ofile(xml_path);
        ofile << "<?xml version=\"1.0\"?>\n";
        ofile << doc;
        ofile.close();
    }
};

#endif // CONFIG_HELPER_H
