
import global_data
import media_player
import server_threads
import devices
import heartbeat
import database
import threading
import networking
import signal
import sys
import queue

main_thread = server_threads.server_thread_class("Main Server Thread")
media_player_thread = server_threads.server_thread_class("Media Player Thread")
heartbeat_listener_thread = server_threads.server_thread_class("Heartbeat Listener Thread")
heartbeat_thread = server_threads.server_thread_class("Heartbeat Thread")
network_thread = server_threads.server_thread_class("Network Thread")

config_changed = 0
skips_till_timeout = 4
system_running = 1

device_list = []
device_timeouts = []

def server_main(main_thread):
   global config_changed
   print("Starting Main")

   #TODO need to make more advanced device list/struct/array for keeping track of stuff like whether current media is linked between other devices
   
   init_system()

   heartbeat_listener_thread.start(heartbeat.heartbeat_listener)
   heartbeat_thread.start(heartbeat.heartbeat_keepalive)
   media_player_thread.start(media_player.run_media_player)
   network_thread.start(networking.network_listener)

   print("main while")
   while (main_thread.is_active()):
      #TODO main will wait for queue to have something (or timeout)
      # if timeout happens we just run through loop to do whatever periodic task is needed... (check ip is only one now)
      # queue is used to return data back to website 
      if(config_changed):
         print("Main config changed")
         hb_instruction = global_data.instruction_class()
         hb_instruction.command = "/heartbeat/reload_config"
         hb_inst_dict = hb_instruction.dump_dict()
         global_data.heartbeat_queue.put(hb_inst_dict)
         nw_instruction = global_data.instruction_class()
         nw_instruction.command = "/network/reload_config"
         nw_inst_dict = nw_instruction.dump_dict()
         global_data.network_queue.put(nw_inst_dict)
         config_changed = 0
      try:
         new_inst_dict = global_data.main_queue.get(timeout=global_data.main_queue_timeout)
         new_main_instruction = global_data.instruction_class()
         new_main_instruction.load_dict(**new_inst_dict)
      except:
         #main queue was empty and timed out
         pass
      else:
         process_main_instruction(new_main_instruction)
   
   server_shutdown()
   return

def process_main_instruction(instruction:global_data.instruction_class):
   global config_changed
   global device_list
   update_config = 0
   instruction_split = instruction.command.split("/")
   process_instruction = 0
   
   if(instruction.is_global):
      process_instruction = 0
      found_device = 0
      found_ip = 0
      for device in device_list:
         if(device.device_id == instruction.global_id):
            found_device = 1
            found_ip = device.ip_addr
            break
      if(found_device):
         if(instruction.src == device_list[0].ip_addr):
            # instruction was received from this device
            # lets see if it's intended for this device or another device
            if((instruction.dst == device_list[0].ip_addr) and (instruction.global_id == device_list[0].device_id)):
               print("Global packet src and dst the same... odd... process or drop?")
               # dropping for now...
            else:
               # intended for another device... send to that device
               #print("Global packet forwarded")
               instruction.dst = found_ip
               fwd_inst_dict = instruction.dump_dict()
               global_data.network_queue.put(fwd_inst_dict, block=False)
               # add to global message queue here...
         else:
            # instrucion was received from another device
            # lets see if it's intended for processing on this device or if it's a response from other device
            if((instruction.dst == device_list[0].ip_addr) and (instruction.global_id == device_list[0].device_id)):
               # intended for this device
               old_ip = instruction.src
               instruction.src = instruction.dst
               instruction.dst = old_ip
               process_instruction = 1
               #print("Global packet received")
               # command will be run below and be returned to src device
            else:
               # global packet response to be returned to this device network
               # find in global message queue at this point...
               #print("Global packet response")
               rsp_inst_dict = instruction.dump_dict()
               global_data.network_queue.put(rsp_inst_dict, block=False)

            # here we need to put the global command in a queue with the instruction socket it was 
            # received from and the command
            # then when we receive the socket again we can check src/dst to see if it's a transmit
            # or a response and know whether to send to other device or send back up to web
            # need to check to make sure device is still connected and if not, send response back
            # up to webpage. Also make sure in network we don't send up to web if that socket dies...
            # it'll probably just get dropped in networking
   else:
      process_instruction = 1

   if(process_instruction):
      if(instruction.command == "/system/shutdown_delay"):
         while(main_thread.is_active()):
            pass

      if(instruction.command == "/heartbeat/ip_changed"):
         device_list[0].ip_addr = networking.get_my_ip()
         update_config = 1

      if(instruction.command == "/heartbeat/new_packet"):
         update_device_list(instruction.data)
      
      if((len(instruction_split) > 1) and (instruction_split[1] == "database")):
         return_data = process_local_task(instruction)
         instruction.data = return_data
         nw_inst_dict = instruction.dump_dict()
         global_data.network_queue.put(nw_inst_dict, block=False)

      if((len(instruction_split) > 1) and (instruction_split[1] == "media")):
         media_inst_dict = instruction.dump_dict()
         global_data.media_queue.put(media_inst_dict, block=False)

      if(instruction.is_global and instruction.global_done):
         global_inst_dict = instruction.dump_dict()
         global_data.network_queue.put(global_inst_dict, block=False)

   if(update_config):
      database.update_db_device_config(device_list[0])
      config_changed = 1

