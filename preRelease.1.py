import requests
from platformio import util
import glob, shutil, os, sys

config = util.load_project_config()
version = config.get("common", "version").replace("\"","")
name = config.get("common", "name").replace("\"","")

with open('.pioenvs/nodemcuv2/spiffs.bin', 'rb') as f:
    r = requests.post('http://hometome.go.ro/uploadSchema', files={'spiffs.bin': f}, data={"version": version, "name": name})

with open('.pioenvs/nodemcuv2/firmware.bin', 'rb') as f:
    r = requests.post('http://hometome.go.ro/uploadSchema', files={'firmware.bin': f}, data={"version": version, "name": name})