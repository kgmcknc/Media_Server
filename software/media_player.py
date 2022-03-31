import database
import global_data

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
   if(instruction.command == "add_media_folder"):
      database.index_media_folder(instruction.data)
   if(instruction.command == "index_media_folder"):
      database.index_media_folder(instruction.data)
