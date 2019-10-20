from platformio import util
import glob, shutil, os, sys
import json

try:
    import configparser
except ImportError:
    import ConfigParser as configparser

print('########### Run release script ###########')

config = configparser.ConfigParser()
config.read("platformio.ini")

exampleFolder = config.get("common", "examples_folder")
version = config.get("common", "version").replace("\"","")

dirpath = os.getcwd()

# def prepExamples():
#     print 'Create examples folder %s' % (exampleFolder)
#     if os.path.exists(exampleFolder):
#         shutil.rmtree(exampleFolder)
#     os.makedirs(exampleFolder)
#     exampleFolderPath = exampleFolder + "/" + os.path.basename(dirpath)
#     os.makedirs(exampleFolder + "/" + os.path.basename(dirpath))
#     os.makedirs(exampleFolder + "/" + os.path.basename(dirpath) + "/data")
#     return exampleFolderPath

# def createExamples():
#     exampleFolder = prepExamples()
#     exampleFolderData = exampleFolder + "/data"
#     print 'Copy main.cpp file'
#     shutil.copy2('src/main.cpp', exampleFolder + "/main.cpp")
#     os.rename(exampleFolder + "/main.cpp", exampleFolder + "/" + os.path.basename(dirpath) + ".ino")

#     for file in glob.iglob('data/*.*'):
#         print 'Copied file %s' % (file)
#         shutil.copy2(file, exampleFolderData)

with open('library.json') as json_file:
    data = json.load(json_file)

version = data['version']
description = data['description']


f = open("library.properties", "r")
nf = open("library_.properties", "w+")
for l in f:
    if l.startswith("version"):
        nf.write("version=%s\n" % version)
    elif l.startswith("paragraph"):
        nf.write("paragraph=%s\n" % description)
    else:
        nf.write(l)

f.close()
nf.close()
os.remove("library.properties")
os.rename("library_.properties", "library.properties")

print('########### Finish release script ###########')
