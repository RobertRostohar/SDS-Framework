from encodings import utf_8
from os import path
import socket
import atexit

sock = socket.socket()

stream_identifier = 0
stream_files = {}

# Command open
def command_open(name, RdWr):
  global stream_identifier
  file_index = 0

  fname = f"{name}.{file_index}.sds"
  while path.exists(fname) == True:
    file_index = file_index + 1
    fname = f"{name}.{file_index}.sds"

  try:
    f = open(fname, "wb")
    stream_identifier += 1
    stream_files.update({stream_identifier: f})
    return stream_identifier
  except:
    print(f"Can not open {fname}")
    return 0

# Command Close
def command_close(id):
  try:
    stream_files.get(id).close()
    stream_files.pop(id)
    return 0x00000000
  except:
    print(f"Can not close {stream_files.get(id)}")
    return 0xFFFFFFFF

# Command Write
def command_write(id, data):
  try:
    stream_files.get(id).write(data)
    return len(data)
  except Exception as err:
    print(f"Can not write to {stream_files.get(id)}")
    print(f"  Error: {err}")
    return 0  

def set_communication ():
  #Get local IP
  udp_ip = socket.gethostbyname(socket.gethostname())
  print(f"{udp_ip}")
  udp_port = 5000
  #Create UDP socket (port 5000)
  sock = socket.socket(socket.AF_INET,    # Internet
                       socket.SOCK_DGRAM) # UDP
  sock.bind((udp_ip, udp_port))
  #sock.settimeout(5)
  return sock

# main
def main():
  response = bytearray()
  request = bytearray()
  packet_idx = 0
  ret = 0

  sock = set_communication()

  while 1:
    #try:
    request, addr = sock.recvfrom(1500)
    idx        = int.from_bytes(request[0:4],  'little')
    command    = int.from_bytes(request[4:8],  'little')
    argument   = int.from_bytes(request[8:12], 'little')
    data_size  = int.from_bytes(request[12:16],'little')
    data       = request[16:16+data_size]

    if idx == 0:
      packet_idx = 0
    
    print(f"Command   {packet_idx}, {idx}")
    if packet_idx == idx:

      print(f"   {packet_idx} {command} {argument} {data_size} {data}")

      # Execute command
      # Open
      if command == 1:
        ret = command_open(data.decode('utf-8').split("\0")[0], argument)
      # Close 
      elif command == 2:
        ret = command_close(argument) 
      # Write
      elif command == 3:
        ret = command_write(argument, data)

      # Prepare response
      response_size = 4
      response[0:4]   = packet_idx.to_bytes(4, byteorder='little')
      response[4:8]   = command.to_bytes(4, byteorder='little')
      response[8:12]  = argument.to_bytes(4, byteorder='little')
      response[12:16] = response_size.to_bytes(4, byteorder='little')
      response[16:20] = ret.to_bytes(4, byteorder='little')

    # Send Response
    print("Response")
    print(f"   {packet_idx} {command} {argument} {response} {ret}")
    try:
      resp = bytes(response)
      sock.sendto(bytes(resp), addr)
      if packet_idx == idx:
        packet_idx = packet_idx + 1
      print("        Response successfully sent!")
    except:        
      print("        Unable to send response")

    #except socket.timeout:
    #  continue


def exit():
  # Close all .sds files
  for file in stream_files.values():
    try:
      file.close()
    except:        
      print("Can not close .sds file")
  #Close Socket
  sock.close()


if __name__ == '__main__':
    atexit.register(exit)
    main()

