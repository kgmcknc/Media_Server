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
      while(not global_data.media_queue.empty()):
         try:
            new_inst_dict = global_data.media_queue.get(block=False)
            new_queue_inst = global_data.instruction_class()
            new_queue_inst.load_dict(**new_inst_dict)
         except:
            #network queue was empty and timed out
            pass
         else:
            process_instruction(new_queue_inst)
         check_media_player()
      player_thread.pause(1)
   return

def check_media_player():
   global player
   global media_source
   global media_active
   global media_playing

   if(media_active == 0):
      return

def process_instruction(instruction:global_data.instruction_class):
   global vlc_instance
   global player
   global media_source

   return_data = ""

   if(instruction.command == "/database/index_media_folder"):
      instruction.global_done = 1
      database.index_media_folder(instruction.data)

   if(instruction.command == "/media/set_display"):
      instruction.global_done = 1
      try:
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
         print("couldn't do system call")
   
   if(instruction.command == "/media/start"):
      if(instruction.is_global):
         new_base = instruction.data["base_path"]
         new_file = instruction.data["file_path"]
         link_path = {"src_path":instruction.data["base_path"]}
         link_dst = database.get_media_link("link_path")
         new_file.replace(new_base, link_dst)
      else:
         new_base = instruction.data["base_path"]
         new_file = instruction.data["file_path"]
      
      media_source = new_file
      # creating a media
      media = vlc_instance.media_new(media_source)
      # setting media to the player
      player.set_media(media)
      player.play()
   if(instruction.command == "/media/play"):
      player.play()
   if(instruction.command == "/media/pause"):
      player.pause()
      
   if(instruction.is_global and instruction.global_done):
      instruction.data = return_data
      ret_inst_dict = instruction.dump_dict()
      global_data.network_queue.put(ret_inst_dict, block=False)
