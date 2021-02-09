
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
         instruction = global_data.instruction_class()
         instruction.group = "heartbeat_tasks"
         instruction.task = "reload_config"
         global_data.heartbeat_queue.put(instruction)
         config_changed = 0
      try:
         new_main_instruction = global_data.main_queue.get(timeout=global_data.main_queue_timeout)
      except:
         #main queue was empty and timed out
         pass
      else:
         process_main_instruction(new_main_instruction)
   
   server_shutdown()
   return

def process_main_instruction(instruction):
   global config_changed
   global device_list
   update_config = 0;

   if(instruction.group == "system_tasks"):
      if(instruction.task == "shutdown_delay"):
         main_thread.pause(1)
   
   if(instruction.group == "heartbeat_tasks"):
      if(instruction.task == "ip_changed"):
         device_list[0].ip_addr = networking.get_my_ip()
         update_config = 1

      if(instruction.task == "new_packet"):
         update_device_list(instruction.data)
   
   if(instruction.group == "local_tasks"):
      if(instruction.data["command"] == "link_device"):
         for dev in device_list:
            if(dev.device_id == instruction.data["device_id"]):
               database.add_linked_device(instruction.data["device_id"])
               config_changed = 1
               break
      if(instruction.data["command"] == "update_config"):
         instruction.data.pop("command")
         for data in instruction.data:
            if((data == 'device_id') or (data == '_id')):
               print("not updating id stuff")
            else:
               setattr(device_list[0], data, instruction.data[data])
         update_config = 1

   if(update_config):
      database.update_db_device_config(device_list[0])
      config_changed = 1

def update_device_list(device_data):
   global device_list
   global device_timeouts
   if(device_data.device_id == device_list[0].device_id):
      update_device_timeouts()
   else:
      for index in range(1, len(device_list)):
         if(device_data.device_id == device_list[index].device_id):
            device_data.connected = 1
            device_list[index] = device_data
            device_timeouts[index] = 0
            database.update_db_device_in_list(device_list[index])
            break
      else:
         device_data.connected = 1
         device_list.append(device_data)
         device_timeouts.append(0)
         database.update_db_device_in_list(device_data)

def update_device_timeouts():
   global device_list
   global device_timeouts
   for index in range(1, len(device_timeouts)):
      if(device_list[index].connected == 1):
         device_timeouts[index] = device_timeouts[index] + device_list[0].hb_period
   for index in range(1, len(device_timeouts)):
      if(device_timeouts[index] > (device_list[index].hb_period*skips_till_timeout)):
         device_list[index].connected = 0
         device_timeouts[index] = 0
         database.update_db_device_in_list(device_list[index])

def init_system():
   global device_list
   global device_timeouts
   if(database.exists()):
      database.open_server_db()
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
      instruction.group = "system_tasks"
      instruction.task = "shutdown_delay"
      global_data.main_queue.put(instruction, block=False)
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

