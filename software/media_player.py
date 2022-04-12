import database
import global_data
import subprocess
import vlc

# creating a vlc instance
vlc_instance = vlc.Instance()
# creating a media player
player = vlc_instance.media_player_new()

media_source = ""
media_active = 0
media_playing = 0

def run_media_player(player_thread):
   while (player_thread.is_active()):
      if(not global_data.media_queue.empty()):
         try:
            new_inst_dict = global_data.media_queue.get(block=False)
            new_queue_inst = global_data.instruction_class()
            new_queue_inst.load_dict(**new_inst_dict)
         except:
            #network queue was empty and timed out
            pass
         else:
            process_instruction(new_queue_inst)
      else:
         player_thread.pause(0.5)
      check_media_player()
   return

def check_media_player():
   global player
   global media_source
   global media_active
   global media_playing

   # make a paused timeout of certain amount of time where we just internally stop and clear media if it stays paused for long enough
   # we can set the media_inactive_timeout period in the config
   # we can also set a timeout for turning off the display or not with the timeout

   try:
      player_state = str(player.get_state())
      if((player_state == 'State.NothingSpecial') or
         (player_state == 'State.Stopped') or
         (player_state == 'State.Ended') or
         (player_state == 'State.Error')):
         media_active = 0
      else:
         media_active = 1
   except:
      media_active = 0

   return

def process_instruction(instruction:global_data.instruction_class):
   global vlc_instance
   global player
   global media_source
   global media_active

   if(instruction.is_global):
      instruction.global_done = 1

   if(instruction.command == "/database/index_media_folder"):
      database.index_media_folder(instruction.data)

   try:
      if(instruction.command == "/media/set_display"):
         if(instruction.data == "on"):
            instruction.data = "on done"
            command = "echo 'on 0' | cec-client -s"
            subprocess.call(command, shell=True, stdout=subprocess.DEVNULL)
         if(instruction.data == "off"):
            instruction.data = "off done"
            command = "echo 'standby 0' | cec-client -s"
            subprocess.call(command, shell=True, stdout=subprocess.DEVNULL)
         if(instruction.data == "active"):
            instruction.data = "active done"
            command = "echo 'as' | cec-client -s"
            subprocess.call(command, shell=True, stdout=subprocess.DEVNULL)
   except:
      instruction.data = "display_error"
      print("Error Setting Display")
   
   try:
      if(instruction.command == "/media/start"):
         if(instruction.is_global):
            new_base = instruction.data["base_path"]
            new_file = instruction.data["file_path"]
            link_path = {"src_path":new_base}
            link_dst = database.get_media_link(link_path)
            file_path = link_dst["dst_path"] + new_file[len(link_dst["src_path"]):]
         else:
            new_base = instruction.data["base_path"]
            new_file = instruction.data["file_path"]
            file_path = new_file
         
         media_source = file_path
         # creating a media
         media = vlc_instance.media_new(media_source)
         # setting media to the player
         player.set_media(media)
         player.play()
         player.toggle_fullscreen()
         instruction.data = "player_started"
      if(instruction.command == "/media/stop"):
         player.stop()
         instruction.data = "player_stopped"
      if(instruction.command == "/media/play"):
         player.play()
         instruction.data = "player_played"
      if(instruction.command == "/media/pause"):
         player.pause()
         instruction.data = "player_paused"
   except:
      instruction.data = "media_error"

   try:
      if(instruction.command == "/media/get_status"):
         status = {}
         status["active"] = media_active
         if(media_active):
            status["source"] = media_source
            status["total_time"] = player.get_length()
            status["current_time"] = player.get_time()
            status["current_position"] = player.get_position()
         instruction.data = status
      if(instruction.command == "/media/get_time"):
         time_status = {}
         time_status["total_time"] = player.get_length()
         time_status["current_time"] = player.get_time()
         time_status["current_position"] = player.get_position()
         instruction.data = time_status
      if(instruction.command == "/media/is_active"):
         instruction.data = media_active
   except:
      instruction.data = "status_error"
      
   if((instruction.is_internal == 0) or (instruction.is_global and instruction.global_done)):
      ret_inst_dict = instruction.dump_dict()
      global_data.network_queue.put(ret_inst_dict, block=False)
