import socket
import heartbeat
import devices
import global_data
import database

device_config = 0
device_list = []

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

   network_thread.pause(1)
   
   load_devices_from_db()

   while (network_thread.is_active()):
      try:
         new_queue_data = global_data.network_queue.get(timeout=global_data.network_queue_timeout)
      except:
         #network queue was empty and timed out
         pass
      else:
         process_instruction(new_queue_data)
      pass

def load_devices_from_db():
   global device_config
   global device_list
   device_config = devices.server_device_class(**database.get_device_config())
   device_list = []
   device_list.append(device_config)
   db_devices = database.get_linked_db_devices()
   for new_device in db_devices:
      device_list.append(new_device)

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



