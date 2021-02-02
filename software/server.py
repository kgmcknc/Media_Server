
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

config_changed = 0

def server_main(main_thread):
   global config_changed
   print("Starting Main")

   #TODO need to make more advanced device list/struct/array for keeping track of stuff like whether current media is linked between other devices
   
   init_system()

   heartbeat.set_heartbeat_period(5)
   heartbeat_thread.start(heartbeat.heartbeat_keepalive)
   media_player_thread.start(media_player.run_media_player)
   heartbeat_listener_thread.start(heartbeat.heartbeat_listener)

   print("main while")
   while (main_thread.is_active()):
      #TODO main will wait for queue to have something (or timeout)
      # if timeout happens we just run through loop to do whatever periodic task is needed... (check ip is only one now)
      # queue is used to return data back to website 
      if(config_changed):
         print("Main config changed")
         global_data.heartbeat_queue.put(config_changed)
         config_changed = 0
      try:
         main_queue_data = global_data.main_queue.get(timeout=global_data.main_queue_timeout)
         if(main_queue_data):
            #process_main_request(main_queue_data)
            print("got new request in main")
      except:
         #main queue was empty and timed out
         pass
   
   server_shutdown()
   return

def init_system():
   if(database.exists()):
      database.open_server_db()
   else:
      database.init_server_db()
   
   device_list = []
   new_device = devices.server_device_class()
   #TODO convert from database to device class...
   new_device = database.get_device_config()
   device_list.append(new_device)

def exit_handler(signum, frame):
   print("caught exit... shutting down")
   main_thread.stop_thread()

def stop_threads():
   heartbeat_listener_thread.stop_thread()
   media_player_thread.stop_thread()
   heartbeat_thread.stop_thread()

def server_shutdown():
   #TODO: clean up device list. Remove un-linked devices and set status of every linked device to "not connected"
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
