import os
from io import BytesIO
from xml.dom import minidom

import xml.etree.cElementTree as ET


class CalibrationConfig:
    def __init__(self, device, images, image_list_path="[INTRINSIC_SNAP_LIST]", output_path="[INTRINSIC_CALIBRATION]"):
        self.device = device
        self.images = images
        self.imageListPath = image_list_path
        self.outputPath = output_path

    def write_config(self, target):
        image_count = len(self.images)

        root = ET.Element('opencv_storage')
        cal = ET.Element('CalibrationSettings')
        ic = ET.SubElement(cal, 'Image_Count')
        ic.text = str(image_count)
        src = ET.SubElement(cal, 'Source')
        src.text = self.device.local_path_finder.get_path(self.imageListPath)
        src_type = ET.SubElement(cal, 'Source_Type')
        src_type.text = "STORED"
        brds = ET.SubElement(cal, 'Board_Settings')
        brds.text = self.device.local_path_finder.get_path("[DEVICE_CONFIG]")
        cfar = ET.SubElement(cal, 'Calibrate_FixAspectRatio')
        cfar.text = str(1.0)
        caztd = ET.SubElement(cal, 'Calibrate_AssumeZeroTangentialDistortion')
        caztd.text = '0'
        cfppatc = ET.SubElement(cal, 'Calibrate_FixPrincipalPointAtTheCenter')
        cfppatc.text = '0'
        of = ET.SubElement(cal, 'Output_File')
        of.text = self.device.local_path_finder.get_path(self.outputPath)

        root.append(cal)
        tree = ET.ElementTree(root)

        f = BytesIO()
        tree.write(f, encoding='utf-8', xml_declaration=True)

        result = minidom.parseString(f.getvalue())
        result = result.toprettyxml(indent="\t")

        with open(target, "w") as text_file:
            text_file.write(result)

    def write_image_list(self, target):
        res = '\n'
        for path in self.images:
            res = res + '\t' + path + '\n'

        root = ET.Element("opencv_storage")
        ET.SubElement(root, "images").text = res

        tree = ET.ElementTree(root)

        f = BytesIO()
        tree.write(f, encoding='utf-8', xml_declaration=True)

        result = minidom.parseString(f.getvalue())
        result = result.toprettyxml(indent="\t")
        with open(target, "w") as text_file:
            text_file.write(result)
