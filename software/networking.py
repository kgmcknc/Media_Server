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
   socket = 0,
   ip_addr = 0,
   port = 0,
   device_id = 0,
   active = 0,
   ready = 0,
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

def receive_broadcast_packet(packet_port, packet_length):
   global udp_rx_sock
   udp_rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
   udp_rx_sock.bind(('',packet_port))
   udp_packet_data = udp_rx_sock.recvfrom(packet_length)
   return udp_packet_data

def abort_broadcast_receive():
   global udp_rx_sock
   udp_rx_sock.close()

def network_listener(network_thread):
   global device_socket_list
   global local_socket_list

   network_thread.pause(1)

   load_devices_from_db()
   max_devices = 5 + len(device_list)

   print("Starting network thread")

   # create an INET, STREAMing socket
   serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
   serversocket.setblocking(False)
   serversocket.bind((device_config.ip_addr, device_config.port))
   # become a server socket
   serversocket.listen(max_devices)

   while (network_thread.is_active()):
      if(reload_devices):
         load_devices_from_db()
         max_devices = len(device_list) + 5
         serversocket.listen(max_devices)

      try:
         new_queue_data = global_data.network_queue.get(block=False)
      except:
         #network queue was empty and timed out
         pass
      else:
         while(not global_data.network_queue.empty()):
            process_instruction(new_queue_data)

      # Process ready devices
      for dev in local_socket_list:
         if(dev.ready):
            dev.ready = 0
            data = dev.socket.recv(1024)
            instruction = global_data.instruction_class()
            instruction.group = "local_tasks"
            instruction.task = ""
            instruction.data = data.decode()
            if(instruction.data[0:3] == "GET"):
               global_data.main_queue.put(instruction)
            else:
               if(instruction.data[0:4] == "POST"):
                  offset = instruction.data.find("q={")
                  instruction.data = instruction.data[offset+2:len(instruction.data)]
                  instruction.data = json.loads(instruction.data)
                  global_data.main_queue.put(instruction)
                  dev.active = 0
                  dev.done = 1
                  dev.socket.close()
               else:
                  pass

      for dev in device_socket_list:
         if(dev.ready):
            data = dev.socket.recv(1024)
            dev.ready = 0
            print("Device Read")

      # Remove old unconnected devices
      for dev in device_socket_list:
         if(dev.done):
            dev.done = 0
            device_socket_list.remove(dev)

      for dev in local_socket_list:
         if(dev.done):
            dev.done = 0
            local_socket_list.remove(dev)
      
      rx_list = [serversocket]
      for local_device in local_socket_list:
         rx_list.append(local_device.socket)
      for rx_device in device_socket_list:
         if(rx_device.active):
            rx_list.append(rx_device.socket)
         else:
            # larger device number is server... random decision
            if(rx_device.device_id > device_config.device_id):
               rx_list.append(rx_device.socket)

      tx_list = []
      for tx_device in device_socket_list:
         if(tx_device.active):
            # should probably check to see if we are able to transmit here...
            pass
         else:
            # smaller device number is client... random decision
            if(tx_device.device_id < device_config.device_id):
               tx_list.append(tx_device.socket)

      rx_ready, tx_ready, x_ready = select.select(rx_list, tx_list, [], select_timeout)

      for ready in rx_ready:
         if(ready == serversocket):
            try:
               new_socket, new_address = serversocket.accept()
               if(new_address[0] == device_config.ip_addr):
                  new_sock = my_socket_class()
                  new_sock.socket = new_socket
                  new_sock.ready = 1
                  new_sock.active = 1
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
               if(ready == dev_sock.socket):
                  dev_sock.socket.connect(dev_sock.ip_addr, dev_sock.ip_port)
                  

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
      if(device_config.device_id in new_device.linked_devices):
         device_list.append(new_device)
         
         # see if device is already in socket list
         for index in device_socket_list:
            if(index.device_id == new_device.device_id):
               index = new_device
               break
         else:
            # if not, then add to device socket list
            new_sock = my_socket_class()
            new_sock.ip_addr = db_devices[index].ip_addr
            new_sock.port = db_devices[index].port
            new_sock.device_id = db_devices[index].device_id
            new_sock.active = 0
            device_socket_list.append(new_sock)
   # old devices that need to be "deleted" from device list is handled in the loop
   # that way the socket can be closed if it's somehow still opened...


def process_instruction(data):
   pass


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



