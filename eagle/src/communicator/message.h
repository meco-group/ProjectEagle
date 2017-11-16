#ifndef MESSAGE_H
#define MESSAGE_H

#include <zyre.h>
#include <vector>
#include <iostream>

namespace eagle {

/* Low level packing and unpacking
 * Force 32 bit unsigned integer types for the packing as the size of size_t may differ depending on the architecture (32 vs 64 bit).
 */
typedef uint32_t communicator_size_t;

class Message {

 private:
  std::string _peer;
  void* _buffer;
  size_t _buffer_size;
  unsigned int _buffer_index;

 public:

  Message() {

  }

  Message(zmsg_t* msg, std::string& peer) {
    _peer = peer;
    zframe_t* frame = zmsg_first(msg);
    _buffer = zframe_data(frame);
    _buffer_size = zframe_size(frame);
    _buffer_index = 0;
  }

  std::string peer() {
    return _peer;
  }

  size_t size() {
    return _buffer_size;
  }

  void read(int n_frames, std::vector<void*>& frames, std::vector<size_t>& sizes) {
    bool check_size = (sizes.size() == frames.size());
    if (!check_size) {
      sizes.resize(frames.size(), 0);
    }
    for (int k = 0; k < n_frames; k++) {
      if (k >= frames.size()) {
        break;
      }
      communicator_size_t size;
      if (available()) {
        memcpy(&size, ((uint8_t*)_buffer) + _buffer_index, sizeof(communicator_size_t));
        if (check_size && sizes[k] != 0 && sizes[k] != size) {
          std::cout << "Given size (" << sizes[k] <<
                    ") does not match received frame size (" <<
                    size << ")!" << std::endl;
          sizes[k] = (sizes[k] < size) ? sizes[k] : size;
        } else {
          sizes[k] = size;
        }
        _buffer_index += sizeof(communicator_size_t);
        memcpy(frames[k], ((uint8_t*)_buffer) + _buffer_index, sizes[k]);
        _buffer_index += sizes[k];
      } else {
        sizes[k] = 0;
      }
    }
  }

  void read(void* frame) {
    std::vector<void*> frames = {frame};
    std::vector<size_t> sizes = {0};
    read(1, frames, sizes);
  }

  void dump_frame() {
    communicator_size_t size;
    memcpy(&size, ((uint8_t*)_buffer) + _buffer_index, sizeof(communicator_size_t));
    _buffer_index += sizeof(communicator_size_t);
    _buffer_index += size;
  }

  size_t framesize() {
    communicator_size_t size;
    memcpy(&size, ((uint8_t*)_buffer) + _buffer_index, sizeof(communicator_size_t));
    return size;
  }

  bool available() {
    return (_buffer_index < _buffer_size);
  }

  unsigned int buffer_index() {
    return _buffer_index;
  }

  size_t buffer_size() {
    return _buffer_size;
  }

};

};

#endif //MESSAGE_H

