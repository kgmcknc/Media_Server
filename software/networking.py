import socket

max_upd_length = 1024

def network_test(network_thread):
   while (network_thread.is_active()):
      network_thread.pause(1)
      receive_broadcast_packet(1900)

   return
   
def send_broadcast_packet(packet_port, packet_data):
   print("would send broadcast here")
   udp_sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
   udp_sock.bind(('',0))
   udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
   udp_sock.sendto(packet_data, ('<broadcast>', packet_port))

def receive_broadcast_packet(packet_port):
   print("waiting for bcast")
   udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
   udp_sock.bind(('',packet_port))
   udp_packet_data = udp_sock.recvfrom(max_upd_length)
   print(udp_packet_data)


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



