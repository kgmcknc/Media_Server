
import media_player
import server_threads
import devices
import heartbeat
import database
import threading
import networking
import signal
import sys

main_thread = server_threads.server_thread_class("Main Server Thread")
media_player_thread = server_threads.server_thread_class("Media Player Thread")
network_thread = server_threads.server_thread_class("Network Thread")
heartbeat_thread = server_threads.server_thread_class("Heartbeat Thread")

def server_main(main_thread):
   print("starting main")

   heartbeat.set_heartbeat_period(5)
   heartbeat_thread.start(heartbeat.heartbeat_keepalive)
   media_player_thread.start(media_player.run_media_player)
   network_thread.start(networking.network_test)
   
   try:
      database.open_server_db()
   except:
      print("Couldn't connect to database... something's wrong...")
      server_shutdown()
      return

   new_device = devices.server_device_class()

   print("main while")
   while (main_thread.is_active()):
      main_thread.pause(4)
      print("Hello!")
   
   server_shutdown()
   return

def exit_handler(signum, frame):
   print("caught exit... shutting down")
   main_thread.stop_thread()

def stop_threads():
   network_thread.stop_thread()
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
