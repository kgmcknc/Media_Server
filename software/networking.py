import socket
import heartbeat
import devices
import global_data
import database
import select
import json

select_timeout = 1
reload_devices = 0
device_config = 0

device_list = []
local_socket_list = []
device_socket_list = []

udp_rx_sock = 0

class my_socket_class:
   socket = 0
   ip_addr = 0
   port = 0
   device_id = 0
   active = 0
   is_get = 0
   ready = 0
   done = 0

def get_my_ip():
   s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
   try:
      # doesn't even have to be reachable
      s.connect(('255.255.255.255', 1))
      ip_address = s.getsockname()[0]
   except Exception:
      try:
         # doesn't even have to be reachable
         s.connect(('10.255.255.255', 1))
         ip_address = s.getsockname()[0]
      except Exception:
         ip_address = '127.0.0.1'
   finally:
      s.close()
   return ip_address
   
def send_broadcast_packet(packet_port, packet_data, packet_length):
   udp_tx_sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
   udp_tx_sock.bind(('',0))
   udp_tx_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
   udp_tx_sock.sendto(packet_data, ('<broadcast>', packet_port))
   udp_tx_sock.close()

def receive_broadcast_packet(packet_port, packet_length):
   global udp_rx_sock
   udp_rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
   udp_rx_sock.bind(('',packet_port))
   udp_packet_data = udp_rx_sock.recvfrom(packet_length)
   udp_rx_sock.close()
   return udp_packet_data

def abort_broadcast_receive():
   global udp_rx_sock
   udp_rx_sock.close()

