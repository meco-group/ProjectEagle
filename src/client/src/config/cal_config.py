import os
from io import BytesIO
from xml.dom import minidom

import xml.etree.cElementTree as ET


class CalibrationConfig:
    def __init__(self, wizard, device=None, output=None, images=None):
        self.wizard = wizard
        if device is None:
            self.device = self.wizard.device
        else:
            self.device = device

        if images is None:
            self.images = self.wizard.file_list.get_selected()
        else:
            self.images = images


        if output is None:
            self.output = self.wizard.device.get_calibration_path()
        else:
            self.output = output

    def write_config(self, target):
        image_count = len(self.images)

        root = ET.Element('opencv_storage')
        cal = ET.Element('CalibrationSettings')
        ic = ET.SubElement(cal, 'Image_Count')
        ic.text = str(image_count)
        src = ET.SubElement(cal, 'Source')
        src.text = os.path.join(self.device.get_root(), self.wizard.IMG_CONFIG)
        src_type = ET.SubElement(cal, 'Source_Type')
        src_type.text = "STORED"
        brds = ET.SubElement(cal, 'Board_Settings')
        brds.text = self.device.get_config_path()
        cfar = ET.SubElement(cal, 'Calibrate_FixAspectRatio')
        cfar.text = str(1.0)
        caztd = ET.SubElement(cal, 'Calibrate_AssumeZeroTangentialDistortion')
        caztd.text = '0'
        cfppatc = ET.SubElement(cal, 'Calibrate_FixPrincipalPointAtTheCenter')
        cfppatc.text = '0'
        of = ET.SubElement(cal, 'Output_File')
        of.text = self.output

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
