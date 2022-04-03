import database
import global_data
import subprocess
import vlc

def run_media_player(player_thread):
   while (player_thread.is_active()):
      while(not global_data.media_queue.empty()):
         try:
            new_queue_data = global_data.media_queue.get(block=False)
         except:
            #network queue was empty and timed out
            player_thread.pause(1)
            pass
         else:
            process_instruction(new_queue_data)
         



   return


def process_instruction(instruction:global_data.instruction_class):
   return_data = ""

   if(instruction.command == "/database/index_media_folder"):
      instruction.global_done = 1
      database.index_media_folder(instruction.data)

   if(instruction.command == "/media/set_display"):
      instruction.global_done = 1
      try:
         if(instruction.data == "on"):
            command = "echo 'on 0' | cec-client -s"
            subprocess.call(command, shell=True, stdout=subprocess.DEVNULL)
         if(instruction.data == "off"):
            command = "echo 'standby 0' | cec-client -s"
            subprocess.call(command, shell=True, stdout=subprocess.DEVNULL)
         if(instruction.data == "active"):
            command = "echo 'as' | cec-client -s"
            subprocess.call(command, shell=True, stdout=subprocess.DEVNULL)
      except:
         print("couldn't do system call")
   
   if(instruction.is_global and instruction.global_done):
      instruction.data = return_data
      return_instruction = instruction.copy()
      global_data.network_queue.put(return_instruction, block=False)