def network_listener(network_thread):
   global reload_devices
   global device_socket_list
   global local_socket_list

   network_thread.pause(1)

   load_devices_from_db()
   max_devices = 5 + len(device_list)

   print("Starting network thread")
   
   header_okay = "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\n"
   header_content = "Content-Length: "
   header_end = "Connection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n"

   serversocket = 0
   # create an INET, STREAMing socket
   while network_thread.is_active() and serversocket == 0:
      try:
         serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
         serversocket.setblocking(False)
         serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
         serversocket.bind((device_config.ip_addr, device_config.port))
         # become a server socket
         serversocket.listen(max_devices)
      except:
         print("Couldn't connect network listener")
         serversocket = 0
         network_thread.pause(2)

   while (network_thread.is_active()):
      if(reload_devices):
         reload_devices = 0
         load_devices_from_db()
         max_devices = len(device_list) + 5
         serversocket.listen(max_devices)
      
      while(not global_data.network_queue.empty()):
         try:
            new_queue_data = global_data.network_queue.get(block=False)
         except:
            #network queue was empty and timed out
            pass
         else:
            if(new_queue_data.command == "/network/reload_config"):
               reload_devices = 1
            else:
               if(new_queue_data.dst == device_config.ip_addr):
                  # packet intended for this device network
                  for dev in local_socket_list:
                     if(new_queue_data.socket == dev.socket):
                        write_data = new_queue_data.data_to_json()
                        packet_data = header_okay+header_content+str(len(write_data))+header_end+write_data
                        write_data_encode = packet_data.encode()
                        dev.socket.send(write_data_encode)
                        dev.done = 1
                        dev.socket.shutdown(socket.SHUT_RDWR)
                        dev.socket.close()
               else:
                  # packet intended for another device... lets see if it's global
                  if(new_queue_data.is_global):
                     for dev in device_socket_list:
                        if(new_queue_data.dst == dev.ip_addr):
                           if(dev.active):
                              new_queue_data.socket = dev.socket
                              new_queue_data.command = "/global/" + str(new_queue_data.global_id) + new_queue_data.command
                              write_data = {new_queue_data.command:new_queue_data.data}
                              write_json = json.dumps(write_data)
                              write_data_encode = write_json.encode()
                              dev.socket.send(write_data_encode)

      # Process ready devices
      for dev in local_socket_list:
         if(dev.ready):
            dev.ready = 0
            if(dev.active):
               try:
                  data = dev.socket.recv(1024)
               except:
                  print("Local Rcv Error")
               else:
                  instruction = global_data.instruction_class()
                  data_string = data.decode()
                  if(data_string[0:3] == "GET"):
                     offset = data_string.find("q={")
                     end_offset = data_string.rfind("}")+1
                     query_string = data_string[offset+2:end_offset]
                     offset = query_string.find(":")
                     instruction.command = query_string[2:offset-1]
                     query_data = query_string[offset+1:len(query_string)-1]
                     instruction.data = json.loads(query_data)
                     instruction.socket = dev.socket
                     instruction.src = dev.ip_addr
                     instruction.dst = dev.ip_addr
                     instruction_split = instruction.command.split("/")
                     if((len(instruction_split) > 1) and (instruction_split[1] == "global")):
                        instruction.is_global = 1
                        instruction.global_id = int(instruction_split[2])
                        new_command = ""
                        for x in range(3, len(instruction_split)):
                           new_command = new_command + "/" + instruction_split[x]
                        instruction.command = new_command
                     global_data.main_queue.put(instruction)
                     dev.active = 0
                  else:
                     if(data_string[0:4] == "POST"):
                        offset = data_string.find("q={")
                        end_offset = data_string.rfind("}")+1
                        query_string = data_string[offset+2:end_offset]
                        offset = query_string.find(":")
                        instruction.command = query_string[2:offset-1]
                        query_data = query_string[offset+1:len(query_string)-1]
                        instruction.data = json.loads(query_data)
                        instruction.socket = dev.socket
                        instruction.src = dev.ip_addr
                        instruction.dst = dev.ip_addr
                        instruction_split = instruction.command.split("/")
                        if((len(instruction_split) > 1) and (instruction_split[1] == "global")):
                           instruction.is_global = 1
                           instruction.global_id = int(instruction_split[2])
                           new_command = ""
                           for x in range(3, len(instruction_split)):
                              new_command = new_command + "/" + instruction_split[x]
                           instruction.command = new_command
                        global_data.main_queue.put(instruction)
                        #packet_data = header_okay+header_end
                        #write_data_encode = packet_data.encode()
                        #dev.socket.send(write_data_encode)
                        dev.active = 0
                        #dev.done = 1
                        #dev.socket.shutdown(socket.SHUT_RDWR)
                        #dev.socket.close()
                     else:
                        pass

      for dev in device_socket_list:
         if(dev.ready):
            dev.ready = 0
            data = b''
            if(dev.active):
               try:
                  data = dev.socket.recv(1024)
               except:
                  print("Socket Closed Error" + dev.ip_addr)
                  data = b''
                  dev.socket.close()
                  dev.done = 1
                  dev.active = 0
               else:
                  if(data == b''):
                     #print("Socket Closed Success" + dev.ip_addr)
                     dev.socket.shutdown(socket.SHUT_RDWR)
                     dev.socket.close()
                     dev.done = 1
                     dev.active = 0
                  else:
                     instruction = global_data.instruction_class()
                     data_string = data.decode()
                     offset = data_string.find(":")
                     instruction.command = data_string[2:offset-1]
                     query_data = data_string[offset+1:len(data_string)-1]
                     instruction.data = json.loads(query_data)
                     instruction.socket = dev.socket
                     instruction.src = dev.ip_addr
                     instruction.dst = device_config.ip_addr
                     instruction_split = instruction.command.split("/")
                     if((len(instruction_split) > 1) and (instruction_split[1] == "global")):
                        instruction.is_global = 1
                        instruction.global_id = int(instruction_split[2])
                        new_command = ""
                        for x in range(3, len(instruction_split)):
                           new_command = new_command + "/" + instruction_split[x]
                        instruction.command = new_command
                     global_data.main_queue.put(instruction)

      # Remove old unconnected devices
      new_list = []
      for dev in device_socket_list:
         if(dev.done):
            dev.ready = 0
            dev.done = 0
         else:
            new_list.append(dev)
      device_socket_list = new_list

      new_list = []
      for dev in local_socket_list:
         if(dev.done):
            dev.ready = 0
            dev.done = 0
         else:
            new_list.append(dev)
      local_socket_list = new_list

      rx_list = [serversocket]
      for local_device in local_socket_list:
         rx_list.append(local_device.socket)
      for rx_device in device_socket_list:
         if(rx_device.active):
            rx_list.append(rx_device.socket)

      tx_list = []
      for local_device in local_socket_list:
         tx_list.append(local_device.socket)
      for tx_device in device_socket_list:
         if(tx_device.active):
            # should probably check to see if we are able to transmit here...
            pass
         else:
            # smaller device number is client... random decision
            if(tx_device.device_id < device_config.device_id):
               try:
                  tx_device.socket.connect((tx_device.ip_addr, tx_device.port))
                  #print("Connected New Socket" + tx_device.ip_addr)
               except:
                  #print("Couldn't make connection")
                  pass
               else:
                  tx_list.append(tx_device.socket)

      rx_ready, tx_ready, x_ready = select.select(rx_list, tx_list, [], select_timeout)
      
      for local_sock in local_socket_list:
         local_sock.ready = 0
      for dev_sock in device_socket_list:
         dev_sock.ready = 0

      for ready in rx_ready:
         if(ready == serversocket):
            try:
               new_socket, new_address = serversocket.accept()
               #print("Accepted New Socket" + new_address[0])
               if(new_address[0] == device_config.ip_addr):
                  new_sock = my_socket_class()
                  new_sock.socket = new_socket
                  new_sock.ready = 1
                  new_sock.active = 1
                  new_sock.ip_addr = device_config.ip_addr
                  local_socket_list.append(new_sock)
               else:
                  for device_sock in device_socket_list:
                     if(new_address[0] == device_sock.ip_addr):
                        device_sock.socket = new_socket
                        device_sock.ready = 0
                        device_sock.active = 1
            except:
               print("accept exception")
         else:
            for local_sock in local_socket_list:
               if(ready == local_sock.socket):
                  local_sock.ready = 1
            for dev_sock in device_socket_list:
               if(ready == dev_sock.socket):
                  dev_sock.ready = 1
      
      # go through other rx devices and find read ready signals for sent messages

      for tx_sock in tx_ready:
         for dev_sock in device_socket_list:
               if(tx_sock == dev_sock.socket):
                  dev_sock.active = 1
   
   for dev in device_socket_list:
      if(dev.active):
         dev.socket.shutdown(socket.SHUT_RDWR)
         dev.socket.close()
   if(serversocket):
      serversocket.close()

   # check for linked and connected devices that we don't have a socket connected for...
   # if so, create and listen on those sockets

   # set up and do select with a timeout period of 1 second
   # could also check to see if we have a registered "response" instruction in progress...
   # if so, we should do non blocking check on sockets so we poll without waiting until the response is back
   # check for closed connections