def process_local_task(instruction:global_data.instruction_class):
   global config_changed
   index_folder = 0
   json_object = instruction.data
   instruction.data = ""
   #instruction.data = {}
   
   if(instruction.command == "/database/link_device"):
      for dev in device_list:
         if(dev.device_id == json_object["device_id"]):
            database.add_linked_device(json_object["device_id"])
            config_changed = 1
            break
   if(instruction.command == "/database/update_config"):
      json_object.pop("command")
      for data in json_object:
         if((data == 'device_id') or (data == '_id')):
            print("not updating id stuff")
         else:
            setattr(device_list[0], data, json_object[data])
      config_changed = 1
   if(instruction.command == "/database/add_media_folder"):
      #instruction.data = json_object
      index_folder = database.add_media_folder(json_object)
   if(instruction.command == "/database/rem_media_folder"):
      database.rem_media_folder(json_object)
   if(instruction.command == "/database/index_media_folder"):
      index_folder = 1
   if(instruction.command == "/database/get_media_folders"):
      instruction.data = database.get_media_folders()
   if(instruction.command == "/database/get_media_data"):
      instruction.data = database.get_media_data(json_object)

   if(instruction.command == "/database/add_media_link"):
      #instruction.data = json_object
      index_folder = database.add_media_link(json_object)
   if(instruction.command == "/database/rem_media_link"):
      database.rem_media_link(json_object)
   if(instruction.command == "/database/get_media_links"):
      instruction.data = database.get_media_links()
   if(instruction.command == "/database/get_media_link"):
      instruction.data = database.get_media_link(json_object)
   
   if(instruction.command == "/database/add_user"):
      instruction.data = database.add_user(json_object)
   if(instruction.command == "/database/rem_user"):
      instruction.data = database.rem_user(json_object)
   if(instruction.command == "/database/get_users"):
      instruction.data = database.get_users()
   if(instruction.command == "/database/get_user_data"):
      instruction.data = database.get_user_data(json_object)
   if(instruction.command == "/database/set_user_data"):
      instruction.data = database.set_user_data(json_object)

   if(instruction.command == "/database/get_db_devices"):
      dev_list = []
      first_device = 1
      for dev in device_list:
         if(dev.connected or first_device):
            first_device = 0
            new_dev = {}
            new_dev["device_id"] = dev.device_id
            new_dev["name"] = dev.name
            new_dev["linked_devices"] = dev.linked_devices
            dev_list.append(new_dev)
      instruction.data = dev_list
   
   if(index_folder):
      index_inst_dict = instruction.dump_dict()
      global_data.media_queue.put(index_inst_dict)

   return instruction.data

def update_device_list(device_data):
   global device_list
   global device_timeouts
   found_device = 0
   device_update = 0
   if(device_data.device_id == device_list[0].device_id):
      device_update = update_device_timeouts()
   else:
      for index in range(1, len(device_list)):
         if(device_data.device_id == device_list[index].device_id):
            found_device = 1
            if(device_list[index].detected == 0):
               device_update = 1
            device_data.detected = 1
            device_list[index] = device_data
            device_timeouts[index] = 0
            database.update_db_device_in_list(device_list[index])
            break
      if(found_device == 0):
         device_data.detected = 1
         device_list.append(device_data)
         device_timeouts.append(0)
         database.update_db_device_in_list(device_data)
         device_update = 1

   if(device_update):
      instruction = global_data.instruction_class()
      instruction.command = "/network/reload_config"
      reload_inst_dict = instruction.dump_dict()
      global_data.network_queue.put(reload_inst_dict)

def update_device_timeouts():
   global device_list
   global device_timeouts
   disconnection = 0
   for index in range(1, len(device_timeouts)):
      if(device_list[index].connected == 1):
         device_timeouts[index] = device_timeouts[index] + device_list[0].hb_period
   for index in range(1, len(device_timeouts)):
      if(device_timeouts[index] > (device_list[index].hb_period*skips_till_timeout)):
         device_list[index].connected = 0
         device_timeouts[index] = 0
         database.update_db_device_in_list(device_list[index])
         disconnection = 1
   return disconnection

def init_system():
   global device_list
   global device_timeouts
   if(database.exists()):
      database.open_server_db()
      database.check_db()
   else:
      database.init_server_db()
   
   device_config = database.get_device_config()
   device_list = []
   device_list.append(device_config)
   device_timeouts = []
   device_timeouts.append(0)
   database.remove_unlinked_db_devices(device_config.device_id)
   linked_devices = database.get_filtered_db_devices()
   for new_device in linked_devices:
      new_device.connected = 0
      device_list.append(new_device)
      device_timeouts.append(0)

def exit_handler(signum, frame):
   global system_running
   if(system_running):
      system_running = 0
      print("caught exit... shutting down")
      instruction = global_data.instruction_class()
      instruction.command = "/system/shutdown_delay"
      sys_inst_dict = instruction.dump_dict()
      global_data.main_queue.put(sys_inst_dict, block=False)
      main_thread.stop_thread()

def stop_threads():
   networking.abort_broadcast_receive()
   heartbeat_listener_thread.stop_thread()
   media_player_thread.stop_thread()
   heartbeat_thread.stop_thread()
   network_thread.stop_thread()

def server_shutdown():
   database.remove_unlinked_db_devices(device_list[0].device_id)
   stop_threads()
   print("done")

print("Setting Up Interrupt Handler")
try:
   signal.signal(signal.SIGINT, exit_handler)
except:
   print("Couldn't lock SIGINT")
try:
   signal.signal(signal.SIGBREAK, exit_handler)
except:
   print("Couldn't lock SIGBREAK")
try:
   signal.signal(signal.SIGKILL, exit_handler)
except:
   print("Couldn't lock SIGKILL")
try:
   signal.signal(signal.SIGQUIT, exit_handler)
except:
   print("Couldn't lock SIGQUIT")
main_thread.start(server_main)
while(main_thread.is_active()):
   main_thread.pause(2)

