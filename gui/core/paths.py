import os
import inspect

CONFIG_DIR = os.path.join(os.path.join(os.getcwd(), 'config'))
MAIN_CONFIG_PATH = os.path.join(CONFIG_DIR, 'main.conf')
DEFAULT_DEVICE_CONFIG_PATH = os.path.join(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))), 'default_config.xml')
LOCAL_ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))), '../../eagle/')
LOCAL_BIN_DIR = os.path.join(LOCAL_ROOT_DIR, 'build/bin/')
DEFAULT_REMOTE_ROOT_DIR = '/home/odroid/ProjectEagle/eagle/'


if not os.path.isdir(CONFIG_DIR):
    os.mkdir(CONFIG_DIR)


def get_device_config_path(device):
    return os.path.join(CONFIG_DIR, 'dev%d_config.xml' % device.index)


def get_intrinsic_calibration_dir(device):
    directory = os.path.join(CONFIG_DIR, 'calibration/intrinsic/dev%d' % device.index)
    if not os.path.isdir(os.path.join(CONFIG_DIR, 'calibration')):
        os.mkdir(os.path.join(os.path.join(CONFIG_DIR, 'calibration')))
    if not os.path.isdir(os.path.join(CONFIG_DIR, 'calibration/intrinsic')):
        os.mkdir(os.path.join(os.path.join(CONFIG_DIR, 'calibration/intrinsic')))
    if not os.path.isdir(directory):
        os.mkdir(directory)
    return directory


def get_extrinsic_calibration_dir(dev1, dev2):
    directory = os.path.join(CONFIG_DIR, 'calibration/extrinsic/dev%d_%d' % (dev1.index, dev2.index))
    dir_dev1 = os.path.join(directory, 'dev%d' % dev1.index)
    dir_dev2 = os.path.join(directory, 'dev%d' % dev2.index)
    if not os.path.isdir(os.path.join(CONFIG_DIR, 'calibration')):
        os.mkdir(os.path.join(os.path.join(CONFIG_DIR, 'calibration')))
    if not os.path.isdir(os.path.join(CONFIG_DIR, 'calibration/extrinsic')):
        os.mkdir(os.path.join(os.path.join(CONFIG_DIR, 'calibration/extrinsic')))
    if not os.path.isdir(directory):
        os.mkdir(directory)
    if not os.path.isdir(dir_dev1):
        os.mkdir(dir_dev1)
    if not os.path.isdir(dir_dev2):
        os.mkdir(dir_dev2)
    return directory, dir_dev1, dir_dev2


def get_remote_bin_dir(device):
    return os.path.join(device.remote_root_dir, 'build/bin/')


def get_remote_config_file(device):
    return os.path.join(device.remote_root_dir, 'config/config.xml')


def get_remote_config_dir(device):
    return os.path.join(device.remote_root_dir, 'config/')

def get_remote_output_dir(device):
    return os.path.join(device.remote_root_dir, 'output/')
