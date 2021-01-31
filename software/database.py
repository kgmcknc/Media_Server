
import pymongo

def open_server_db():
   db_client = pymongo.MongoClient("mongodb://localhost:27017/")
   dblist = db_client.list_database_names()
   if ("server_db" in dblist):
      server_db = db_client["server_db"]
      print("Server Database Exists")
   else:
      print("Creating Server Database")
      server_db = db_client["server_db"]
      
      update_config()
      
      update_features()

      server_devices = server_db["devices"]
      server_devices.insert_one({"name":"none", "id":"none", "ip":"none"})
      
      media_folder = server_db["media"]
      media_folder_list = media_folder["media_folder_list"]
      media_folder_list.insert_one({"name":"none", "path":"none"})

def update_config():
   server_config = server_db["config"]
   config_info = {"name": "blankname", "address": "none", "linked": "false", "connected": "false", "device_id": "0"}
   server_config.insert_one(config_info)

def update_feature():
   server_features = server_db["features"]
   feature_list = {"media_player":"false", "door_sensors":"false", "alarm":"false"}
   server_features.insert_one(feature_list)

def add_media_folder():
   pass

def rem_media_folder():
   pass

def index_media_folder():
   pass

def get_device_config():
   return b"testconfig"