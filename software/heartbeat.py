
import server_threads
import networking
import database

heartbeat_delay = 30

def set_heartbeat_period(heartbeat_period):
   global heartbeat_delay
   heartbeat_delay = heartbeat_period
   return

def heartbeat_keepalive(heartbeat_thread):
   print("Starting Heartbeat")

   while (heartbeat_thread.is_active()):
      send_heartbeat_packet()
      heartbeat_thread.pause(heartbeat_delay)
   
   return

def send_heartbeat_packet():
   device_config = 0
   device_config = database.get_device_config()
   udp_packet = device_config
   broadcast_port = 1900
   networking.send_broadcast_packet(broadcast_port, udp_packet)

def receive_heartbeat_packet():
   pass