def load_devices_from_db():
   global device_socket_list
   global device_config
   global device_list

   device_config = database.get_device_config()
   
   device_list = []
   db_filter = {"connected":1}
   db_devices = database.get_filtered_db_devices(**db_filter)
   for new_device in db_devices:
      # rebuild new device list with all current devices
      # add device to list
      if(1):#device_config.device_id in new_device.linked_devices):
         device_list.append(new_device)
         
         # see if device is already in socket list
         for index in device_socket_list:
            if(index.device_id == new_device.device_id):
               index = new_device
               break
         else:
            # if not, then add to device socket list
            new_sock = my_socket_class()
            new_sock.ip_addr = new_device.ip_addr
            new_sock.port = new_device.port
            new_sock.device_id = new_device.device_id
            new_sock.active = 0
            new_sock.ready = 0
            new_sock.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            device_socket_list.append(new_sock)
   # old devices that need to be "deleted" from device list is handled in the loop
   # that way the socket can be closed if it's somehow still opened...

#get database exists
#set init database
#set reset database
#set update database

#set add media folder
#set remove media folder
#set index media folder (set file names, location, type (music/movie), last play timestamp)
#  each media folder gets it's own database (albums, artists, files, timestamps)
#  have playlist database with playlist name, playlist items, last item played, last item time

#get media status (get playing/file/playlist,location)
#set start media (set playlist/start playing, choose to sync?)
#set media control
#turn tv on/off/active

#get door sensors (name, location, status)
#get alarm status (armed, status)
#set alarm status (armed)



