import os


class PathFinder:
    # Folder definitions
    PROJECT_FOLDER = "ProjectEagle"
    # Note: the [] notation indicates a string that will be filled in later
    #       the order of the folder key order is important. A value can only use keys that have been
    #       defined before it already
    #       special keys are:
    #       [PROJECT_FOLDER]    = the root of the project
    #       [DEVICE_NAME]       = the name of the device
    __FOLDER_KEY_ORDER = ["[CONFIG_FOLDER]", "[DEVICE_FOLDER]", "[INTRINSIC_SNAP_FOLDER]", "[EXTRINSIC_SNAP_FOLDER]"]
    __FOLDERS = {
        "[CONFIG_FOLDER]": "[PROJECT_FOLDER]/src/client/config/",
        "[DEVICE_FOLDER]": "[CONFIG_FOLDER]/devices/[DEVICE_NAME]/",
        "[INTRINSIC_SNAP_FOLDER]": "[DEVICE_FOLDER]/intrinsic_snaps/",
        "[EXTRINSIC_SNAP_FOLDER]": "[DEVICE_FOLDER]/extrinsic_snaps/",
    }

    # File definitions
    __FILES = {
        "[MAIN_CONFIG]": "[CONFIG_FOLDER]/main.conf",
        "[DETECTOR_CONFIG]": "[CONFIG_FOLDER]/detector.yml",
        "[DEVICE_CONFIG]": "[DEVICE_FOLDER]/config.xml",

        "[INTRINSIC_CALIBRATION]": "[DEVICE_FOLDER]/calibration.xml",
        "[INTRINSIC_CONFIG]": "[DEVICE_FOLDER]/intrinsic_conf.xml",
        "[INTRINSIC_SNAP_LIST]": "[DEVICE_FOLDER]/intrinsic_snaps.xml",

        "[EXTRINSIC_CALIBRATION]": "[DEVICE_FOLDER]/extrinsic.xml",
        "[EXTRINSIC_CONFIG]": "[DEVICE_FOLDER]/extrinsic_conf.xml",
        "[EXTRINSIC_SNAP_LIST]": "[DEVICE_FOLDER]/extrinsic_snaps.xml",

        "[BACKGROUND]": "[DEVICE_FOLDER]/background.png",
    }

    def __init__(self, device, remote=False):
        self.device = device
        self.remote = remote

        # parse file and folder names
        self.__folders = self.__FOLDERS.copy()
        for key in self.__FOLDER_KEY_ORDER:
            self.__folders[key] = self.__parse_path(self.__folders[key])

        self.__files = self.__FILES.copy()
        for key in self.__files.keys():
            self.__files[key] = self.__parse_path(self.__files[key])

    def __parse_path(self, path):
        project_folder = self.__get_root()
        for key in self.__FOLDER_KEY_ORDER:
            path = path.replace(key, self.__folders[key])
        path = path.replace("[PROJECT_FOLDER]", project_folder)
        path = path.replace("[DEVICE_NAME]", self.device.name)
        return os.path.normpath(path)

    def __get_root(self):
        if self.remote:
            return os.path.join("/home", self.device.username, self.PROJECT_FOLDER)
        else:
            path = os.getcwd()
            parts = path.split(self.PROJECT_FOLDER)
            res = "/"
            for part in parts[0:-1]:
                res = os.path.join(res, part)
            return os.path.join(res, self.PROJECT_FOLDER)

    def get_path(self, path):
        if path in self.__files.keys():
            path = self.__files[path]
        return self.__parse_path(path)