import socket
import heartbeat

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
   udp_sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
   udp_sock.bind(('',0))
   udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
   udp_sock.sendto(packet_data, ('<broadcast>', packet_port))

def receive_broadcast_packet(packet_port, packet_length):
   udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
   udp_sock.bind(('',packet_port))
   udp_packet_data = udp_sock.recvfrom(packet_length)
   return udp_packet_data


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



