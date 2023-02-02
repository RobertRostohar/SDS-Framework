# Copyright (c) 2022 Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# SDS I/O TCP

import sys

import os.path as path
import socket

class sdsio_manager:
  def __init__(self):
    self.stream_identifier = 0
    self.stream_files = {}

  # Open
  def __open(self, mode, name):
    file_index = 0
    response = bytearray()

    if mode == 1:
      # Write mode 
      fname = f"{name}.{file_index}.sds"
      while path.exists(fname) == True:
        file_index = file_index + 1
        fname = f"{name}.{file_index}.sds"
      try:
        f = open(fname, "wb")
        self.stream_identifier += 1
        self.stream_files.update({self.stream_identifier: f})

        command   = 1
        data_size = 0
        response.extend(command.to_bytes(4, byteorder='little'))
        response.extend(self.stream_identifier.to_bytes(4, byteorder='little'))
        response.extend(mode.to_bytes(4, byteorder='little'))
        response.extend(data_size.to_bytes(4, byteorder='little'))
      except Exception as e:
        print(f"Could not open file {fname}. Error: {e}\n")
        return 0

      # if mode == 0:
      #   read mode not supported

      return response


  # Close
  def __close(self, id):
    response = bytearray()

    try:
      self.stream_files.get(id).close()
      self.stream_files.pop(id)
    except Exception as e:
      print(f"Could not close file {self.stream_files.get(id)}. Error: {e}\n")
    return response


  # Write
  def __write(self, id, data):
    response = bytearray()

    try:
      self.stream_files.get(id).write(data)
    except Exception as e:
      print(f"Could not write to file {self.stream_files.get(id)}. Error: {e}\n")
    return response


  # Execute request
  def execute_request (self, request_buf):
    response = bytearray()

    command    = int.from_bytes(request_buf[0:4],   'little')
    sdsio_id   = int.from_bytes(request_buf[4:8],   'little')
    argument   = int.from_bytes(request_buf[8:12],  'little')
    data_size  = int.from_bytes(request_buf[12:16], 'little')
    data       = request_buf[16:16 + data_size]

    # Open
    if command == 1:
      response = self.__open(argument, data.decode('utf-8').split("\0")[0])
    # Close 
    elif command == 2:
      self.__close(sdsio_id) 
    # Write
    elif command == 3:
      self.__write(sdsio_id, data)
    # Invalid command
    else:
      print(f"Invalid command: {command}")
    return response


# Start TCP Server
def start_server ():
  #Get local IP
  if sys.platform == "darwin":
    ip = socket.gethostbyname(socket.gethostname() + '.local')
  else:
    ip = socket.gethostbyname(socket.gethostname())

  print(f"Start TCP Server.\n")
  print(f"  Server IP: {ip}\n")
  port = 5050
  # Create TCP socket (port 5000)
  sock_listening = socket.socket(socket.AF_INET,     # Internet
                                 socket.SOCK_STREAM) # TCP
  sock_listening.bind((ip, port))
  sock_listening.listen()
  return sock_listening

# main
def main():
  stream_buf_cnt    = 0

  header_size       = 16
  header_buf        = bytearray()

  request_buf_size  = 0
  request_buf       = bytearray()
 
  manager = sdsio_manager()

  sock_listening = start_server()
  sock, addr = sock_listening.accept()
  print(f"Device connected.\n")
  print(f"  Device IP: {addr}\n")

  while True:
    try:
      # Read new stream data
      stream_buf = sock.recv(8192)
      if not stream_buf:
        # Close socket
        sock.close()
        print("Device disconnected.\n")

        # Wait device to connect
        sock, addr = sock_listening.accept()
        print(f"Device connected.\n")
        print(f"  Device IP: {addr}\n")
    except Exception as e: 
      print(f"Socket recv error: {e}\n")
      sys.exit(1)
    except KeyboardInterrupt:
      print("\nExit\n")
      sys.exit(0)

    stream_buf_cnt = 0

    while stream_buf_cnt < len(stream_buf):
      
      if request_buf_size == 0:
        # Request buffer is empty. Get new request
        cnt = header_size - len(header_buf)
        if cnt > (len(stream_buf) - stream_buf_cnt):
          cnt = len(stream_buf) - stream_buf_cnt
        header_buf.extend(stream_buf[stream_buf_cnt: stream_buf_cnt + cnt])
        stream_buf_cnt += cnt

        if len(header_buf) != header_size:
          # Header not complete. Read new data
          break
        else:
          # New request
          request_buf = bytearray()
          request_buf.extend(header_buf)
          request_buf_size = int.from_bytes(header_buf[12: 16], 'little') + header_size
          # Clear Header buffer
          del header_buf[0:]

      cnt = request_buf_size - len(request_buf)
      if cnt > (len(stream_buf) - stream_buf_cnt):
        # Not all data is yet available.
        cnt = len(stream_buf) - stream_buf_cnt

      # Copy request data
      request_buf.extend(stream_buf[stream_buf_cnt : stream_buf_cnt + cnt])

      # Update data count
      stream_buf_cnt += cnt

      if len(request_buf) == request_buf_size:
        # Whole request is prepared. Execute request.
        response = manager.execute_request(request_buf)

        # Reset request buffer size. New request can now be processed.
        request_buf_size = 0

        # Send response
        if response:
          try:
            sock.send(bytes(response))
          except socket.error as e: 
            print(f"Socket send error: {e}\n")
            sys.exit(1)

# main
if __name__ == '__main__':
  try:
    print("Press Ctrl+C to exit.\n")
    main()
  except KeyboardInterrupt:
    print("\nExit\n")
    sys.exit(0)

