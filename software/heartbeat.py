
import devices
import server_threads
import networking
import database
import json
import global_data

media_server_heartbeat_string = "Media_Server_Heartbeat:"

heartbeat_delay = 30
max_heartbeat_length = 1024
heartbeat_port = 1900

device_config = devices.server_device_class()
last_ip = 0

def heartbeat_keepalive(heartbeat_thread):
   global device_config
   global last_ip
   print("Starting Heartbeat")

   last_ip = networking.get_my_ip()
   device_config.update_device_info(**database.get_device_config())

   while (heartbeat_thread.is_active()):
      if(ip_addr_changed()): # check queue to see if ip address changed
         print("IP Address Changed... Sending update to main")
         new_ip = 1
         global_data.main_queue.put(new_ip)
      
      if(not global_data.heartbeat_queue.empty()): # check queue to see if current device config changed
         print("Updating Device Config From Database")
         while(not global_data.heartbeat_queue.empty()):
            data = global_data.heartbeat_queue.get(block=false) # pull data from buffer until empty
         device_config.update_device_info(**database.get_device_config())

      send_heartbeat_packet()
      heartbeat_thread.pause(heartbeat_delay)
   
   return

def ip_addr_changed():
   global last_ip
   device_ip = networking.get_my_ip()
   if(device_ip != last_ip):
      last_ip = device_ip
      return 1
   else:
      return 0

def set_heartbeat_period(heartbeat_period):
   global heartbeat_delay
   heartbeat_delay = heartbeat_period
   return

def send_heartbeat_packet():
   heartbeat_data = media_server_heartbeat_string + device_config.to_json()
   networking.send_broadcast_packet(heartbeat_port, heartbeat_data.encode(), max_heartbeat_length)

def heartbeat_listener(heartbeat_listener_thread):
   while (heartbeat_listener_thread.is_active()):
      heartbeat_data = networking.receive_broadcast_packet(heartbeat_port, max_heartbeat_length)
      if(is_heartbeat_packet(heartbeat_data)):
         receive_heartbeat_packet(heartbeat_data)

   return

def is_heartbeat_packet(heartbeat_data):
   heartbeat_text = heartbeat_data[0]
   heartbeat_text = heartbeat_text.decode()
   if(media_server_heartbeat_string in heartbeat_text):
      return 1
   else:
      return 0

def receive_heartbeat_packet(heartbeat_data):
   this_ip = networking.get_my_ip()
   if(this_ip != heartbeat_data[1][0]):
      global_data.main_queue.put(heartbeat_data)
   else:
      #heartbeat was from this device... ignore
      pass
   
