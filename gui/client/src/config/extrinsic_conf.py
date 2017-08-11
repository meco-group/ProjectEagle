import os
import xml.etree.cElementTree as ET

from io import BytesIO
from xml.dom import minidom
from xml.sax.saxutils import unescape


class ExtrinsicConfig:
    def __init__(self):
        pass

    def write(self, target):
        root = ET.Element('opencv_storage')

        T = ET.SubElement(root, 'rotation_matrix')
        T.set('type_id', 'opencv-matrix')
        rows = ET.SubElement(T, 'rows')
        rows.text = str(3)
        cols = ET.SubElement(T, 'cols')
        cols.text = str(3)
        dt = ET.SubElement(T, 'dt')
        dt.text = 'd'
        data = ET.SubElement(T, 'data')
        data.text = '1. 0. 0. 0. 1. 0. 0. 0. 1.'

        T = ET.SubElement(root, 'translation_matrix')
        T.set('type_id', 'opencv-matrix')
        rows = ET.SubElement(T, 'rows')
        rows.text = str(3)
        cols = ET.SubElement(T, 'cols')
        cols.text = str(1)
        dt = ET.SubElement(T, 'dt')
        dt.text = 'd'
        data = ET.SubElement(T, 'data')
        data.text = '0. 0. 0.'

        tree = ET.ElementTree(root)

        f = BytesIO()
        tree.write(f, encoding='utf-8', xml_declaration=True)

        result = minidom.parseString(f.getvalue())
        result = unescape(result.toprettyxml(indent="\t"), {"&quot;": '"'})

        with open(target, "w") as text_file:
            text_file.write(result)
