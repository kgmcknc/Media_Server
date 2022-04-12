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

local_socket_list = []
device_socket_list = []

udp_rx_sock = 0

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
   max_devices = 5 + len(device_socket_list)

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
         max_devices = len(device_socket_list) + 5
         serversocket.listen(max_devices)
      
      while(not global_data.network_queue.empty()):
         try:
            new_queue_dict = global_data.network_queue.get(block=False)
            new_queue_data = global_data.instruction_class()
            new_queue_data.load_dict(**new_queue_dict)
         except:
            #network queue was empty and timed out
            pass
         else:
            if(new_queue_data.command == "/network/reload_config"):
               print("Got Reload " + new_queue_data.data)
               reload_devices = 1
            else:
               if(new_queue_data.dst == device_config.ip_addr):
                  # packet intended for this device network
                  for dev in local_socket_list:
                     if(new_queue_data.port == dev.port):
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
                           if(dev.connected):
                              new_queue_data.command = "/global/" + str(new_queue_data.global_id) + new_queue_data.command
                              write_data = new_queue_data.dump_global()
                              write_json = json.dumps(write_data)
                              write_data_encode = write_json.encode()
                              dev.socket.send(write_data_encode)
                              break

      # Process ready devices
      for dev in local_socket_list:
         if(dev.ready):
            dev.ready = 0
            if(dev.connected):
               try:
                  data = dev.socket.recv(1024)
               except:
                  print("Local Rcv Error")
                  dev.connected = 0
                  dev.detected = 0
                  dev.done = 1
                  dev.socket.close()
               else:
                  local_instruction = global_data.instruction_class()
                  local_instruction.is_local = 1
                  data_string = data.decode()
                  if(data_string[0:3] == "GET"):
                     start_offset = data_string.find("q={")
                     if(start_offset > 0):
                        end_offset = data_string.rfind("}")+1
                        query_string = data_string[start_offset+2:end_offset]
                        offset = query_string.find(":")
                        if(offset < 1):
                           local_instruction.command = query_string[2:len(query_string)-1]
                           local_instruction.data = ""
                        else:
                           local_instruction.command = query_string[2:offset-1]
                           query_data = query_string[offset+1:len(query_string)-1]
                           local_instruction.data = json.loads(query_data)
                     else:
                        local_instruction.command = ""
                        local_instruction.data = ""
                     local_instruction.src = dev.ip_addr
                     local_instruction.dst = dev.ip_addr
                     local_instruction.port = dev.port
                     local_instruction_split = local_instruction.command.split("/")
                     if((len(local_instruction_split) > 1) and (local_instruction_split[1] == "global")):
                        local_instruction.is_global = 1
                        local_instruction.global_id = int(local_instruction_split[2])
                        new_command = ""
                        for x in range(3, len(local_instruction_split)):
                           new_command = new_command + "/" + local_instruction_split[x]
                        local_instruction.command = new_command
                     local_get_dict = local_instruction.dump_dict()
                     global_data.main_queue.put(local_get_dict)
                     dev.connected = 0
                     dev.detected = 0
                  else:
                     if(data_string[0:4] == "POST"):
                        start_offset = data_string.find("q={")
                        if(start_offset > 1):
                           end_offset = data_string.rfind("}")+1
                           query_string = data_string[start_offset+2:end_offset]
                           offset = query_string.find(":")
                           if(offset < 1):
                              local_instruction.command = query_string[2:len(query_string)-1]
                              query_data = ""
                           else:
                              local_instruction.command = query_string[2:offset-1]
                              query_data = query_string[offset+1:len(query_string)-1]
                              local_instruction.data = json.loads(query_data)
                        else:
                           local_instruction.command = ""
                           local_instruction.data = ""
                        local_instruction.src = dev.ip_addr
                        local_instruction.dst = dev.ip_addr
                        local_instruction.port = dev.port
                        local_instruction_split = local_instruction.command.split("/")
                        if((len(local_instruction_split) > 1) and (local_instruction_split[1] == "global")):
                           local_instruction.is_global = 1
                           local_instruction.global_id = int(local_instruction_split[2])
                           new_command = ""
                           for x in range(3, len(local_instruction_split)):
                              new_command = new_command + "/" + local_instruction_split[x]
                           local_instruction.command = new_command
                        local_post_dict = local_instruction.dump_dict()
                        global_data.main_queue.put(local_post_dict)
                        #packet_data = header_okay+header_end
                        #write_data_encode = packet_data.encode()
                        #dev.socket.send(write_data_encode)
                        dev.connected = 0
                        #dev.done = 1
                        #dev.socket.shutdown(socket.SHUT_RDWR)
                        #dev.socket.close()
                     else:
                        pass

      for dev in device_socket_list:
         if(dev.ready):
            dev.ready = 0
            data = b''
            if(dev.connected):
               try:
                  data = dev.socket.recv(1024)
               except:
                  print("Socket Closed Error" + dev.ip_addr)
                  data = b''
                  dev.socket.close()
                  dev.done = 1
                  dev.connected = 0
                  disconnect_inst = global_data.instruction_class()
                  disconnect_inst.command = "/heartbeat/device_disconnected"
                  disconnect_inst.is_internal = 1
                  disconnect_inst.data = dev.device_id
                  disconnect_dict = disconnect_inst.dump_dict()
                  global_data.main_queue.put(disconnect_dict)
               else:
                  if(data == b''):
                     print("Socket Closed Success" + dev.ip_addr)
                     dev.socket.shutdown(socket.SHUT_RDWR)
                     dev.socket.close()
                     dev.done = 1
                     dev.connected = 0
                     disconnect_inst = global_data.instruction_class()
                     disconnect_inst.command = "/heartbeat/device_disconnected"
                     disconnect_inst.is_internal = 1
                     disconnect_inst.data = dev.device_id
                     disconnect_dict = disconnect_inst.dump_dict()
                     global_data.main_queue.put(disconnect_dict)
                     #send something to main here for disconnect? clear db?
                  else:
                     dev_instruction = global_data.instruction_class()
                     data_string = data.decode()
                     global_dict = json.loads(data_string)
                     dev_instruction.load_global(global_dict)
                     dev_instruction.src = dev.ip_addr
                     dev_instruction.dst = device_config.ip_addr
                     dev_instruction_split = dev_instruction.command.split("/")
                     if((len(dev_instruction_split) > 1) and (dev_instruction_split[1] == "global")):
                        dev_instruction.is_global = 1
                        dev_instruction.global_id = int(dev_instruction_split[2])
                        new_command = ""
                        for x in range(3, len(dev_instruction_split)):
                           new_command = new_command + "/" + dev_instruction_split[x]
                        dev_instruction.command = new_command
                     else:
                        dev_instruction.is_device = 1
                     dev_inst_dict = dev_instruction.dump_dict()
                     global_data.main_queue.put(dev_inst_dict)

      # Remove old unconnected devices
      # new_list = []
      # for dev in device_socket_list:
      #    if(dev.done):
      #       dev.ready = 0
      #       dev.done = 0
      #    else:
      #       new_list.append(dev)
      # device_socket_list = new_list

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
         if(rx_device.connected):
            rx_list.append(rx_device.socket)

      tx_list = []
      for local_device in local_socket_list:
         tx_list.append(local_device.socket)
      for tx_device in device_socket_list:
         if(tx_device.connected):
            # should probably check to see if we are able to transmit here...
            pass
         else:
            # smaller device number is client... random decision
            if((tx_device.detected) and (tx_device.device_id < device_config.device_id)):
               try:
                  tx_device.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                  tx_device.socket.connect((tx_device.ip_addr, tx_device.port))
                  print("Connected New Socket" + tx_device.ip_addr)
               except:
                  print("Couldn't make connection" + tx_device.ip_addr)
                  tx_device.connected = 0
               else:
                  tx_device.connected = 1
                  tx_list.append(tx_device.socket)
                  device_inst = global_data.instruction_class()
                  device_inst.command = "/heartbeat/device_connected"
                  device_inst.is_internal = 1
                  device_inst.data = tx_device.device_id
                  hb_dict = device_inst.dump_dict()
                  global_data.main_queue.put(hb_dict)

      rx_ready, tx_ready, x_ready = select.select(rx_list, tx_list, [], select_timeout)
      
      for local_sock in local_socket_list:
         local_sock.ready = 0
      for dev_sock in device_socket_list:
         dev_sock.ready = 0

      # checking connections coming in
      for ready in rx_ready:
         if(ready == serversocket):
            #found connection to our main socket input
            try:
               new_socket, new_address = serversocket.accept()
               # if the address is this devices address, then it came from local web
               if(new_address[0] == device_config.ip_addr):
                  print("Accepted Local Socket" + new_address[0])
                  new_sock = devices.server_device_class()
                  new_sock.socket = new_socket
                  new_sock.ready = 0
                  new_sock.detected = 1
                  new_sock.connected = 1
                  new_sock.ip_addr = new_address[0]
                  new_sock.port = new_address[1]
                  local_socket_list.append(new_sock)
               else:
                  # this is a new remote device connecting
                  found_connection = 0
                  for device_sock in device_socket_list:
                     if(new_address[0] == device_sock.ip_addr):
                        if(device_sock.detected):
                           found_connection = 1
                           print("Accepted New Socket" + new_address[0])
                           device_sock.socket = new_socket
                           device_sock.ready = 0
                           device_sock.connected = 1
                           device_sock.ip_addr = new_address[0]
                           device_sock.port = new_address[1]
                           device_inst = global_data.instruction_class()
                           device_inst.command = "/heartbeat/device_connected"
                           device_inst.is_internal = 1
                           device_inst.data = device_sock.device_id
                           hb_dict = device_inst.dump_dict()
                           global_data.main_queue.put(hb_dict)
                           print("Put device connection change ", new_address[0])
                        else:
                           print("Accept error, device wasn't detected...")
                  if(found_connection == 0):
                     print("Accept didn't find known device... closing connection ", new_address[0])
                     # send packet indicating unknown device
                     conn_inst = global_data.instruction_class()
                     conn_inst.command = "/network/received_unknown_connection"
                     conn_inst.is_internal = 1 # this just goes to other device, where it will get decoded as device instruction
                     conn_inst.src = new_address[0]
                     conn_inst.dst = new_address[0]
                     conn_inst.port = new_address[1]
                     write_data = conn_inst.dump_global()
                     write_json = json.dumps(write_data)
                     write_data_encode = write_json.encode()
                     new_socket.send(write_data_encode)
                     new_socket.shutdown(socket.SHUT_RDWR)
                     new_socket.close()
            except:
               print("accept exception")
         else:
            # connection we have already established is ready for packet transmit
            for local_sock in local_socket_list:
               if(ready == local_sock.socket):
                  local_sock.ready = 1
            for dev_sock in device_socket_list:
               if(ready == dev_sock.socket):
                  dev_sock.ready = 1
      
      # go through other rx devices and find read ready signals for sent messages

      # for tx_sock in tx_ready:
      #    for dev_sock in device_socket_list:
      #          if(tx_sock == dev_sock.socket):
      #             dev_sock.active = 1
   
   for dev in device_socket_list:
      if(dev.connected):
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

   device_config = database.get_device_config()
   
   db_filter = {"detected":1}
   db_devices = database.get_filtered_db_devices(**db_filter)
   if(len(db_devices) == 0):
      for dev in device_socket_list:
         if(dev.done == 0):
               dev.socket.shutdown(socket.SHUT_RDWR)
               dev.socket.close()
      device_socket_list = []
   else:
      for new_device in db_devices:
         # rebuild new device list with all current devices
         
         # see if device is already in socket list
         found_device = 0
         for index in device_socket_list:
            if(index.device_id == new_device.device_id):
               found_device = 1
               # need to make sure this doesn't overwrite the socket of already connected devices...
               new_vars = vars(new_device)
               for key in new_vars:
                  if((key != "socket") and (key != "_id")):
                     setattr(index, key, getattr(new_device, key))
               break
         if(found_device == 0):
            # if not, then add to device socket list
            new_sock = devices.server_device_class()
            new_sock.ip_addr = new_device.ip_addr
            new_sock.port = new_device.port
            new_sock.device_id = new_device.device_id
            new_sock.detected = new_device.detected
            new_sock.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            device_socket_list.append(new_sock)
      
         # Remove devices here
         # check all current devices and see if they are still in the database
         # if they aren't in the database, close the connection if it's not done
         new_list = []
         for dev in device_socket_list:
            for db_dev in db_devices:
               if(dev.device_id == db_dev.device_id):
                  #found device so keep in list
                  new_list.append(dev)
                  break
            else:
               # device wasn't in database...
               print("Removing Device " + dev.ip_addr)
               if(dev.done == 0):
                  if(dev.connected):
                     dev.socket.shutdown(socket.SHUT_RDWR)
                  dev.socket.close()
         device_socket_list = new_list

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



