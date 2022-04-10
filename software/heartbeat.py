
import devices
import server_threads
import networking
import database
import json
import global_data

media_server_heartbeat_string = "Media_Server_Heartbeat:"

min_delay = 5
max_heartbeat_length = 1024
heartbeat_port = 1900

def heartbeat_keepalive(heartbeat_thread):

   print("Starting Heartbeat")
   heartbeat_thread.pause(min_delay) # delay to allow main thread to advance
   device_config = database.get_device_config()
   last_ip = device_config.ip_addr
   
   while (heartbeat_thread.is_active()):
      new_ip = check_new_ip_addr(last_ip)
      if(new_ip != 0): # check queue to see if ip address changed
         print("IP Address Changed... Sending update to main")
         last_ip = new_ip
         device_config.ip_addr = new_ip
         instruction = global_data.instruction_class()
         instruction.is_local = 1
         instruction.command = "/heartbeat/ip_changed"
         hb_inst_dict = instruction.dump_dict()
         global_data.main_queue.put(hb_inst_dict)
      
      if(not global_data.heartbeat_queue.empty()): # check queue to see if current device config changed
         print("Updating Heartbeat Device Config From Database")
         while(not global_data.heartbeat_queue.empty()):
            try:
               new_inst_dict = global_data.heartbeat_queue.get(block=False) # pull instruction from buffer until empty
               hb_instruction = global_data.instruction_class()
               hb_instruction.load_dict(**new_inst_dict)
               if(hb_instruction.command == "/heartbeat/reload_config"):
                  device_config = database.get_device_config()
            except:
               #fifo was empty... move on
               pass

      send_heartbeat_packet(device_config)
      if(device_config.hb_period < min_delay):
         heartbeat_thread.pause(min_delay)
      else:
         heartbeat_thread.pause(device_config.hb_period)
   
   return

def check_new_ip_addr(old_ip):
   my_ip = networking.get_my_ip()
   if(my_ip != old_ip):
      return my_ip
   else:
      return 0

def send_heartbeat_packet(device_config):
   heartbeat_data = media_server_heartbeat_string + device_config.to_json()
   networking.send_broadcast_packet(heartbeat_port, heartbeat_data.encode(), max_heartbeat_length)

def heartbeat_listener(heartbeat_listener_thread):
   heartbeat_listener_thread.pause(1)
   while (heartbeat_listener_thread.is_active()):
      try:
         heartbeat_data = networking.receive_broadcast_packet(heartbeat_port, max_heartbeat_length)
      except:
         heartbeat_listener_thread.pause(1)
      else:
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
   # pass heartbeat packet up to main server to process
   instruction = global_data.instruction_class()
   instruction.is_local = 1
   instruction.command = "/heartbeat/new_packet"
   packet_data = heartbeat_data[0].decode()
   packet_data = json.loads(packet_data[len(media_server_heartbeat_string):])
   instruction.data = devices.server_device_class(**packet_data)
   hb_dict = instruction.dump_dict()
   global_data.main_queue.put(hb_dict)
   
