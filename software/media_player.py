

def run_media_player(player_thread):
   while (player_thread.is_active()):
      player_thread.pause(1)

   return
